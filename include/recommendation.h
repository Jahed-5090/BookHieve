#pragma once
// +----------------------------------------------------------------------╗
// |  Enhancement 1 - Smart Book Recommendation Engine                   |
// |  Uses frequency maps to suggest books by same author/genre and      |
// |  collaborative filtering via co-borrow counts.                       |
// +----------------------------------------------------------------------╝
#ifndef RECOMMENDATION_H
#define RECOMMENDATION_H

#include <string>
#include "dynamic_array.h"
#include <fstream>
#include <sstream>
#include <iostream>

// ── Data Structures ───────────────────────────────────────────────────────
class BookEntry {
public:
    string id;
    string title;
    string author;
    string genre;
};

class RecommendationEngine {
private:
    class StringIntEntry {
    public:
        string key;
        int value;
    };
    class CatalogueEntry {
    public:
        string bookId;
        BookEntry entry;
    };
    class UserBooksEntry {
    public:
        string userId;
        Array<string> books;
    };
    class CoBorrowEntry {
    public:
        string bookId;
        Array<StringIntEntry> counts;
    };
    class ScoreEntry {
    public:
        int score;
        string bookId;
    };

    // frequency map: bookId → how many times borrowed globally
    Array<StringIntEntry> globalBorrowFreq;
    // co-borrow map: bookId → set of bookIds borrowed by the same users
    Array<CoBorrowEntry> coBorrowMap;
    // catalogue: bookId → BookEntry
    Array<CatalogueEntry> catalogue;

    static size_t findIndex(const Array<StringIntEntry>& arr,
                                 const string& key) {
        for (size_t i = 0; i < arr.size(); ++i)
            if (arr[i].key == key)
                return i;
        return arr.size();
    }

    static size_t findIndex(const Array<CatalogueEntry>& arr,
                                 const string& key) {
        for (size_t i = 0; i < arr.size(); ++i)
            if (arr[i].bookId == key)
                return i;
        return arr.size();
    }

    static size_t findIndex(const Array<CoBorrowEntry>& arr,
                                 const string& key) {
        for (size_t i = 0; i < arr.size(); ++i)
            if (arr[i].bookId == key)
                return i;
        return arr.size();
    }

    static size_t findIndex(const Array<ScoreEntry>& arr,
                                 const string& key) {
        for (size_t i = 0; i < arr.size(); ++i)
            if (arr[i].bookId == key)
                return i;
        return arr.size();
    }

    static size_t findIndex(const Array<string>& arr,
                                 const string& key) {
        for (size_t i = 0; i < arr.size(); ++i)
            if (arr[i] == key)
                return i;
        return arr.size();
    }

    static bool contains(const Array<string>& arr, const string& key) {
        return findIndex(arr, key) < arr.size();
    }

    static size_t findIndexInCounts(const Array<StringIntEntry>& arr,
                                          const string& key) {
        for (size_t i = 0; i < arr.size(); ++i)
            if (arr[i].key == key)
                return i;
        return arr.size();
    }

    static int getCount(const Array<StringIntEntry>& arr, const string& key) {
        auto idx = findIndexInCounts(arr, key);
        return idx < arr.size() ? arr[idx].value : 0;
    }

    static void incrementCount(Array<StringIntEntry>& arr, const string& key) {
        auto idx = findIndexInCounts(arr, key);
        if (idx < arr.size())
            arr[idx].value += 1;
        else {
            StringIntEntry entry;
            entry.key = key;
            entry.value = 1;
            arr.push_back(entry);
        }
    }

    static void setCount(Array<StringIntEntry>& arr, const string& key, int value) {
        auto idx = findIndexInCounts(arr, key);
        if (idx < arr.size())
            arr[idx].value = value;
        else {
            StringIntEntry entry;
            entry.key = key;
            entry.value = value;
            arr.push_back(entry);
        }
    }

    static Array<string>* findUserBooks(Array<UserBooksEntry>& arr,
                                             const string& userId) {
        for (size_t i = 0; i < arr.size(); ++i)
            if (arr[i].userId == userId)
                return &arr[i].books;
        return nullptr;
    }

    BookEntry* findCatalogueEntry(const string& bookId) {
        auto idx = findIndex(catalogue, bookId);
        return idx < catalogue.size() ? &catalogue[idx].entry : nullptr;
    }

    const BookEntry* findCatalogueEntry(const string& bookId) const {
        auto idx = findIndex(catalogue, bookId);
        return idx < catalogue.size() ? &catalogue[idx].entry : nullptr;
    }

    int getGlobalBorrowFreq(const string& bookId) const {
        auto idx = findIndex(globalBorrowFreq, bookId);
        return idx < globalBorrowFreq.size() ? globalBorrowFreq[idx].value : 0;
    }

    void incrementGlobalBorrowFreq(const string& bookId) {
        auto idx = findIndex(globalBorrowFreq, bookId);
        if (idx < globalBorrowFreq.size())
            globalBorrowFreq[idx].value += 1;
        else {
            StringIntEntry entry;
            entry.key = bookId;
            entry.value = 1;
            globalBorrowFreq.push_back(entry);
        }
    }

    Array<StringIntEntry>* findCoBorrowSet(const string& bookId) {
        auto idx = findIndex(coBorrowMap, bookId);
        return idx < coBorrowMap.size() ? &coBorrowMap[idx].counts : nullptr;
    }

    const Array<StringIntEntry>* findCoBorrowSet(const string& bookId) const {
        auto idx = findIndex(coBorrowMap, bookId);
        return idx < coBorrowMap.size() ? &coBorrowMap[idx].counts : nullptr;
    }

