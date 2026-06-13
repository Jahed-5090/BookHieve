#pragma once
// +----------------------------------------------------------------------╗
// |  Enhancement 5 - Overdue Early Warning System                       |
// |  User login: warn if any borrowed book is due within 2 days         |
// |  Admin panel: "soon overdue" list before fines accumulate           |
// +----------------------------------------------------------------------╝
#ifndef OVERDUE_WARNING_H
#define OVERDUE_WARNING_H

#include <string>
#include "dynamic_array.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <cmath>
#include <iomanip>

class DueSoonEntry {
public:
    string userId;
    string bookId;
    string bookTitle;
    string dueDate;
    int         daysLeft;   // negative = already overdue
};

class OverdueWarningSystem {
private:
    // ── Date utilities ────────────────────────────────────────────────────
    static string todayStr() {
        time_t t = time(nullptr);
        char buf[11];
        strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&t));
        return string(buf);
    }

    // Returns days until due (negative = already overdue)
    static int daysUntilDue(const string& dueDate) {
        auto parseDate = [](const string& s) -> time_t {
            tm tm{};
            istringstream ss(s);
            ss >> get_time(&tm, "%Y-%m-%d");
            return mktime(&tm);
        };
        time_t now   = time(nullptr);
        time_t due   = parseDate(dueDate);
        double diff = difftime(due, now);
        return (int)round(diff / 86400.0);
    }

    // ── Load active borrows for a single user ─────────────────────────────
    // File: data/active/<userId>.txt - format: bookId|bookTitle|dueDate
    static Array<DueSoonEntry> loadUserActive(const string& userId) {
        Array<DueSoonEntry> entries;
        string path = "data/active/" + userId + ".txt";
        ifstream f(path);
        if (!f.is_open()) return entries;
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            istringstream ss(line);
            DueSoonEntry e;
            e.userId = userId;
            getline(ss, e.bookId,    '|');
            getline(ss, e.bookTitle, '|');
            getline(ss, e.dueDate);
            e.daysLeft = daysUntilDue(e.dueDate);
            entries.push_back(e);
        }
        return entries;
    }

public:
    // ── Called at user login (inside user_panel login flow) ───────────────
    static void checkOnLogin(const string& userId) {
        auto entries = loadUserActive(userId);
        Array<DueSoonEntry> warnings;

        for (auto& e : entries) {
            if (e.daysLeft <= 2)   // within 2 days OR already overdue
                warnings.push_back(e);
        }

        if (warnings.empty()) return;

        cout << "\n  +-- [!]  DUE DATE ALERT -------------------------------╗\n";
        for (auto& w : warnings) {
            if (w.daysLeft < 0) {
                cout << "  |  OVERDUE by " << -w.daysLeft << " day"
                          << (-w.daysLeft > 1 ? "s" : "") << ": \""
                          << w.bookTitle << "\" (due " << w.dueDate << ")\n";
            } else if (w.daysLeft == 0) {
                cout << "  |  [TODAY] DUE TODAY: \"" << w.bookTitle << "\"\n";
            } else {
                cout << "  |  Due in " << w.daysLeft << " day"
                          << (w.daysLeft > 1 ? "s" : "") << ": \""
                          << w.bookTitle << "\" (due " << w.dueDate << ")\n";
            }
        }
        cout << "  |  Please return or renew on time to avoid fines.\n"
                  << "  +---------------------------------------------------╝\n";
    }

    // ── Admin panel: list all borrows due within `days` days ─────────────
    // Reads data/userlist.txt to iterate all users
    static void showAdminWarnings(int days = 2) {
        ifstream ul("data/userlist.txt");
        if (!ul.is_open()) {
            cout << "  [Overdue Warning] Cannot open userlist.\n";
            return;
        }

        Array<DueSoonEntry> soonOverdue;
        Array<DueSoonEntry> alreadyOverdue;

        string userId;
        while (getline(ul, userId)) {
            if (userId.empty()) continue;
            auto entries = loadUserActive(userId);
            for (auto& e : entries) {
                if (e.daysLeft < 0)
                    alreadyOverdue.push_back(e);
                else if (e.daysLeft <= days)
                    soonOverdue.push_back(e);
            }
        }

        cout << "\n  +-- Admin: Overdue & Soon-Due Report ------------------╗\n";

        if (!alreadyOverdue.empty()) {
            cout << "  |  Currently Overdue:\n";
            for (auto& e : alreadyOverdue)
                cout << "  |    User: " << e.userId
                          << "  Book: " << e.bookTitle
                          << "  (" << -e.daysLeft << " days overdue)\n";
        }

        if (!soonOverdue.empty()) {
            cout << "  |\n  |  Due Within " << days << " Day"
                      << (days > 1 ? "s" : "") << ":\n";
            for (auto& e : soonOverdue)
                cout << "  |    User: " << e.userId
                          << "  Book: " << e.bookTitle
                          << "  (due " << e.dueDate << ", "
                          << e.daysLeft << "d left)\n";
        }

        if (alreadyOverdue.empty() && soonOverdue.empty())
            cout << "  |  No overdue or near-due books right now.\n";

        cout << "  +-----------------------------------------------------╝\n";
    }
};

#endif // OVERDUE_WARNING_H
