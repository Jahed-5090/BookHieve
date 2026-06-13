#pragma once
#ifndef FINE_SYSTEM_H
#define FINE_SYSTEM_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <cmath>


class FineConfig {
public:
    double tier1Rate    = 5.0;
    double tier2Rate    = 15.0;
    int    tier1Days    = 3;
};

class FineSystem {
public:
    double calculateFine(int daysOverdue, const FineConfig& cfg = {}) const {
        if (daysOverdue <= 0) return 0.0;

        double fine = 0.0;
        int tier1 = min(daysOverdue, cfg.tier1Days);
        int tier2 = max(0, daysOverdue - cfg.tier1Days);
        fine += tier1 * cfg.tier1Rate;
        fine += tier2 * cfg.tier2Rate;
        return fine;
    }

    void showFineBreakdown(const string& userId,
                           int daysOverdue,
                           const FineConfig& cfg = {}) const {
        (void)userId;
        double gross = calculateFine(daysOverdue, cfg);

        cout << "\n  ╔══ Fine Breakdown ═══════════════════════════════╗\n";
        if (daysOverdue <= 0) {
            cout << "  ║  Returned on time — no fine.                 ║\n";
        } else {
            int t1 = min(daysOverdue, cfg.tier1Days);
            int t2 = max(0, daysOverdue - cfg.tier1Days);
            cout << "  ║  Days overdue    : " << daysOverdue << "\n";
            if (t1 > 0)
                cout << "  ║  Tier-1 (" << t1 << " day"
                          << (t1>1?"s":"") << " × BDT " << cfg.tier1Rate
                          << ") = BDT " << (t1 * cfg.tier1Rate) << "\n";
            if (t2 > 0)
                cout << "  ║  Tier-2 (" << t2 << " day"
                          << (t2>1?"s":"") << " × BDT " << cfg.tier2Rate
                          << ") = BDT " << (t2 * cfg.tier2Rate) << "\n";
            cout << "  ║  Gross fine      : BDT " << gross << "\n";
        }
        cout << "  ╚════════════════════════════════════════════════╝\n";
    }

    double applyPayment(const string& userId,
                        int daysOverdue,
                        bool useGrace,
                        const FineConfig& cfg = {}) {
        (void)userId;
        (void)useGrace;
        if (daysOverdue <= 0) return 0.0;

        double amount = calculateFine(daysOverdue, cfg);
        return amount;
    }

    static int daysBetween(const string& from, const string& to) {
        auto parseDate = [](const string& s) -> time_t {
            tm tm{};
            istringstream ss(s);
            ss >> get_time(&tm, "%Y-%m-%d");
            return mktime(&tm);
        };
        double diff = difftime(parseDate(to), parseDate(from));
        return (int)round(diff / 86400.0);
    }

    void showAdminFineSummary(const string& activeBorrowsFile = "data/active_borrows.txt") const {
        ifstream f(activeBorrowsFile);
        if (!f.is_open()) {
            cout << "  [Fine System] No active borrows file found.\n";
            return;
        }
        cout << "\n  ╔══ Active Fine Summary (Admin) ══════════════════╗\n";
        cout << "  ║  " << left << setw(10) << "User"
             << " " << left << setw(16) << "Book"
             << " " << right << setw(8) << "Days OD"
             << " " << right << setw(14) << "Fine" << "\n";
        cout << "  ║  " << string(10, '-') << " " << string(16, '-') << " "
             << string(8, '-') << " " << string(14, '-') << "\n";
        string line;
        bool any = false;
        while (getline(f, line)) {
            if (line.empty()) continue;
            istringstream ss(line);
            string uid, bid, due;
            getline(ss, uid, '|');
            getline(ss, bid, '|');
            getline(ss, due);

            time_t now = time(nullptr);
            char buf[11];
            strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&now));
            int od = daysBetween(due, string(buf));
            if (od > 0) {
                double fine = calculateFine(od);
                ostringstream fineText;
                fineText << fixed << setprecision(2) << "BDT " << fine;
                cout << "  ║  " << left << setw(10) << uid
                     << " " << left << setw(16) << bid
                     << " " << right << setw(8) << (to_string(od) + "d")
                     << " " << right << setw(14) << fineText.str() << "\n";
                any = true;
            }
        }
        if (!any)
            cout << "  ║  No overdue borrows at this time.\n";
        cout << "  ╚════════════════════════════════════════════════╝\n";
    }
};

#endif
