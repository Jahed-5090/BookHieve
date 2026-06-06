#pragma once
// ╔══════════════════════════════════════════════════════════════════════╗
// ║  Enhancement 2 – Borrow Waitlist Queue                              ║
// ║  Per-book FIFO queue persisted to data/waitlists/<bookId>.txt       ║
// ║  On book return, first user in queue is auto-notified.              ║
// ╚══════════════════════════════════════════════════════════════════════╝
#ifndef WAITLIST_H
#define WAITLIST_H

#include <string>
#include <queue>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>


class WaitlistManager {
private:
    const std::string waitlistDir = "data/waitlists/";
    const std::string notifyFile  = "data/notifications.txt";

    std::string waitlistPath(const std::string& bookId) const {
        return waitlistDir + bookId + ".txt";
    }

    // ── Persistence ──────────────────────────────────────────────────────
    std::queue<std::string> loadQueue(const std::string& bookId) const {
        std::queue<std::string> q;
        std::ifstream f(waitlistPath(bookId));
        if (!f.is_open()) return q;
        std::string uid;
        while (std::getline(f, uid))
            if (!uid.empty()) q.push(uid);
        return q;
    }

    void saveQueue(const std::string& bookId,
                   std::queue<std::string> q) const {
        portableMkdir(waitlistDir);
        std::ofstream f(waitlistPath(bookId));
        while (!q.empty()) {
            f << q.front() << "\n";
            q.pop();
        }
    }

    // Append a notification line: userId|bookId|message
    void addNotification(const std::string& userId,
                         const std::string& bookId,
                         const std::string& bookTitle) const {
        std::ofstream f(notifyFile, std::ios::app);
        f << userId << "|" << bookId << "|"
          << "Your waitlisted book is now available: " << bookTitle << "\n";
    }

public:
    WaitlistManager() {
        portableMkdir(waitlistDir);
    }

    // ── Add user to queue (called from "Borrow Book" when unavailable) ──
    bool enqueue(const std::string& userId, const std::string& bookId) {
        auto q = loadQueue(bookId);
        // Prevent duplicate entries
        std::queue<std::string> temp = q;
        while (!temp.empty()) {
            if (temp.front() == userId) {
                std::cout << "\n  [Waitlist] You are already in the queue "
                             "for this book.\n";
                return false;
            }
            temp.pop();
        }
        q.push(userId);
        saveQueue(bookId, q);
        // Show position
        std::cout << "\n  ╔══ Added to Waitlist ════════════════════════╗\n"
                  << "  ║  You are #" << q.size()
                  << " in the queue.                      ║\n"
                  << "  ║  We'll notify you when it becomes available. ║\n"
                  << "  ╚════════════════════════════════════════════╝\n";
        return true;
    }

    // ── Called automatically when a book is returned ─────────────────────
    // Returns the userId that was next in line (or "" if queue was empty).
    std::string notifyNext(const std::string& bookId,
                           const std::string& bookTitle) {
        auto q = loadQueue(bookId);
        if (q.empty()) return "";
        std::string nextUser = q.front();
        q.pop();
        saveQueue(bookId, q);
        addNotification(nextUser, bookId, bookTitle);
        std::cout << "\n  [Waitlist] User '" << nextUser
                  << "' has been notified that '" << bookTitle
                  << "' is now available.\n";
        return nextUser;
    }

    // ── Called at login to display pending notifications ──────────────────
    void showNotifications(const std::string& userId) const {
        std::vector<std::string> pending;
        std::vector<std::string> remaining;

        std::ifstream f(notifyFile);
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string uid, bid, msg;
            std::getline(ss, uid, '|');
            std::getline(ss, bid, '|');
            std::getline(ss, msg);
            if (uid == userId)
                pending.push_back(msg);
            else
                remaining.push_back(line);
        }
        f.close();

        if (!pending.empty()) {
            std::cout << "\n  ╔══ 📬 You Have Waitlist Notifications ══════════╗\n";
            for (auto& m : pending)
                std::cout << "  ║  ► " << m << "\n";
            std::cout << "  ╚════════════════════════════════════════════════╝\n";

            // Remove shown notifications so they don't repeat
            std::ofstream out(notifyFile);
            for (auto& r : remaining) out << r << "\n";
        }
    }

    // ── Queue size (display in catalog next to unavailable books) ─────────
    int queueSize(const std::string& bookId) const {
        auto q = loadQueue(bookId);
        return (int)q.size();
    }

    // ── Remove user from queue (e.g., user cancels) ───────────────────────
    void dequeue(const std::string& userId, const std::string& bookId) {
        auto q = loadQueue(bookId);
        std::queue<std::string> newQ;
        bool removed = false;
        while (!q.empty()) {
            if (q.front() == userId && !removed)
                removed = true; // skip once
            else
                newQ.push(q.front());
            q.pop();
        }
        saveQueue(bookId, newQ);
        if (removed)
            std::cout << "\n  [Waitlist] Removed from queue for book " << bookId << ".\n";
    }
};

#endif // WAITLIST_H
