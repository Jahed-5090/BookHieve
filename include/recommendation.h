#pragma once
// ╔══════════════════════════════════════════════════════════════════════╗
// ║  Enhancement 1 – Smart Book Recommendation Engine                   ║
// ║  Uses frequency maps to suggest books by same author/genre and      ║
// ║  collaborative filtering via co-borrow counts.                       ║
// ╚══════════════════════════════════════════════════════════════════════╝
#ifndef RECOMMENDATION_H
#define RECOMMENDATION_H

#include <string>
#include "dynamic_array.h"
#include <unordered_map>
#include <map>
#include <set>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>

// ── Data Structures ───────────────────────────────────────────────────────
class BookEntry {
public:
    std::string id;
    std::string title;
    std::string author;
    std::string genre;
};

class RecommendationEngine {
private:
    // frequency map: bookId → how many times borrowed globally
    std::unordered_map<std::string, int> globalBorrowFreq;
    // co-borrow map: bookId → set of bookIds borrowed by the same users
    std::unordered_map<std::string, std::map<std::string,int>> coBorrowMap;
    // catalogue: bookId → BookEntry
    std::unordered_map<std::string, BookEntry> catalogue;

    // Load all books from data/books.txt  (pipe-delimited: id|title|author|genre|...)
    void loadCatalogue(const std::string& booksFile = "data/books.txt") {
        std::ifstream f(booksFile);
        if (!f.is_open()) return;
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::istringstream ss(line);
            BookEntry b;
            std::string tok;
            int col = 0;
            while (std::getline(ss, tok, '|')) {
                switch (col++) {
                    case 0: b.id     = tok; break;
                    case 1: b.title  = tok; break;
                    case 2: b.author = tok; break;
                    case 3: b.genre  = tok; break;
                }
            }
            if (!b.id.empty()) catalogue[b.id] = b;
        }
    }

    // Load borrow history from data/borrows.txt (the actual data file)
    // File format per line: recordId|userId|bookId|bookTitle|borrowDate|returnDate|returned|fine
    void loadAllHistories() {
        std::ifstream f("data/borrows.txt");
        if (!f.is_open()) return;

        // Group borrows by userId
        std::unordered_map<std::string, Array<std::string>> userBooks;
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string recId, userId, bookId;
            std::getline(ss, recId,  '|');
            std::getline(ss, userId, '|');
            std::getline(ss, bookId, '|');
                if (!userId.empty() && !bookId.empty()) {
                userBooks[userId].push_back(bookId);
                globalBorrowFreq[bookId]++;
            }
        }

        // Build co-borrow pairs for collaborative filtering
        for (auto& p : userBooks) {
            auto& books = p.second;
            for (size_t i = 0; i < books.size(); ++i)
                for (size_t j = 0; j < books.size(); ++j)
                    if (i != j)
                        coBorrowMap[books[i]][books[j]]++;
        }
    }

public:
    RecommendationEngine() {
        loadCatalogue();
        loadAllHistories();
    }

    // ── Main API ────────────────────────────────────────────────────────
    // Returns up to `limit` recommended BookEntry objects for a given user.
    // Priority: 1) co-borrowed companions  2) same author  3) same genre
    Array<BookEntry> recommend(const std::string& userId,
                                     int limit = 5) const {
        // Step 1: collect what this user has already read from data/borrows.txt
        std::set<std::string> alreadyRead;
        std::set<std::string> knownAuthors, knownGenres;
        {
            std::ifstream f("data/borrows.txt");
            std::string line;
            while (std::getline(f, line)) {
                if (line.empty()) continue;
                std::istringstream ss(line);
                std::string recId, uid, bookId;
                std::getline(ss, recId,  '|');
                std::getline(ss, uid,    '|');
                std::getline(ss, bookId, '|');
                if (uid != userId) continue;
                if (bookId.empty()) continue;
                alreadyRead.insert(bookId);
                auto it = catalogue.find(bookId);
                if (it != catalogue.end()) {
                    knownAuthors.insert(it->second.author);
                    knownGenres.insert(it->second.genre);
                }
            }
        }

        // Step 2: score every unread book
        std::map<std::string, int> score; // bookId → score
        for (auto& p : catalogue) {
            auto& bid = p.first;
            auto& book = p.second;
            if (alreadyRead.count(bid)) continue;

            int s = 0;
            // Co-borrow signal
            for (const auto& readBid : alreadyRead) {
                auto cit = coBorrowMap.find(readBid);
                if (cit != coBorrowMap.end()) {
                    auto pit = cit->second.find(bid);
                    if (pit != cit->second.end())
                        s += pit->second * 3; // weight 3 for co-borrow
                }
            }
            // Same author bonus
            if (knownAuthors.count(book.author)) s += 5;
            // Same genre bonus
            if (knownGenres.count(book.genre))   s += 2;
            // Global popularity tiebreaker
            auto git = globalBorrowFreq.find(bid);
            if (git != globalBorrowFreq.end())   s += git->second;

            if (s > 0) score[bid] = s;
        }

        // Step 3: sort by score descending, return top `limit`
        Array<std::pair<int,std::string>> ranked;
        ranked.reserve(score.size());
        for (auto& p : score) {
            auto& bid = p.first;
            auto& sc = p.second;
            ranked.push_back({sc, bid});
        }
        std::sort(ranked.begin(), ranked.end(),
                  [](auto& a, auto& b){ return a.first > b.first; });

        Array<BookEntry> result;
        for (int i = 0; i < std::min(limit, (int)ranked.size()); ++i) {
            auto it = catalogue.find(ranked[i].second);
            if (it != catalogue.end())
                result.push_back(it->second);
        }
        return result;
    }

    // ── Display helper (call from user_panel / admin) ────────────────────
    void displayRecommendations(const std::string& userId) const {
        auto recs = recommend(userId);
        if (recs.empty()) {
            std::cout << "\n  [Recommendations] Borrow a few books first "
                         "to unlock personalised suggestions!\n";
            return;
        }
        std::cout << "\n  ╔══ Recommended For You ════════════════════════════╗\n";
        int n = 1;
        for (auto& b : recs) {
            std::cout << "  ║  " << n++ << ". " << b.title
                      << "  (" << b.author << ")  [" << b.genre << "]\n";
        }
        std::cout << "  ╚═══════════════════════════════════════════════════╝\n";
    }
};

#endif // RECOMMENDATION_H
