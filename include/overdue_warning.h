#pragma once
// ╔══════════════════════════════════════════════════════════════════════╗
// ║  Enhancement 5 – Overdue Early Warning System                       ║
// ║  User login: warn if any borrowed book is due within 2 days         ║
// ║  Admin panel: "soon overdue" list before fines accumulate           ║
// ╚══════════════════════════════════════════════════════════════════════╝
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
    std::string userId;
    std::string bookId;
    std::string bookTitle;
    std::string dueDate;
    int         daysLeft;   // negative = already overdue
};

class OverdueWarningSystem {
private:
    // ── Date utilities ────────────────────────────────────────────────────
    static std::string todayStr() {
        std::time_t t = std::time(nullptr);
        char buf[11];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&t));
        return std::string(buf);
    }

    // Returns days until due (negative = already overdue)
    static int daysUntilDue(const std::string& dueDate) {
        auto parseDate = [](const std::string& s) -> std::time_t {
            std::tm tm{};
            std::istringstream ss(s);
            ss >> std::get_time(&tm, "%Y-%m-%d");
            return std::mktime(&tm);
        };
        std::time_t now   = std::time(nullptr);
        std::time_t due   = parseDate(dueDate);
        double diff = std::difftime(due, now);
        return (int)std::round(diff / 86400.0);
    }

    // ── Load active borrows for a single user ─────────────────────────────
    // File: data/active/<userId>.txt — format: bookId|bookTitle|dueDate
    static Array<DueSoonEntry> loadUserActive(const std::string& userId) {
        Array<DueSoonEntry> entries;
        std::string path = "data/active/" + userId + ".txt";
        std::ifstream f(path);
        if (!f.is_open()) return entries;
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            std::istringstream ss(line);
            DueSoonEntry e;
            e.userId = userId;
            std::getline(ss, e.bookId,    '|');
            std::getline(ss, e.bookTitle, '|');
            std::getline(ss, e.dueDate);
            e.daysLeft = daysUntilDue(e.dueDate);
            entries.push_back(e);
        }
        return entries;
    }

public:
    // ── Called at user login (inside user_panel login flow) ───────────────
    static void checkOnLogin(const std::string& userId) {
        auto entries = loadUserActive(userId);
        Array<DueSoonEntry> warnings;

        for (auto& e : entries) {
            if (e.daysLeft <= 2)   // within 2 days OR already overdue
                warnings.push_back(e);
        }

        if (warnings.empty()) return;

        std::cout << "\n  ╔══ ⚠️  DUE DATE ALERT ═══════════════════════════════╗\n";
        for (auto& w : warnings) {
            if (w.daysLeft < 0) {
                std::cout << "  ║  OVERDUE by " << -w.daysLeft << " day"
                          << (-w.daysLeft > 1 ? "s" : "") << ": \""
                          << w.bookTitle << "\" (due " << w.dueDate << ")\n";
            } else if (w.daysLeft == 0) {
                std::cout << "  ║  🟠 DUE TODAY: \"" << w.bookTitle << "\"\n";
            } else {
                std::cout << "  ║  Due in " << w.daysLeft << " day"
                          << (w.daysLeft > 1 ? "s" : "") << ": \""
                          << w.bookTitle << "\" (due " << w.dueDate << ")\n";
            }
        }
        std::cout << "  ║  Please return or renew on time to avoid fines.\n"
                  << "  ╚═══════════════════════════════════════════════════╝\n";
    }

    // ── Admin panel: list all borrows due within `days` days ─────────────
    // Reads data/userlist.txt to iterate all users
    static void showAdminWarnings(int days = 2) {
        std::ifstream ul("data/userlist.txt");
        if (!ul.is_open()) {
            std::cout << "  [Overdue Warning] Cannot open userlist.\n";
            return;
        }

        Array<DueSoonEntry> soonOverdue;
        Array<DueSoonEntry> alreadyOverdue;

        std::string userId;
        while (std::getline(ul, userId)) {
            if (userId.empty()) continue;
            auto entries = loadUserActive(userId);
            for (auto& e : entries) {
                if (e.daysLeft < 0)
                    alreadyOverdue.push_back(e);
                else if (e.daysLeft <= days)
                    soonOverdue.push_back(e);
            }
        }

        std::cout << "\n  ╔══ Admin: Overdue & Soon-Due Report ══════════════════╗\n";

        if (!alreadyOverdue.empty()) {
            std::cout << "  ║  Currently Overdue:\n";
            for (auto& e : alreadyOverdue)
                std::cout << "  ║    User: " << e.userId
                          << "  Book: " << e.bookTitle
                          << "  (" << -e.daysLeft << " days overdue)\n";
        }

        if (!soonOverdue.empty()) {
            std::cout << "  ║\n  ║  Due Within " << days << " Day"
                      << (days > 1 ? "s" : "") << ":\n";
            for (auto& e : soonOverdue)
                std::cout << "  ║    User: " << e.userId
                          << "  Book: " << e.bookTitle
                          << "  (due " << e.dueDate << ", "
                          << e.daysLeft << "d left)\n";
        }

        if (alreadyOverdue.empty() && soonOverdue.empty())
            std::cout << "  ║  No overdue or near-due books right now.\n";

        std::cout << "  ╚═════════════════════════════════════════════════════╝\n";
    }
};

#endif // OVERDUE_WARNING_H
