#pragma once
// +----------------------------------------------------------------------╗
// |  Enhancement 3 - Progressive Fine System with Grace Period          |
// |  Tier 1 (days 1-3 overdue) : BDT 5/day                             |
// |  Tier 2 (day 4+)           : BDT 15/day                            |
// |  Grace period              : 1 free waiver per user per year        |
// +----------------------------------------------------------------------╝
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
    double tier1Rate    = 5.0;   // BDT per day, days 1-3
    double tier2Rate    = 15.0;  // BDT per day, day 4+
    int    tier1Days    = 3;     // how many days before escalation
};

class FineSystem {
private:
    const string graceDir = "data/grace/";

    // Grace record file: data/grace/<userId>.txt
    // Content: year used (e.g. "2025") or empty = never used
    string gracePath(const string& userId) const {
        return graceDir + userId + ".txt";
    }

    int currentYear() const {
        time_t t = time(nullptr);
        return 1900 + localtime(&t)->tm_year;
    }

    bool graceUsedThisYear(const string& userId) const {
        ifstream f(gracePath(userId));
        if (!f.is_open()) return false;
        int yr = 0;
        f >> yr;
        return yr == currentYear();
    }

    void markGraceUsed(const string& userId) const {
        portableMkdir(graceDir);
        ofstream f(gracePath(userId));
        f << currentYear() << "\n";
    }

public:
    FineSystem() {
        portableMkdir(graceDir);
    }

    // ── Calculate fine for `daysOverdue` days ────────────────────────────
    // Does NOT apply grace here - grace is applied separately at payment time.
    double calculateFine(int daysOverdue, const FineConfig& cfg = {}) const {
        if (daysOverdue <= 0) return 0.0;

        double fine = 0.0;
        int tier1 = min(daysOverdue, cfg.tier1Days);
        int tier2 = max(0, daysOverdue - cfg.tier1Days);
        fine += tier1 * cfg.tier1Rate;
        fine += tier2 * cfg.tier2Rate;
        return fine;
    }

    // ── Display fine breakdown at "Fine History" ──────────────────────────
    void showFineBreakdown(const string& userId,
                           int daysOverdue,
                           const FineConfig& cfg = {}) const {
        bool graceAvail = !graceUsedThisYear(userId);
        double gross = calculateFine(daysOverdue, cfg);

        cout << "\n  +-- Fine Breakdown -------------------------------╗\n";
        if (daysOverdue <= 0) {
            cout << "  |  Returned on time - no fine.                 |\n";
        } else {
            int t1 = min(daysOverdue, cfg.tier1Days);
            int t2 = max(0, daysOverdue - cfg.tier1Days);
            cout << "  |  Days overdue    : " << daysOverdue << "\n";
            if (t1 > 0)
                cout << "  |  Tier-1 (" << t1 << " day"
                          << (t1>1?"s":"") << " × BDT " << cfg.tier1Rate
                          << ") = BDT " << (t1 * cfg.tier1Rate) << "\n";
            if (t2 > 0)
                cout << "  |  Tier-2 (" << t2 << " day"
                          << (t2>1?"s":"") << " × BDT " << cfg.tier2Rate
                          << ") = BDT " << (t2 * cfg.tier2Rate) << "\n";
            cout << "  |  Gross fine      : BDT " << gross << "\n";

            if (graceAvail) {
                cout << "  |  Grace Period  : Available! (1 free waiver/year)\n"
                          << "  |  Net fine        : BDT 0.00 (if grace applied)\n";
            } else {
                cout << "  |  Grace Period    : Already used this year.\n"
                          << "  |  Net fine        : BDT " << gross << "\n";
            }
        }
        cout << "  +------------------------------------------------╝\n";
    }

    // ── Apply payment (called from "Pay Fine") ───────────────────────────
    // Returns the actual amount charged (0 if grace was applied).
    double applyPayment(const string& userId,
                        int daysOverdue,
                        bool useGrace,
                        const FineConfig& cfg = {}) {
        if (daysOverdue <= 0) return 0.0;

        bool canUseGrace = useGrace && !graceUsedThisYear(userId);
        if (canUseGrace) {
            markGraceUsed(userId);
            cout << "\n  Grace period applied - fine waived for this offence.\n"
                      << "  (Grace period quota exhausted until next year.)\n";
            return 0.0;
        }

        double amount = calculateFine(daysOverdue, cfg);
        // Caller records the payment in the existing fine ledger
        return amount;
    }

    // ── Helper: days between two date strings "YYYY-MM-DD" ───────────────
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

    // ── Admin view: summary of all active fines ──────────────────────────
    void showAdminFineSummary(const string& activeBorrowsFile = "data/active_borrows.txt") const {
        ifstream f(activeBorrowsFile);
        if (!f.is_open()) {
            cout << "  [Fine System] No active borrows file found.\n";
            return;
        }
        cout << "\n  +-- Active Fine Summary (Admin) ------------------╗\n";
        cout << "  |  " << left << setw(10) << "User"
             << " " << left << setw(16) << "Book"
             << " " << right << setw(8) << "Days OD"
             << " " << right << setw(14) << "Fine" << "\n";
        cout << "  |  " << string(10, '-') << " " << string(16, '-') << " "
             << string(8, '-') << " " << string(14, '-') << "\n";
        string line;
        bool any = false;
        // File format: userId|bookId|dueDate
        while (getline(f, line)) {
            if (line.empty()) continue;
            istringstream ss(line);
            string uid, bid, due;
            getline(ss, uid, '|');
            getline(ss, bid, '|');
            getline(ss, due);

            // Get today's date string
            time_t now = time(nullptr);
            char buf[11];
            strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&now));
            int od = daysBetween(due, string(buf));
            if (od > 0) {
                double fine = calculateFine(od);
                ostringstream fineText;
                fineText << fixed << setprecision(2) << "BDT " << fine;
                cout << "  |  " << left << setw(10) << uid
                     << " " << left << setw(16) << bid
                     << " " << right << setw(8) << (to_string(od) + "d")
                     << " " << right << setw(14) << fineText.str() << "\n";
                any = true;
            }
        }
        if (!any)
            cout << "  |  No overdue borrows at this time.\n";
        cout << "  +------------------------------------------------╝\n";
    }
};

#endif // FINE_SYSTEM_H
