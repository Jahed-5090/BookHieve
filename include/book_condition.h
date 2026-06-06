#pragma once
// ╔══════════════════════════════════════════════════════════════════════╗
// ║  Enhancement 6 – Book Condition Tagging                             ║
// ║  Admin tags each book: New / Good / Fair / Worn                     ║
// ║  Worn books are flagged in admin catalog for potential removal.      ║
// ╚══════════════════════════════════════════════════════════════════════╝
#ifndef BOOK_CONDITION_H
#define BOOK_CONDITION_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <vector>


enum class BookCondition {
    New  = 0,
    Good = 1,
    Fair = 2,
    Worn = 3,
    Unknown = 4
};

// ── Conversion helpers ─────────────────────────────────────────────────
inline std::string conditionToStr(BookCondition c) {
    switch (c) {
        case BookCondition::New:  return "New";
        case BookCondition::Good: return "Good";
        case BookCondition::Fair: return "Fair";
        case BookCondition::Worn: return "Worn";
        default:                  return "Unknown";
    }
}

inline BookCondition strToCondition(const std::string& s) {
    if (s == "New")  return BookCondition::New;
    if (s == "Good") return BookCondition::Good;
    if (s == "Fair") return BookCondition::Fair;
    if (s == "Worn") return BookCondition::Worn;
    return BookCondition::Unknown;
}

// Visual badge shown in catalog
inline std::string conditionBadge(BookCondition c) {
    switch (c) {
        case BookCondition::New:  return "[🟢 New ]";
        case BookCondition::Good: return "[🔵 Good]";
        case BookCondition::Fair: return "[🟡 Fair]";
        case BookCondition::Worn: return "[🔴 Worn]";
        default:                  return "[❓    ]";
    }
}

class ConditionManager {
private:
    // Persisted as data/conditions.txt — format: bookId|condition
    const std::string condFile = "data/conditions.txt";
    std::unordered_map<std::string, BookCondition> condMap;

    void load() {
        condMap.clear();
        std::ifstream f(condFile);
        if (!f.is_open()) return;
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            auto pos = line.find('|');
            if (pos == std::string::npos) continue;
            std::string id  = line.substr(0, pos);
            std::string cnd = line.substr(pos + 1);
            condMap[id] = strToCondition(cnd);
        }
    }

    void save() {
        std::ofstream f(condFile);
        for (auto& p : condMap) {
            auto& id = p.first;
            auto& c = p.second;
            f << id << "|" << conditionToStr(c) << "\n";
        }
    }

public:
    ConditionManager() { load(); }

    // ── Admin: set or update condition ────────────────────────────────────
    void setCondition(const std::string& bookId, BookCondition c) {
        condMap[bookId] = c;
        save();
        std::cout << "  ✅ Condition for book [" << bookId << "] set to: "
                  << conditionToStr(c) << "\n";
    }

    // ── Admin: interactive prompt to tag a condition ───────────────────────
    BookCondition promptCondition() const {
        std::cout << "\n  Set book condition:\n"
                  << "    1. New\n"
                  << "    2. Good\n"
                  << "    3. Fair\n"
                  << "    4. Worn\n"
                  << "  Choice: ";
        int ch = 0;
        std::cin >> ch;
        switch (ch) {
            case 1: return BookCondition::New;
            case 2: return BookCondition::Good;
            case 3: return BookCondition::Fair;
            case 4: return BookCondition::Worn;
            default:
                std::cout << "  Invalid; defaulting to Good.\n";
                return BookCondition::Good;
        }
    }

    // ── Lookup ────────────────────────────────────────────────────────────
    BookCondition getCondition(const std::string& bookId) {
        auto it = condMap.find(bookId);
        if (it == condMap.end()) return BookCondition::Unknown;
        return it->second;
    }

    // Returns the badge string for inline display in catalog/search results
    std::string getBadge(const std::string& bookId) {
        return conditionBadge(getCondition(bookId));
    }

    // ── Admin: flag worn books for maintenance review ─────────────────────
    void showWornBooks() {
        load(); // refresh
        std::cout << "\n  ╔══ Admin: Books Flagged for Review (Worn) ══════════╗\n";
        bool any = false;
        for (auto& p : condMap) {
            auto& id = p.first;
            auto& c = p.second;
            if (c == BookCondition::Worn) {
                std::cout << "  ║  🔴 Book ID: " << id << "\n";
                any = true;
            }
        }
        if (!any)
            std::cout << "  ║  ✅ No worn books flagged.\n";
        std::cout << "  ╚════════════════════════════════════════════════════╝\n";
    }

    // ── For catalog display: append condition to a book row ───────────────
    // Call this inside your existing catalog print loop, e.g.:
    //   std::cout << book.title << " " << condMgr.getBadge(book.id) << "\n";
    void printCatalogRow(const std::string& bookId,
                         const std::string& title,
                         const std::string& author,
                         const std::string& genre) {
        std::string badge = getBadge(bookId);
        std::cout << "  " << badge << "  " << title
                  << "  by " << author
                  << "  [" << genre << "]\n";
    }
};

#endif // BOOK_CONDITION_H
