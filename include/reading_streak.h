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
#include "dynamic_array.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>

class ReadingStats {
public:
    int totalBorrowed  = 0;
    int currentStreak  = 0;   // consecutive months with ≥1 borrow
    int longestStreak  = 0;
};

// Milestone thresholds and their badge labels
class Milestone {
public:
    int threshold;
    string badge;
};
static const Array<Milestone> MILESTONES = {
    {1,   "First Borrow"},
    {5,   "Bookworm (5 books)"},
    {10,  "Avid Reader (10 books)"},
    {25,  "Dedicated Reader (25 books)"},
    {50,  "50-Book Reader"},
    {100, "Century Reader"},
};

class ReadingStreakTracker {
private:
    // Returns "YYYY-MM" string from a "YYYY-MM-DD" date string
    static string yearMonth(const string& date) {
        return date.size() >= 7 ? date.substr(0, 7) : "";
    }

    // Current "YYYY-MM"
    static string currentYearMonth() {
        time_t t = time(nullptr);
        char buf[8];
        strftime(buf, sizeof(buf), "%Y-%m", localtime(&t));
        return string(buf);
    }

    // Decrement a "YYYY-MM" by one month
    static string prevMonth(const string& ym) {
        int yr  = stoi(ym.substr(0, 4));
        int mon = stoi(ym.substr(5, 2));
        if (--mon == 0) { mon = 12; --yr; }
        char buf[8];
        snprintf(buf, sizeof(buf), "%04d-%02d", yr, mon);
        return string(buf);
    }

public:
    // ── Compute stats from borrow records ──────────────────────────────
    // Reads data/borrows.txt format: recordId|userId|bookId|bookTitle|borrowDate|returnDate|returned|fine
    static size_t findStringIndex(const Array<string>& arr,
                                        const string& value) {
        for (size_t i = 0; i < arr.size(); ++i)
            if (arr[i] == value)
                return i;
        return arr.size();
    }

    static void insertUnique(Array<string>& arr, const string& value) {
        if (findStringIndex(arr, value) == arr.size())
            arr.push_back(value);
    }

    static void sortStrings(Array<string>& arr) {
        for (size_t i = 1; i < arr.size(); ++i) {
            string key = arr[i];
            size_t j = i;
            while (j > 0 && arr[j - 1] > key) {
                arr[j] = arr[j - 1];
                --j;
            }
            arr[j] = key;
        }
    }

    static ReadingStats compute(const string& userId) {
        ReadingStats stats;
        ifstream f("data/borrows.txt");
        if (!f.is_open()) return stats;

        Array<string> activeMonths;
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            istringstream ss(line);
            string recId, uid, bookId, bookTitle, bdate;
            getline(ss, recId,     '|');
            getline(ss, uid,       '|');
            getline(ss, bookId,    '|');
            getline(ss, bookTitle, '|');
            getline(ss, bdate,     '|');
            if (uid != userId) continue;
            if (bookId.empty() || bdate.empty()) continue;
            stats.totalBorrowed++;
            string ym = yearMonth(bdate);
            if (!ym.empty()) insertUnique(activeMonths, ym);
        }

        // ── Streak: walk backwards from current month ─────────────────────
        string cur = currentYearMonth();
        int streak = 0;
        while (findStringIndex(activeMonths, cur) < activeMonths.size()) {
            streak++;
            cur = prevMonth(cur);
        }
        stats.currentStreak = streak;

        // Longest streak: brute force over sorted month list
        if (!activeMonths.empty()) {
            sortStrings(activeMonths);
            int best = 1;
            int run = 1;
            for (size_t i = 1; i < activeMonths.size(); ++i) {
                string expected = prevMonth(activeMonths[i]);
                if (expected == activeMonths[i-1])
                    run++;
                else
                    run = 1;
                if (run > best) best = run;
            }
            stats.longestStreak = best;
        }

        return stats;
    }

    // ── Display (called from "View Borrow History") ───────────────────────
    static void display(const string& userId) {
        ReadingStats s = compute(userId);

        cout << "\n  ╔══ Your Reading Journey ═══════════════════════════╗\n";
        cout << "  ║    Total books borrowed : " << s.totalBorrowed << "\n";
        cout << "  ║  Current streak       : " << s.currentStreak
                  << " month" << (s.currentStreak != 1 ? "s" : "") << " in a row\n";
        cout << "  ║  Best streak ever      : " << s.longestStreak
                  << " month" << (s.longestStreak != 1 ? "s" : "") << "\n";

        // Show earned milestones
        cout << "  ║\n  ║  Milestones:\n";
        bool anyEarned = false;
        for (auto& p : MILESTONES) {
            auto& threshold = p.threshold;
            auto& badge = p.badge;
            if (s.totalBorrowed >= threshold) {
                cout << "  ║       " << badge << "\n";
                anyEarned = true;
            }
        }
        if (!anyEarned)
            cout << "  ║    (Borrow your first book to earn a badge!)\n";

        // Next milestone hint
        for (auto& p : MILESTONES) {
            auto& threshold = p.threshold;
            auto& badge = p.badge;
            if (s.totalBorrowed < threshold) {
                int left = threshold - s.totalBorrowed;
                cout << "  ║\n  ║  Next: " << badge << "  ("
                          << left << " book" << (left>1?"s":"") << " to go)\n";
                break;
            }
        }

        cout << "  ╚═══════════════════════════════════════════════════╝\n";
    }
};

#endif // READING_STREAK_H
