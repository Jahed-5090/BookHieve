#pragma once
#include "book.h"
#include "user.h"
#include "borrow.h"
#include "sorting.h"

#include "heap.h"

#include "filemanager.h"
#include "recommendation.h"  // Enhancement 1
#include "waitlist.h"        // Enhancement 2
#include "fine_system.h"     // Enhancement 3
#include "reading_streak.h"  // Enhancement 4
#include "overdue_warning.h" // Enhancement 5
#include "book_condition.h"  // Enhancement 6
#include "borrow_limits.h"   // Enhancement 7

extern RecommendationEngine recEngine;
extern WaitlistManager      waitMgr;
extern FineSystem           fineSys;
extern ConditionManager     condMgr;
extern BorrowLimitManager   limitMgr;

// ══════════════════════════════════════════════════════════════════════════════
//  Library  –  central engine that owns all DSA structures
// ══════════════════════════════════════════════════════════════════════════════
class Library
{
public:
    BookBST catalogue;     
    UserList members;      
    BorrowHistory history; 
    BorrowStack undoStack; 
    BorrowQueue waitQueue; 

    FineMaxHeap fineHeap;  
    DueMinHeap dueHeap;    


    User *currentUser = nullptr;

    // 2. REMOVE the manager variables from inside the class. 
    // They are now global externs, so the Library doesn't need to "own" them.

    Library()
    {
        FileManager::ensureDataDir();
        FileManager::loadBooks(catalogue);
        FileManager::loadUsers(members);
        FileManager::loadBorrows(history);
        // Sync user.borrowCount with active (non-returned) borrows
        // Prevents mismatch between stored user counters and history
        for (auto &u_copy : members.getAll()) {
            User *u = members.findById(u_copy.id);
            if (u) u->borrowCount = 0;
        }
        for (auto &r : history.getAll()) {
            if (!r.returned) {
                User *u = members.findById(r.userId);
                if (u) u->borrowCount++;
            }
        }
        FileManager::seedDefaults(catalogue, members);

        rebuildHeaps();
    }

    ~Library() { save(); }

    void save()
    {
        FileManager::saveBooks(catalogue);
        FileManager::saveUsers(members);
        FileManager::saveBorrows(history);
        // Generate per-user data files for enhancements
        FileManager::saveUserList(members);
        FileManager::savePerUserHistory(history);
        FileManager::saveActiveBorrows(history, catalogue);
    }

    void rebuildHeaps()
    {
        // Rebuild fine & due heaps from current history
        // (heaps are in-memory, rebuilt on load)
        map<int, double> userFines;
        string today = currentDate();
        for (auto &r : history.getAll())
        {
            if (!r.returned)
            {
                int overdue = daysBetween(r.borrowDate, today) - BORROW_DAYS;
                if (overdue > 0)
                {
                    double grossFine = fineSys.calculateFine(overdue);
                    double owed = max(0.0, grossFine - r.finePaid);
                    userFines[r.userId] += owed;
                    // Due date
                    // (simple: borrowDate + BORROW_DAYS, skip for brevity)
                }
            }
        }
        for (auto &p : userFines) {
            auto &uid = p.first;
            auto &fine = p.second;
            User *u = members.findById(uid);
            if (u)
            {
                fineHeap.insert(FineEntry(uid, u->name, fine));
            }
        }
    }

    // ─── Book next ID ─────────────────────────────────────────────────────
    int nextBookId()
    {
        auto books = catalogue.getAll();
        int mx = 0;
        for (auto &b : books)
            mx = max(mx, b.id);
        return mx + 1;
    }