    void loadCatalogue(const string& booksFile = "data/books.txt") {
        ifstream f(booksFile);
        if (!f.is_open()) return;
        string line;
        while (getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            istringstream ss(line);
            BookEntry b;
            string tok;
            int col = 0;
            while (getline(ss, tok, '|')) {
                switch (col++) {
                    case 0: b.id     = tok; break;
                    case 1: b.title  = tok; break;
                    case 2: b.author = tok; break;
                    case 3: b.genre  = tok; break;
                }
            }
            if (!b.id.empty()) {
                auto idx = findIndex(catalogue, b.id);
                if (idx < catalogue.size())
                    catalogue[idx].entry = b;
                else {
                    CatalogueEntry entry;
                    entry.bookId = b.id;
                    entry.entry = b;
                    catalogue.push_back(entry);
                }
            }
        }
    }

    void loadAllHistories() {
        ifstream f("data/borrows.txt");
        if (!f.is_open()) return;

        Array<UserBooksEntry> userBooks;
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            istringstream ss(line);
            string recId, userId, bookId;
            getline(ss, recId,  '|');
            getline(ss, userId, '|');
            getline(ss, bookId, '|');
            if (!userId.empty() && !bookId.empty()) {
                auto books = findUserBooks(userBooks, userId);
                if (!books) {
                    UserBooksEntry userEntry;
                    userEntry.userId = userId;
                    userBooks.push_back(userEntry);
                    books = &userBooks[userBooks.size() - 1].books;
                }
                books->push_back(bookId);
                incrementGlobalBorrowFreq(bookId);
            }
        }

        for (size_t p = 0; p < userBooks.size(); ++p) {
            auto& books = userBooks[p].books;
            for (size_t i = 0; i < books.size(); ++i)
                for (size_t j = 0; j < books.size(); ++j)
                    if (i != j) {
                        auto bookId = books[i];
                        auto otherId = books[j];
                        auto setPtr = findCoBorrowSet(bookId);
                        if (!setPtr) {
                            CoBorrowEntry entry;
                            entry.bookId = bookId;
                            coBorrowMap.push_back(entry);
                            setPtr = &coBorrowMap[coBorrowMap.size() - 1].counts;
                        }
                        incrementCount(*setPtr, otherId);
                    }
        }
    }

public:
    RecommendationEngine() {
        loadCatalogue();
        loadAllHistories();
    }

    Array<BookEntry> recommend(const string& userId,
                                     int limit = 5) const {
        Array<string> alreadyRead;
        Array<string> knownAuthors;
        Array<string> knownGenres;
        {
            ifstream f("data/borrows.txt");
            string line;
            while (getline(f, line)) {
                if (line.empty()) continue;
                istringstream ss(line);
                string recId, uid, bookId;
                getline(ss, recId,  '|');
                getline(ss, uid,    '|');
                getline(ss, bookId, '|');
                if (uid != userId) continue;
                if (bookId.empty()) continue;
                if (!contains(alreadyRead, bookId))
                    alreadyRead.push_back(bookId);
                const BookEntry* entry = findCatalogueEntry(bookId);
                if (entry != nullptr) {
                    if (!contains(knownAuthors, entry->author))
                        knownAuthors.push_back(entry->author);
                    if (!contains(knownGenres, entry->genre))
                        knownGenres.push_back(entry->genre);
                }
            }
        }

        Array<ScoreEntry> score;
        for (size_t i = 0; i < catalogue.size(); ++i) {
            const auto& bid = catalogue[i].bookId;
            const auto& book = catalogue[i].entry;
            if (contains(alreadyRead, bid)) continue;

            int s = 0;
            for (size_t j = 0; j < alreadyRead.size(); ++j) {
                const auto& readBid = alreadyRead[j];
                const auto* coSet = findCoBorrowSet(readBid);
                if (coSet != nullptr) {
                    int count = getCount(*coSet, bid);
                    if (count > 0)
                        s += count * 3;
                }
            }
            if (contains(knownAuthors, book.author)) s += 5;
            if (contains(knownGenres, book.genre))   s += 2;
            s += getGlobalBorrowFreq(bid);

            if (s > 0) {
                auto idx = findIndex(score, bid);
                if (idx < score.size())
                    score[idx].score = s;
                else {
                    ScoreEntry entry;
                    entry.score = s;
                    entry.bookId = bid;
                    score.push_back(entry);
                }
            }
        }

        // Simple descending insertion sort based on score
        for (size_t i = 1; i < score.size(); ++i) {
            ScoreEntry key = score[i];
            size_t j = i;
            while (j > 0 && score[j - 1].score < key.score) {
                score[j] = score[j - 1];
                --j;
            }
            score[j] = key;
        }

        Array<BookEntry> result;
        for (int i = 0; i < limit && i < (int)score.size(); ++i) {
            const BookEntry* entry = findCatalogueEntry(score[i].bookId);
            if (entry)
                result.push_back(*entry);
        }
        return result;
    }

    void displayRecommendations(const string& userId) const {
        auto recs = recommend(userId);
        if (recs.empty()) {
            cout << "\n  [Recommendations] Borrow a few books first "
                         "to unlock personalised suggestions!\n";
            return;
        }
        cout << "\n  +-- Recommended For You ----------------------------╗\n";
        int n = 1;
        for (auto& b : recs) {
            cout << "  |  " << n++ << ". " << b.title
                      << "  (" << b.author << ")  [" << b.genre << "]\n";
        }
        cout << "  +---------------------------------------------------╝\n";
    }
};

#endif // RECOMMENDATION_H
