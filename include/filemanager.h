#pragma once
#include "globals.h"
#include "book.h"
#include "user.h"
#include "borrow.h"
#include <map>


// ══════════════════════════════════════════════════════════════════════════════
//  FileManager  –  load / save all data to text files
// ══════════════════════════════════════════════════════════════════════════════
class FileManager {
public:
    static void ensureDataDir() {
        portableMkdir("data");
    }

    // ─── Books ────────────────────────────────────────────────────────────
    static void saveBooks(const BookBST& bst) {
        ofstream f(BOOKS_FILE);
        if (!f) return;
        for (auto& b : bst.getAll())
            f << b.serialize() << "\n";
    }

    static void loadBooks(BookBST& bst) {
        ifstream f(BOOKS_FILE);
        if (!f) return;
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            Book b = Book::deserialize(line);
            if (b.isValid())          // skip corrupted / garbage entries
                bst.insert(b);
        }
    }

    // ─── Users ────────────────────────────────────────────────────────────
    static void saveUsers(const UserList& ul) {
        ofstream f(USERS_FILE);
        if (!f) return;
        for (auto& u : ul.getAll())
            f << u.serialize() << "\n";
    }

    static void loadUsers(UserList& ul) {
        ifstream f(USERS_FILE);
        if (!f) return;
        string line;
        while (getline(f, line))
            if (!line.empty()) ul.insertEnd(User::deserialize(line));
    }

    // ─── Borrow Records ───────────────────────────────────────────────────
    static void saveBorrows(const BorrowHistory& bh) {
        ofstream f(BORROWS_FILE);
        if (!f) return;
        for (auto& r : const_cast<BorrowHistory&>(bh).getAll())
            f << r.serialize() << "\n";
    }

    static void loadBorrows(BorrowHistory& bh) {
        ifstream f(BORROWS_FILE);
        if (!f) return;
        string line;
        while (getline(f, line))
            if (!line.empty()) bh.add(BorrowRecord::deserialize(line));
    }

    // ─── Seed default data ────────────────────────────────────────────────
    static void seedDefaults(BookBST& bst, UserList& ul) {
        // Admin
        if (!ul.findByEmail("admin@bookhieve.com")) {
            ul.insertEnd(User(1,"Admin","admin@bookhieve.com","admin123",true));
        }
        // Sample books
        if (bst.empty()) {
            bst.insert(Book(1,"The Pragmatic Programmer","Hunt & Thomas","Programming",1999,3));
            bst.insert(Book(2,"Clean Code","Robert C. Martin","Programming",2008,2));
            bst.insert(Book(3,"Introduction to Algorithms","CLRS","Science",2009,4));
            bst.insert(Book(4,"1984","George Orwell","Fiction",1949,5));
            bst.insert(Book(5,"Dune","Frank Herbert","Science Fiction",1965,3));
            bst.insert(Book(6,"The Hobbit","J.R.R. Tolkien","Fantasy",1937,4));
            bst.insert(Book(7,"Sapiens","Yuval Noah Harari","History",2011,3));
            bst.insert(Book(8,"Thinking Fast and Slow","Daniel Kahneman","Non-Fiction",2011,2));
            bst.insert(Book(9,"Harry Potter PS","J.K. Rowling","Fantasy",1997,5));
            bst.insert(Book(10,"The Great Gatsby","F. Scott Fitzgerald","Fiction",1925,3));
        }
    }

    // ─── Per-user data files (for enhancements) ──────────────────────────

    // data/userlist.txt — one userId per line
    static void saveUserList(const UserList& ul) {
        portableMkdir("data");
        ofstream f("data/userlist.txt");
        if (!f) return;
        for (auto& u : ul.getAll())
            f << u.id << "\n";
    }

    // data/history/<userId>.txt — bookId|borrowDate|returnDate
    static void savePerUserHistory(const BorrowHistory& bh) {
        portableMkdir("data/history");
        // Group records by userId
        map<int, vector<const BorrowRecord*>> byUser;
        for (auto& r : const_cast<BorrowHistory&>(bh).getAll())
            byUser[r.userId].push_back(&r);

        for (auto& p : byUser) {
            auto& uid = p.first;
            auto& records = p.second;
            ofstream f("data/history/" + to_string(uid) + ".txt");
            if (!f) continue;
            for (auto* r : records)
                f << r->bookId << "|" << r->borrowDate << "|" << r->returnDate << "\n";
        }
    }

    // data/active/<userId>.txt — bookId|bookTitle|dueDate|genre
    // (only non-returned borrows; needs catalogue for genre lookup)
    static void saveActiveBorrows(const BorrowHistory& bh, const BookBST& catalogue) {
        portableMkdir("data/active");
        // Group active (non-returned) records by userId
        map<int, vector<const BorrowRecord*>> byUser;
        for (auto& r : const_cast<BorrowHistory&>(bh).getAll())
            if (!r.returned)
                byUser[r.userId].push_back(&r);

        for (auto& p : byUser) {
            auto& uid = p.first;
            auto& records = p.second;
            ofstream f("data/active/" + to_string(uid) + ".txt");
            if (!f) continue;
            for (auto* r : records) {
                // Compute due date: borrowDate + BORROW_DAYS
                string dueDate;
                {
                    tm t{};
                    sscanf(r->borrowDate.c_str(), "%d-%d-%d",
                           &t.tm_year, &t.tm_mon, &t.tm_mday);
                    t.tm_year -= 1900; t.tm_mon -= 1;
                    time_t ts = mktime(&t);
                    ts += (time_t)BORROW_DAYS * 86400;
                    tm* lt = localtime(&ts);
                    char buf[11];
                    strftime(buf, sizeof(buf), "%Y-%m-%d", lt);
                    dueDate = string(buf);
                }
                // Look up genre from catalogue
                string genre;
                BSTNode* node = catalogue.search(r->bookId);
                if (node) genre = node->data.genre;

                f << r->bookId << "|" << r->bookTitle << "|"
                  << dueDate << "|" << genre << "\n";
            }
        }
    }
};
