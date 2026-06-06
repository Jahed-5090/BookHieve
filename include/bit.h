#pragma once
#include "globals.h"
#include <vector>

// ══════════════════════════════════════════════════════════════════════════════
//  BinaryIndexedTree (Fenwick Tree)
//  Stores cumulative borrow counts / fine totals per user-id bucket
//  Supports: point update, prefix-sum query  – both O(log n)
//
//  Index = userId (1-based),  Value = total fines incurred
// ══════════════════════════════════════════════════════════════════════════════
class BIT {
    int            n;
    vector<double> tree;

    int lowbit(int i) const { return i & (-i); }

public:
    explicit BIT(int maxUserId) : n(maxUserId+1), tree(maxUserId+2, 0.0) {}

    // Add `val` to position i (1-based)
    void update(int i, double val) {
        for (; i <= n; i += lowbit(i))
            tree[i] += val;
    }

    // Prefix sum [1 .. i]
    double query(int i) const {
        double s = 0;
        for (; i > 0; i -= lowbit(i))
            s += tree[i];
        return s;
    }

    // Range sum [l .. r]
    double query(int l, int r) const {
        if (l > r) return 0;
        return query(r) - query(l-1);
    }

    void printStats(int maxUid) const {
        cout << BOLD << "  Cumulative Fine Statistics (BIT prefix sums):\n" << RESET;
        cout << BOLD << left << setw(10) << "UserID" << "Cumulative Fine (BDT)\n" << RESET;
        printLine();
        for (int i = 1; i <= maxUid; i++) {
            double v = query(i) - query(i-1);   // point value
            if (v > 0)
                cout << CYAN << left << setw(10) << i
                     << RED << fixed << setprecision(2) << v << RESET << "\n";
        }
    }
};
