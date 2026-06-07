#pragma once
// ╔══════════════════════════════════════════════════════════════════════╗
// ║  Enhancement 4 – Reading Streak & Borrowing Milestones              ║
// ║  Parses borrow history timestamps to compute:                        ║
// ║   • Total books borrowed (with milestone badges)                     ║
// ║   • Consecutive monthly streak                                       ║
// ╚══════════════════════════════════════════════════════════════════════╝
#ifndef READING_STREAK_H
#define READING_STREAK_H

#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <ctime>

struct ReadingStats {
    int totalBorrowed  = 0;
    int currentStreak  = 0;   // consecutive months with ≥1 borrow
    int longestStreak  = 0;
};

// Milestone thresholds and their badge labels
static const std::vector<std::pair<int,std::string>> MILESTONES = {
    {1,   "🌱 First Borrow"},
    {5,   " Bookworm (5 books)"},
    {10,  "⭐ Avid Reader (10 books)"},
    {25,  "🔥 Dedicated Reader (25 books)"},
    {50,  "🏆 50-Book Reader"},
    {100, "👑 Century Reader"},
};

class ReadingStreakTracker {
private:
    // Returns "YYYY-MM" string from a "YYYY-MM-DD" date string
    static std::string yearMonth(const std::string& date) {
        return date.size() >= 7 ? date.substr(0, 7) : "";
    }

    // Current "YYYY-MM"
    static std::string currentYearMonth() {
        std::time_t t = std::time(nullptr);
        char buf[8];
        std::strftime(buf, sizeof(buf), "%Y-%m", std::localtime(&t));
        return std::string(buf);
    }

    // Decrement a "YYYY-MM" by one month
    static std::string prevMonth(const std::string& ym) {
        int yr  = std::stoi(ym.substr(0, 4));
        int mon = std::stoi(ym.substr(5, 2));
        if (--mon == 0) { mon = 12; --yr; }
        char buf[8];
        std::snprintf(buf, sizeof(buf), "%04d-%02d", yr, mon);
        return std::string(buf);
    }

public:
    // ── Compute stats from borrow records ──────────────────────────────
    // Reads data/borrows.txt format: recordId|userId|bookId|bookTitle|borrowDate|returnDate|returned|fine
    static ReadingStats compute(const std::string& userId) {
        ReadingStats stats;
        std::ifstream f("data/borrows.txt");
        if (!f.is_open()) return stats;

        std::set<std::string> activeMonths;
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string recId, uid, bookId, bookTitle, bdate;
            std::getline(ss, recId,     '|');
            std::getline(ss, uid,       '|');
            std::getline(ss, bookId,    '|');
            std::getline(ss, bookTitle, '|');
            std::getline(ss, bdate,     '|');
            if (uid != userId) continue;
            if (bookId.empty() || bdate.empty()) continue;
            stats.totalBorrowed++;
            std::string ym = yearMonth(bdate);
            if (!ym.empty()) activeMonths.insert(ym);
        }

        // ── Streak: walk backwards from current month ─────────────────────
        std::string cur = currentYearMonth();
        int streak = 0;
        while (activeMonths.count(cur)) {
            streak++;
            cur = prevMonth(cur);
        }
        stats.currentStreak = streak;

        // Longest streak: brute force over sorted month set
        if (!activeMonths.empty()) {
            std::vector<std::string> months(activeMonths.begin(), activeMonths.end());
            int best = 1, run = 1;
            for (size_t i = 1; i < months.size(); ++i) {
                // Check consecutive
                std::string expected = prevMonth(months[i]);
                if (expected == months[i-1])
                    run++;
                else
                    run = 1;
                best = std::max(best, run);
            }
            stats.longestStreak = best;
        }

        return stats;
    }

    // ── Display (called from "View Borrow History") ───────────────────────
    static void display(const std::string& userId) {
        ReadingStats s = compute(userId);

        std::cout << "\n  ╔══ Your Reading Journey ═══════════════════════════╗\n";
        std::cout << "  ║    Total books borrowed : " << s.totalBorrowed << "\n";
        std::cout << "  ║  🔥 Current streak       : " << s.currentStreak
                  << " month" << (s.currentStreak != 1 ? "s" : "") << " in a row\n";
        std::cout << "  ║  🏅 Best streak ever      : " << s.longestStreak
                  << " month" << (s.longestStreak != 1 ? "s" : "") << "\n";

        // Show earned milestones
        std::cout << "  ║\n  ║  Milestones:\n";
        bool anyEarned = false;
        for (auto& p : MILESTONES) {
            auto& threshold = p.first;
            auto& badge = p.second;
            if (s.totalBorrowed >= threshold) {
                std::cout << "  ║       " << badge << "\n";
                anyEarned = true;
            }
        }
        if (!anyEarned)
            std::cout << "  ║    (Borrow your first book to earn a badge!)\n";

        // Next milestone hint
        for (auto& p : MILESTONES) {
            auto& threshold = p.first;
            auto& badge = p.second;
            if (s.totalBorrowed < threshold) {
                int left = threshold - s.totalBorrowed;
                std::cout << "  ║\n  ║  Next: " << badge << "  ("
                          << left << " book" << (left>1?"s":"") << " to go)\n";
                break;
            }
        }

        std::cout << "  ╚═══════════════════════════════════════════════════╝\n";
    }
};

#endif // READING_STREAK_H
