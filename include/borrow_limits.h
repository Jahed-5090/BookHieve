#pragma once
// ╔══════════════════════════════════════════════════════════════════════╗
// ║  Enhancement 7 – Borrow Limit by Book Category                     ║
// ║  Admin configures per-genre limits in data/borrow_limits.txt        ║
// ║  "Borrow Book" checks both global and category limits before        ║
// ║  allowing a borrow.                                                 ║
// ╚══════════════════════════════════════════════════════════════════════╝
#ifndef BORROW_LIMITS_H
#define BORROW_LIMITS_H

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>


class BorrowLimitManager {
private:
    // ── Config ────────────────────────────────────────────────────────────
    // data/borrow_limits.txt — format: genre|limit  (e.g. "Academic|2")
    // A special key "GLOBAL" sets the overall cap.
    const std::string limitsFile  = "data/borrow_limits.txt";
    // data/active/<userId>.txt — each line: bookId|bookTitle|dueDate|genre
    // (genre is the 4th field — add it when writing active borrows)
    const std::string activeDir   = "data/active/";

    std::unordered_map<std::string, int> limits; // genre→max (case-insensitive key)
    int globalLimit = 5;

    // Normalise genre key: trim + lowercase for case-insensitive comparison
    static std::string normalise(const std::string& s) {
        std::string r;
        for (char c : s) {
            if (c == ' ' || c == '\t') continue;
            r += (char)std::tolower(c);
        }
        return r;
    }

    void loadLimits() {
        limits.clear();
        std::ifstream f(limitsFile);
        if (!f.is_open()) {
            // Write sane defaults on first run
            writeLimits();
            return;
        }
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            auto pos = line.find('|');
            if (pos == std::string::npos) continue;
            std::string genre = line.substr(0, pos);
            int lim = std::stoi(line.substr(pos + 1));
            if (normalise(genre) == "global")
                globalLimit = lim;
            else
                limits[normalise(genre)] = lim;
        }
    }

    void writeLimits() {
        portableMkdir("data");
        std::ofstream f(limitsFile);
        f << "# Per-category borrow limits\n"
          << "# Format: genre|max_books\n"
          << "GLOBAL|5\n"
          << "Academic|2\n"
          << "Textbook|2\n"
          << "Fiction|3\n"
          << "Reference|1\n"
          << "Magazine|2\n";
    }

public:
    BorrowLimitManager() { loadLimits(); }

    // ── Count how many books of each genre a user currently has borrowed ──
    std::unordered_map<std::string,int> currentCountByGenre(const std::string& userId) const {
        std::unordered_map<std::string,int> counts;
        std::ifstream f(activeDir + userId + ".txt");
        if (!f.is_open()) return counts;
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string bid, title, due, genre;
            std::getline(ss, bid,   '|');
            std::getline(ss, title, '|');
            std::getline(ss, due,   '|');
            std::getline(ss, genre);
            if (!genre.empty())
                counts[normalise(genre)]++;
            counts["__total__"]++;
        }
        return counts;
    }

    // ── Main check: can userId borrow a book of genre `genre`? ───────────
    // Returns true if allowed, false + explanation if blocked.
    bool canBorrow(const std::string& userId, const std::string& genre) const {
        auto counts = currentCountByGenre(userId);

        // Global cap check
        int total = 0;
        auto git = counts.find("__total__");
        if (git != counts.end()) total = git->second;
        if (total >= globalLimit) {
            std::cout << "\n  ❌ Global borrow limit reached (" << globalLimit
                      << " books). Please return a book first.\n";
            return false;
        }

        // Category cap check
        std::string key = normalise(genre);
        auto lit = limits.find(key);
        if (lit != limits.end()) {
            int catLimit = lit->second;
            auto cit = counts.find(key);
            int catCount = (cit != counts.end()) ? cit->second : 0;
            if (catCount >= catLimit) {
                std::cout << "\n  ❌ Category limit for \"" << genre
                          << "\" reached (" << catCount << "/" << catLimit
                          << "). Return a book in this category first.\n";
                return false;
            }
        }
        return true;
    }

    // ── Display current usage to user ─────────────────────────────────────
    void showUsage(const std::string& userId) const {
        auto counts = currentCountByGenre(userId);
        int total = 0;
        auto git = counts.find("__total__");
        if (git != counts.end()) total = git->second;

        std::cout << "\n  ╔══ Your Borrow Allowance ═══════════════════════════╗\n";
        std::cout << "  ║  Overall: " << total << " / " << globalLimit << " books\n";
        for (auto& p : limits) {
            auto& genre = p.first;
            auto& lim = p.second;
            auto cit = counts.find(genre);
            int used = (cit != counts.end()) ? cit->second : 0;
            std::cout << "  ║  " << genre << ": " << used << " / " << lim << "\n";
        }
        std::cout << "  ╚════════════════════════════════════════════════════╝\n";
    }

    // ── Admin: update a category limit interactively ─────────────────────
    void adminUpdateLimit() {
        loadLimits();
        std::cout << "\n  Current limits:\n";
        std::cout << "    GLOBAL = " << globalLimit << "\n";
        for (auto& p : limits) {
            auto& g = p.first;
            auto& l = p.second;
            std::cout << "    " << g << " = " << l << "\n";
        }

        std::cout << "\n  Enter genre to update (or GLOBAL): ";
        std::string genre;
        std::cin.ignore();
        std::getline(std::cin, genre);

        std::cout << "  New limit: ";
        int lim;
        std::cin >> lim;

        if (normalise(genre) == "global")
            globalLimit = lim;
        else
            limits[normalise(genre)] = lim;

        writeLimits();
        // Rewrite with current values
        portableMkdir("data");
        std::ofstream f(limitsFile);
        f << "# Per-category borrow limits (updated by admin)\n"
          << "GLOBAL|" << globalLimit << "\n";
        for (auto& p : limits) {
            auto& g = p.first;
            auto& l = p.second;
            f << g << "|" << l << "\n";
        }

        std::cout << "  ✅ Limit updated.\n";
    }
};

#endif // BORROW_LIMITS_H
