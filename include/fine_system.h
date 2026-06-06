#pragma once
// ╔══════════════════════════════════════════════════════════════════════╗
// ║  Enhancement 3 – Progressive Fine System with Grace Period          ║
// ║  Tier 1 (days 1-3 overdue) : BDT 5/day                             ║
// ║  Tier 2 (day 4+)           : BDT 15/day                            ║
// ║  Grace period              : 1 free waiver per user per year        ║
// ╚══════════════════════════════════════════════════════════════════════╝
#ifndef FINE_SYSTEM_H
#define FINE_SYSTEM_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <cmath>


struct FineConfig {
    double tier1Rate    = 5.0;   // BDT per day, days 1-3
    double tier2Rate    = 15.0;  // BDT per day, day 4+
    int    tier1Days    = 3;     // how many days before escalation
};

class FineSystem {
private:
    const std::string graceDir = "data/grace/";

    // Grace record file: data/grace/<userId>.txt
    // Content: year used (e.g. "2025") or empty = never used
    std::string gracePath(const std::string& userId) const {
        return graceDir + userId + ".txt";
    }

    int currentYear() const {
        std::time_t t = std::time(nullptr);
        return 1900 + std::localtime(&t)->tm_year;
    }

    bool graceUsedThisYear(const std::string& userId) const {
        std::ifstream f(gracePath(userId));
        if (!f.is_open()) return false;
        int yr = 0;
        f >> yr;
        return yr == currentYear();
    }

    void markGraceUsed(const std::string& userId) const {
        portableMkdir(graceDir);
        std::ofstream f(gracePath(userId));
        f << currentYear() << "\n";
    }

public:
    FineSystem() {
        portableMkdir(graceDir);
    }

    // ── Calculate fine for `daysOverdue` days ────────────────────────────
    // Does NOT apply grace here — grace is applied separately at payment time.
    double calculateFine(int daysOverdue, const FineConfig& cfg = {}) const {
        if (daysOverdue <= 0) return 0.0;

        double fine = 0.0;
        int tier1 = std::min(daysOverdue, cfg.tier1Days);
        int tier2 = std::max(0, daysOverdue - cfg.tier1Days);
        fine += tier1 * cfg.tier1Rate;
        fine += tier2 * cfg.tier2Rate;
        return fine;
    }

    // ── Display fine breakdown at "Fine History" ──────────────────────────
    void showFineBreakdown(const std::string& userId,
                           int daysOverdue,
                           const FineConfig& cfg = {}) const {
        bool graceAvail = !graceUsedThisYear(userId);
        double gross = calculateFine(daysOverdue, cfg);

        std::cout << "\n  ╔══ Fine Breakdown ═══════════════════════════════╗\n";
        if (daysOverdue <= 0) {
            std::cout << "  ║  ✅ Returned on time — no fine.                 ║\n";
        } else {
            int t1 = std::min(daysOverdue, cfg.tier1Days);
            int t2 = std::max(0, daysOverdue - cfg.tier1Days);
            std::cout << "  ║  Days overdue    : " << daysOverdue << "\n";
            if (t1 > 0)
                std::cout << "  ║  Tier-1 (" << t1 << " day"
                          << (t1>1?"s":"") << " × BDT " << cfg.tier1Rate
                          << ") = BDT " << (t1 * cfg.tier1Rate) << "\n";
            if (t2 > 0)
                std::cout << "  ║  Tier-2 (" << t2 << " day"
                          << (t2>1?"s":"") << " × BDT " << cfg.tier2Rate
                          << ") = BDT " << (t2 * cfg.tier2Rate) << "\n";
            std::cout << "  ║  Gross fine      : BDT " << gross << "\n";

            if (graceAvail) {
                std::cout << "  ║  ⭐ Grace Period  : Available! (1 free waiver/year)\n"
                          << "  ║  Net fine        : BDT 0.00 (if grace applied)\n";
            } else {
                std::cout << "  ║  Grace Period    : Already used this year.\n"
                          << "  ║  Net fine        : BDT " << gross << "\n";
            }
        }
        std::cout << "  ╚════════════════════════════════════════════════╝\n";
    }

    // ── Apply payment (called from "Pay Fine") ───────────────────────────
    // Returns the actual amount charged (0 if grace was applied).
    double applyPayment(const std::string& userId,
                        int daysOverdue,
                        bool useGrace,
                        const FineConfig& cfg = {}) {
        if (daysOverdue <= 0) return 0.0;

        bool canUseGrace = useGrace && !graceUsedThisYear(userId);
        if (canUseGrace) {
            markGraceUsed(userId);
            std::cout << "\n  ✅ Grace period applied — fine waived for this offence.\n"
                      << "  (Grace period quota exhausted until next year.)\n";
            return 0.0;
        }

        double amount = calculateFine(daysOverdue, cfg);
        // Caller records the payment in the existing fine ledger
        return amount;
    }

    // ── Helper: days between two date strings "YYYY-MM-DD" ───────────────
    static int daysBetween(const std::string& from, const std::string& to) {
        auto parseDate = [](const std::string& s) -> std::time_t {
            std::tm tm{};
            std::istringstream ss(s);
            ss >> std::get_time(&tm, "%Y-%m-%d");
            return std::mktime(&tm);
        };
        double diff = std::difftime(parseDate(to), parseDate(from));
        return (int)std::round(diff / 86400.0);
    }

    // ── Admin view: summary of all active fines ──────────────────────────
    void showAdminFineSummary(const std::string& activeBorrowsFile = "data/active_borrows.txt") const {
        std::ifstream f(activeBorrowsFile);
        if (!f.is_open()) {
            std::cout << "  [Fine System] No active borrows file found.\n";
            return;
        }
        std::cout << "\n  ╔══ Active Fine Summary (Admin) ══════════════════╗\n";
        std::cout << "  ║  User         | Book          | Days OD | Fine\n";
        std::cout << "  ║  ─────────────────────────────────────────────\n";
        std::string line;
        bool any = false;
        // File format: userId|bookId|dueDate
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string uid, bid, due;
            std::getline(ss, uid, '|');
            std::getline(ss, bid, '|');
            std::getline(ss, due);

            // Get today's date string
            std::time_t now = std::time(nullptr);
            char buf[11];
            std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&now));
            int od = daysBetween(due, std::string(buf));
            if (od > 0) {
                double fine = calculateFine(od);
                std::cout << "  ║  " << uid << " | " << bid
                          << " | " << od << "d | BDT " << fine << "\n";
                any = true;
            }
        }
        if (!any)
            std::cout << "  ║  No overdue borrows at this time.\n";
        std::cout << "  ╚════════════════════════════════════════════════╝\n";
    }
};

#endif // FINE_SYSTEM_H
