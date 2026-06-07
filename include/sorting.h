#pragma once
#include "book.h"
#include <vector>
#include <functional>

// ══════════════════════════════════════════════════════════════════════════════
//  Sorting utilities for Book vectors
//  Implements: Bubble, Insertion, Selection, Quick, Merge
//  Used when admin views sorted catalogue views
// ══════════════════════════════════════════════════════════════════════════════

// ─── Bubble Sort ─────────────────────────────────────────────────────────────
inline void bubbleSortByTitle(vector<Book>& v) {
    int n = v.size();
    for (int i = 0; i < n-1; i++)
        for (int j = 0; j < n-i-1; j++)
            if (v[j].title > v[j+1].title)
                swap(v[j], v[j+1]);
}

// ─── Insertion Sort ──────────────────────────────────────────────────────────
inline void insertionSortByYear(vector<Book>& v) {
    int n = v.size();
    for (int i = 1; i < n; i++) {
        Book key = v[i];
        int j = i - 1;
        while (j >= 0 && v[j].year > key.year) {
            v[j+1] = v[j];
            j--;
        }
        v[j+1] = key;
    }
}

// ─── Selection Sort ──────────────────────────────────────────────────────────
inline void selectionSortByAuthor(vector<Book>& v) {
    int n = v.size();
    for (int i = 0; i < n-1; i++) {
        int minIdx = i;
        for (int j = i+1; j < n; j++)
            if (v[j].author < v[minIdx].author)
                minIdx = j;
        if (minIdx != i) swap(v[i], v[minIdx]);
    }
}

// ─── Quick Sort ──────────────────────────────────────────────────────────────
inline int partition(vector<Book>& v, int low, int high) {
    string pivot = v[high].title;
    int i = low - 1;
    for (int j = low; j < high; j++)
        if (v[j].title <= pivot)
            swap(v[++i], v[j]);
    swap(v[i+1], v[high]);
    return i + 1;
}

inline void quickSort(vector<Book>& v, int low, int high) {
    if (low < high) {
        int p = partition(v, low, high);
        quickSort(v, low,  p-1);
        quickSort(v, p+1, high);
    }
}

inline void quickSortByTitle(vector<Book>& v) {
    if (!v.empty()) quickSort(v, 0, (int)v.size()-1);
}

// ─── Merge Sort ──────────────────────────────────────────────────────────────
inline void mergeByAvailability(vector<Book>& v, int l, int m, int r) {
    vector<Book> left(v.begin()+l, v.begin()+m+1);
    vector<Book> right(v.begin()+m+1, v.begin()+r+1);
    int i=0, j=0, k=l;
    while (i<(int)left.size() && j<(int)right.size())
        v[k++] = (left[i].availableCopies >= right[j].availableCopies) ? left[i++] : right[j++];
    while (i<(int)left.size())  v[k++] = left[i++];
    while (j<(int)right.size()) v[k++] = right[j++];
}

inline void mergeSortByAvailability(vector<Book>& v, int l, int r) {
    if (l >= r) return;
    int m = (l+r)/2;
    mergeSortByAvailability(v, l, m);
    mergeSortByAvailability(v, m+1, r);
    mergeByAvailability(v, l, m, r);
}

inline void sortByAvailability(vector<Book>& v) {
    if (!v.empty()) mergeSortByAvailability(v, 0, (int)v.size()-1);
}

// ─── Genre Sort (Bubble Sort) ────────────────────────────────────────────────
inline void bubbleSortByGenre(vector<Book>& v) {
    int n = v.size();
    for (int i = 0; i < n-1; i++)
        for (int j = 0; j < n-i-1; j++)
            if (v[j].genre > v[j+1].genre)
                swap(v[j], v[j+1]);
}

// ─── Print sorted catalogue ───────────────────────────────────────────────────
inline void printSortedCatalogue(vector<Book>& books, int choice) {
    if (books.empty()) { cout << RED << "  Catalogue is empty.\n" << RESET; return; }
    switch (choice) {
        case 1: bubbleSortByTitle(books);      cout << YELLOW << "  Sorted by Title (Bubble Sort)\n" << RESET; break;
        case 2: insertionSortByYear(books);    cout << YELLOW << "  Sorted by Year (Insertion Sort)\n" << RESET; break;
        case 3: selectionSortByAuthor(books);  cout << YELLOW << "  Sorted by Author (Selection Sort)\n" << RESET; break;
        case 4: quickSortByTitle(books);       cout << YELLOW << "  Sorted by Title (Quick Sort)\n" << RESET; break;
        case 5: sortByAvailability(books);     cout << YELLOW << "  Sorted by Availability (Merge Sort)\n" << RESET; break;
        case 6: bubbleSortByGenre(books);       cout << YELLOW << "  Sorted by Genre\n" << RESET; break;
        default: break;
    }
    cout << BOLD << left
         << setw(6)  << "ID"
         << setw(32) << "Title"
         << setw(22) << "Author"
         << setw(14) << "Genre"
         << setw(6)  << "Year"
         << "Avail/Total\n" << RESET;
    printLine();
    for (auto& b : books) b.print();
}
