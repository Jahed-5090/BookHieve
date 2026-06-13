#pragma once
// ╔══════════════════════════════════════════════════════════════════════╗
// ║  Enhancement 2 – Borrow Waitlist Queue                              ║
// ║  Per-book FIFO queue persisted to data/waitlists/<bookId>.txt       ║
// ║  On book return, first user in queue is auto-notified.              ║
// ╚══════════════════════════════════════════════════════════════════════╝
#ifndef WAITLIST_H
#define WAITLIST_H

#include <string>
#include "custom_queue.h"
#include "dynamic_array.h"
#include <fstream>
#include <sstream>
#include <iostream>


class WaitlistManager {
private:
    const string waitlistDir = "data/waitlists/";
    const string notifyFile  = "data/notifications.txt";

    string waitlistPath(const string& bookId) const {
        return waitlistDir + bookId + ".txt";
    }

    // ── Persistence ──────────────────────────────────────────────────────
    Queue<string> loadQueue(const string& bookId) const {
        Queue<string> q;
        ifstream f(waitlistPath(bookId));
        if (!f.is_open()) return q;
        string uid;
        while (getline(f, uid))
            if (!uid.empty()) q.push(uid);
        return q;
    }

    void saveQueue(const string& bookId,
                   Queue<string> q) const {
        portableMkdir(waitlistDir);
        ofstream f(waitlistPath(bookId));
        while (!q.empty()) {
            f << q.front() << "\n";
            q.pop();
        }
    }

    // Append a notification line: userId|bookId|message
    void addNotification(const string& userId,
                         const string& bookId,
                         const string& bookTitle) const {
        ofstream f(notifyFile, ios::app);
        f << userId << "|" << bookId << "|"
          << "Your waitlisted book is now available: " << bookTitle << "\n";
    }

public:
    WaitlistManager() {
        portableMkdir(waitlistDir);
    }

    // ── Add user to queue (called from "Borrow Book" when unavailable) ──
    bool enqueue(const string& userId, const string& bookId) {
        auto q = loadQueue(bookId);
        
        // Prevent duplicate entries - check if user is already in queue
        Queue<string> temp;
        bool isDuplicate = false;
        while (!q.empty()) {
            string current = q.front();
            if (current == userId) {
                isDuplicate = true;
            }
            temp.push(current);
            q.pop();
        }
        
        if (isDuplicate) {
            cout << "\n  [Waitlist] You are already in the queue "
                         "for this book.\n";
            return false;
        }
        
        // Add user to queue
        temp.push(userId);
        saveQueue(bookId, temp);
        
        // Show position
        cout << "\n  ╔══ Added to Waitlist ════════════════════════╗\n"
                  << "  ║  You are #" << (temp.size())
                  << " in the queue.                      ║\n"
                  << "  ║  We'll notify you when it becomes available. ║\n"
                  << "  ╚════════════════════════════════════════════╝\n";
        return true;
    }

    // ── Called automatically when a book is returned ─────────────────────
    // Returns the userId that was next in line (or "" if queue was empty).
    string notifyNext(const string& bookId,
                           const string& bookTitle) {
        auto q = loadQueue(bookId);
        if (q.empty()) return "";
        string nextUser = q.front();
        q.pop();
        saveQueue(bookId, q);
        addNotification(nextUser, bookId, bookTitle);
        cout << "\n  [Waitlist] User '" << nextUser
                  << "' has been notified that '" << bookTitle
                  << "' is now available.\n";
        return nextUser;
    }

    // ── Called at login to display pending notifications ──────────────────
    void showNotifications(const string& userId) const {
        Array<string> pending;
        Array<string> remaining;

        ifstream f(notifyFile);
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            istringstream ss(line);
            string uid, bid, msg;
            getline(ss, uid, '|');
            getline(ss, bid, '|');
            getline(ss, msg);
            if (uid == userId)
                pending.push_back(msg);
            else
                remaining.push_back(line);
        }
        f.close();

        if (!pending.empty()) {
            cout << "\n  ╔══ 📬 You Have Waitlist Notifications ══════════╗\n";
            for (auto& m : pending)
                cout << "  ║  ► " << m << "\n";
            cout << "  ╚════════════════════════════════════════════════╝\n";

            // Remove shown notifications so they don't repeat
            ofstream out(notifyFile);
            for (auto& r : remaining) out << r << "\n";
        }
    }

    // ── Queue size (display in catalog next to unavailable books) ─────────
    int queueSize(const string& bookId) const {
        auto q = loadQueue(bookId);
        return (int)q.size();
    }

    // ── Remove user from queue (e.g., user cancels) ───────────────────────
    void dequeue(const string& userId, const string& bookId) {
        auto q = loadQueue(bookId);
        Queue<string> newQ;
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
            cout << "\n  [Waitlist] Removed from queue for book " << bookId << ".\n";
    }
};

#endif // WAITLIST_H