    // ─── Borrow a book ────────────────────────────────────────────────────
    bool borrowBook(int uid, int bid)
    {
        User *u = members.findById(uid);
        if (!u)
        {
            cout << RED << "  User not found.\n"
                 << RESET;
            return false;
        }
        if (u->borrowCount >= MAX_BORROW)
        {
            cout << RED << "  Borrow limit (" << MAX_BORROW << ") reached.\n"
                 << RESET;
            return false;
        }
        BSTNode *node = catalogue.search(bid);
        if (!node)
        {
            cout << RED << "  Book not found.\n"
                 << RESET;
            return false;
        }
        if (node->data.availableCopies <= 0)
        {
            cout << YELLOW << "  No copies available. Adding to wait queue.\n"
                 << RESET;
            BorrowRecord req(history.nextId(), uid, bid, node->data.title, currentDate());
            waitQueue.enqueue(req);
            return false;
        }
        // Block borrowing only if outstanding unpaid fines exceed BDT 100
        double unpaid = currentFine(uid);
        if (unpaid > 100.0)
        {
            cout << RED << "  Your unpaid fines are BDT " << fixed << setprecision(2)
                 << unpaid << ". You must pay them before borrowing more books.\n"
                 << RESET;
            return false;
        }
        // Check already borrowed
        if (history.findActive(uid, bid))
        {
            cout << RED << "  You have already borrowed this book.\n"
                 << RESET;
            return false;
        }
        BorrowRecord rec(history.nextId(), uid, bid, node->data.title, currentDate());
        history.add(rec);
        undoStack.push(rec);
        node->data.availableCopies--;
        u->borrowCount++;
        // Add to dueHeap
        // Compute due date string (simple: add BORROW_DAYS)
        time_t t = time(nullptr) + (time_t)BORROW_DAYS * 86400;
        tm *lt = localtime(&t);
        char buf[11];
        strftime(buf, sizeof(buf), "%Y-%m-%d", lt);
        dueHeap.insert(DueEntry(rec.recordId, uid, bid, string(buf), node->data.title));
        save();
        cout << GREEN << "  Borrowed: " << node->data.title
             << " (due " << buf << ")\n"
             << RESET;
        return true;
    }

    // ─── Return a book ────────────────────────────────────────────────────
    bool returnBook(int uid, int bid)
    {
        BorrowRecord *rec = history.findActive(uid, bid);
        if (!rec)
        {
            cout << RED << "  No active borrow record found.\n"
                 << RESET;
            return false;
        }

        string today = currentDate();
        rec->returnDate = today;
        rec->returned = true;

        int overdue = daysBetween(rec->borrowDate, today) - BORROW_DAYS;
        if (overdue > 0)
        {
            double grossFine = fineSys.calculateFine(overdue);
            double owed = max(0.0, grossFine - rec->finePaid);
            rec->fine = grossFine;
            if (owed > 0)
            {
                cout << YELLOW << "  Book returned " << overdue << " day(s) late."
                     << " Fine: BDT " << fixed << setprecision(2) << owed << "\n"
                     << RESET;
                User *u = members.findById(uid);
                if (u)
                    fineHeap.insert(FineEntry(uid, u->name, owed));
            }
            else
            {
                cout << GREEN << "  Book returned late, but overdue fine already cleared.\n"
                     << RESET;
            }
        }
        else
        {
            cout << GREEN << "  Book returned on time. No fine.\n"
                 << RESET;
        }

        BSTNode *node = catalogue.search(bid);
        if (node)
            node->data.availableCopies++;

        User *u = members.findById(uid);
        if (u && u->borrowCount > 0)
            u->borrowCount--;

        // Check wait queue for this book
        // (simplified: peek queue, if book matches, approve it)

        save();
        return true;
    }

    // ─── Pay fine ────────────────────────────────────────────────────────
    double currentFine(int uid)
    {
        double total = 0;
        for (auto &r : history.getAll())
        {
            if (r.userId != uid)
                continue;
            total += r.dueFine();
        }
        return total;
    }

    double payFine(int uid)
    {
        double totalCharged = 0;
        bool changed = false;

        for (auto &r : history.getAll())
        {
            if (r.userId != uid)
                continue;

            double owed = r.dueFine();
            if (owed <= 0)
                continue;

            double grossFine = 0.0;
            if (r.returned)
                grossFine = fineSys.calculateFine(daysBetween(r.borrowDate, r.returnDate) - BORROW_DAYS);
            else
                grossFine = fineSys.calculateFine(daysBetween(r.borrowDate, currentDate()) - BORROW_DAYS);

            r.finePaid += owed;
            if (r.finePaid >= grossFine)
                r.fine = 0.0;
            else
                r.fine = grossFine;

            totalCharged += owed;
            changed = true;
        }

        if (changed)
        {
            rebuildHeaps();
            save();
        }

        return totalCharged;
    }
};
