#pragma once
#include "globals.h"
#include "dynamic_array.h"
#include <algorithm>

// ══════════════════════════════════════════════════════════════════════════════
//  FineEntry  –  user + outstanding fine
// ══════════════════════════════════════════════════════════════════════════════
struct FineEntry {
    int    userId;
    string userName;
    double fineAmount;

    FineEntry() : userId(0), fineAmount(0) {}
    FineEntry(int uid, const string& name, double fine)
        : userId(uid), userName(name), fineAmount(fine) {}

    void print() const {
        cout << CYAN << left
             << setw(8)  << userId
             << setw(24) << userName.substr(0,23)
             << RED
             << fixed << setprecision(2) << fineAmount << " BDT"
             << RESET << "\n";
    }
};

// ══════════════════════════════════════════════════════════════════════════════
//  MaxHeap  –  FineEntry keyed on fineAmount
//  Used for: "Who owes the most?" query & heap sort demo
// ══════════════════════════════════════════════════════════════════════════════
class FineMaxHeap {
    Array<FineEntry> heap;

    void heapifyUp(int i) {
        while (i > 0) {
            int parent = (i-1)/2;
            if (heap[parent].fineAmount < heap[i].fineAmount) {
                swap(heap[parent], heap[i]);
                i = parent;
            } else break;
        }
    }

    void heapifyDown(int i) {
        int n = heap.size();
        while (true) {
            int largest = i, l = 2*i+1, r = 2*i+2;
            if (l < n && heap[l].fineAmount > heap[largest].fineAmount) largest = l;
            if (r < n && heap[r].fineAmount > heap[largest].fineAmount) largest = r;
            if (largest == i) break;
            swap(heap[i], heap[largest]);
            i = largest;
        }
    }

public:
    void insert(const FineEntry& e) {
        heap.push_back(e);
        heapifyUp(heap.size()-1);
    }

    FineEntry extractMax() {
        if (heap.empty()) throw runtime_error("Heap empty");
        FineEntry top = heap[0];
        heap[0] = heap[heap.size()-1];
        heap.pop_back();
        if (!heap.empty()) heapifyDown(0);
        return top;
    }

    FineEntry& peekMax() {
        if (heap.empty()) throw runtime_error("Heap empty");
        return heap[0];
    }

    bool  empty() const { return heap.empty(); }
    int   size()  const { return heap.size(); }

    // Heap Sort – sorts a copy in ascending order, prints result
    void heapSort(Array<FineEntry> arr) {
        // Build max heap
        int n = arr.size();
        for (int i = n/2-1; i >= 0; i--) {
            // sift down in-place
            int root = i;
            while (true) {
                int largest = root, l = 2*root+1, r = 2*root+2;
                if (l < n && arr[l].fineAmount > arr[largest].fineAmount) largest = l;
                if (r < n && arr[r].fineAmount > arr[largest].fineAmount) largest = r;
                if (largest == root) break;
                swap(arr[root], arr[largest]);
                root = largest;
            }
        }
        // Extract
        for (int i = n-1; i > 0; i--) {
            swap(arr[0], arr[i]);
            int root = 0, sz = i;
            while (true) {
                int largest = root, l = 2*root+1, r = 2*root+2;
                if (l < sz && arr[l].fineAmount > arr[largest].fineAmount) largest = l;
                if (r < sz && arr[r].fineAmount > arr[largest].fineAmount) largest = r;
                if (largest == root) break;
                swap(arr[root], arr[largest]);
                root = largest;
            }
        }
        // arr is now sorted ascending
        cout << BOLD << left << setw(8) << "UserID" << setw(24) << "Name" << "Fine (BDT)\n" << RESET;
        printLine();
        for (auto& e : arr) e.print();
    }

    void printTopFines(int k=5) {
        // Copy heap and extract top k
        FineMaxHeap tmp = *this;
        int cnt = min(k, (int)tmp.size());
        cout << BOLD << "  Top " << cnt << " outstanding fines:\n" << RESET;
        cout << BOLD << left << setw(8) << "UserID" << setw(24) << "Name" << "Fine\n" << RESET;
        printLine();
        for (int i = 0; i < cnt; i++) {
            tmp.extractMax().print();
        }
    }
};

// ══════════════════════════════════════════════════════════════════════════════
//  MinHeap  –  for "next return due" queries
// ══════════════════════════════════════════════════════════════════════════════
struct DueEntry {
    int    recordId, userId, bookId;
    string dueDate, bookTitle;
    DueEntry() : recordId(0), userId(0), bookId(0) {}
    DueEntry(int rid, int uid, int bid, const string& dd, const string& bt)
        : recordId(rid), userId(uid), bookId(bid), dueDate(dd), bookTitle(bt) {}
};

class DueMinHeap {
    Array<DueEntry> heap;

    void heapifyUp(int i) {
        while (i > 0) {
            int p = (i-1)/2;
            if (heap[p].dueDate > heap[i].dueDate) { swap(heap[p], heap[i]); i = p; }
            else break;
        }
    }

    void heapifyDown(int i) {
        int n = heap.size();
        while (true) {
            int smallest = i, l = 2*i+1, r = 2*i+2;
            if (l < n && heap[l].dueDate < heap[smallest].dueDate) smallest = l;
            if (r < n && heap[r].dueDate < heap[smallest].dueDate) smallest = r;
            if (smallest == i) break;
            swap(heap[i], heap[smallest]);
            i = smallest;
        }
    }

public:
    void insert(const DueEntry& e) {
        heap.push_back(e);
        heapifyUp(heap.size()-1);
    }

    void printUpcoming(int n=10) const {
        if (heap.empty()) { cout << RED << "  No active borrows.\n" << RESET; return; }
        cout << BOLD << left
             << setw(8) << "RecID"
             << setw(8) << "User"
             << setw(28)<< "Book"
             << "Due Date\n" << RESET;
        printLine();
        Array<DueEntry> tmp = heap;
        // Simple display (don't destroy heap)
        sort(tmp.begin(), tmp.end(), [](auto& a, auto& b){ return a.dueDate < b.dueDate; });
        int cnt = min(n, (int)tmp.size());
        for (int i = 0; i < cnt; i++) {
            cout << CYAN << left
                 << setw(8) << tmp[i].recordId
                 << setw(8) << tmp[i].userId
                 << setw(28)<< tmp[i].bookTitle.substr(0,27)
                 << YELLOW << tmp[i].dueDate
                 << RESET << "\n";
        }
    }

    bool empty() const { return heap.empty(); }
};
