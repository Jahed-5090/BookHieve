#pragma once
#include "globals.h"
#include "dynamic_array.h"

// ══════════════════════════════════════════════════════════════════════════════
//  BorrowRecord  –  one borrow event
// ══════════════════════════════════════════════════════════════════════════════
class BorrowRecord {
public:
    int    recordId;
    int    userId;
    int    bookId;
    string bookTitle;
    string borrowDate;
    string returnDate;   // empty if not returned
    bool   returned;
    double fine;
    double finePaid;

    BorrowRecord() : recordId(0), userId(0), bookId(0), returned(false), fine(0), finePaid(0) {}

    BorrowRecord(int rid, int uid, int bid, const string& bt, const string& bd)
        : recordId(rid), userId(uid), bookId(bid), bookTitle(bt),
          borrowDate(bd), returned(false), fine(0), finePaid(0) {}

    static double calculateFineForDays(int daysOverdue) {
        if (daysOverdue <= 0) return 0.0;
        int tier1 = min(daysOverdue, 3);
        int tier2 = max(0, daysOverdue - 3);
        return tier1 * 5.0 + tier2 * 15.0;
    }

    double dueFine() const {
        if (returned) {
            return max(0.0, fine - finePaid);
        }
        int overdue = daysBetween(borrowDate, currentDate()) - BORROW_DAYS;
        if (overdue <= 0) return 0.0;
        return max(0.0, calculateFineForDays(overdue) - finePaid);
    }

    void print() const {
        double due = dueFine();
        cout << left
             << setw(6)  << recordId
             << setw(8)  << userId
             << setw(8)  << bookId
             << setw(28) << bookTitle.substr(0,27)
             << setw(12) << borrowDate
             << setw(12) << (returned ? returnDate : "—")
             << (due > 0 ? "Yes" : "No ")
             << fixed << setprecision(2) << due
             << "\n";
    }

    string serialize() const {
        return to_string(recordId) + "|" + to_string(userId) + "|" +
               to_string(bookId)   + "|" + bookTitle + "|" +
               borrowDate + "|" + returnDate + "|" +
               (returned ? "1" : "0") + "|" + to_string(fine) +
               "|" + to_string(finePaid);
    }

    static BorrowRecord deserialize(const string& line) {
        BorrowRecord r;
        stringstream ss(line);
        string tok;
        getline(ss, tok, '|'); r.recordId   = stoi(tok);
        getline(ss, tok, '|'); r.userId     = stoi(tok);
        getline(ss, tok, '|'); r.bookId     = stoi(tok);
        getline(ss, r.bookTitle,  '|');
        getline(ss, r.borrowDate, '|');
        getline(ss, r.returnDate, '|');
        getline(ss, tok, '|'); r.returned   = tok == "1";
        getline(ss, tok, '|'); r.fine       = stod(tok);
        if (getline(ss, tok, '|'))
            r.finePaid = stod(tok);
        else
            r.finePaid = 0;
        return r;
    }
};

// ══════════════════════════════════════════════════════════════════════════════
//  Stack  –  used for "undo last borrow" and fine history navigation
// ══════════════════════════════════════════════════════════════════════════════
class StackNode {
public:
    BorrowRecord data;
    StackNode*   next;
    StackNode(const BorrowRecord& r) : data(r), next(nullptr) {}
};

class BorrowStack {
    StackNode* top_;
    int        size_;
public:
    BorrowStack() : top_(nullptr), size_(0) {}
    ~BorrowStack() { while (!empty()) pop(); }

    void push(const BorrowRecord& r) {
        StackNode* node = new StackNode(r);
        node->next = top_;
        top_  = node;
        size_++;
    }

    BorrowRecord pop() {
        if (empty()) throw runtime_error("Stack underflow");
        StackNode* tmp = top_;
        BorrowRecord d = tmp->data;
        top_  = top_->next;
        delete tmp;
        size_--;
        return d;
    }

    BorrowRecord& peek() {
        if (empty()) throw runtime_error("Stack empty");
        return top_->data;
    }

    bool empty() const { return top_ == nullptr; }
    int  size()  const { return size_; }
};

// ══════════════════════════════════════════════════════════════════════════════
//  Queue  –  borrow request queue (FIFO waiting list)
// ══════════════════════════════════════════════════════════════════════════════
class QueueNode {
public:
    BorrowRecord data;
    QueueNode*   next;
    QueueNode(const BorrowRecord& r) : data(r), next(nullptr) {}
};

class BorrowQueue {
    QueueNode* front_;
    QueueNode* rear_;
    int        size_;
public:
    BorrowQueue() : front_(nullptr), rear_(nullptr), size_(0) {}
    ~BorrowQueue() { while (!empty()) dequeue(); }

    void enqueue(const BorrowRecord& r) {
        QueueNode* node = new QueueNode(r);
        if (!rear_) { front_ = rear_ = node; }
        else        { rear_->next = node; rear_ = node; }
        size_++;
    }

    BorrowRecord dequeue() {
        if (empty()) throw runtime_error("Queue underflow");
        QueueNode* tmp = front_;
        BorrowRecord d = tmp->data;
        front_ = front_->next;
        if (!front_) rear_ = nullptr;
        delete tmp;
        size_--;
        return d;
    }

    BorrowRecord& frontItem() {
        if (empty()) throw runtime_error("Queue empty");
        return front_->data;
    }

    bool empty() const { return front_ == nullptr; }
    int  size()  const { return size_; }

    void printAll() const {
        if (empty()) { cout << "  No pending requests.\n"; return; }
        cout << left
             << setw(6)  << "RecID"
             << setw(8)  << "User"
             << setw(8)  << "Book"
             << setw(28) << "Title"
             << "Date\n";
        printLine();
        QueueNode* cur = front_;
        while (cur) { cur->data.print(); cur = cur->next; }
    }
};

// ══════════════════════════════════════════════════════════════════════════════
//  BorrowHistory  –  array-based, sorted by recordId using merge sort
// ══════════════════════════════════════════════════════════════════════════════
class BorrowHistory {
    Array<BorrowRecord> records;

    // Merge sort for history view
    void merge(Array<BorrowRecord>& arr, int l, int m, int r) {
        Array<BorrowRecord> left(arr.begin()+l, arr.begin()+m+1);
        Array<BorrowRecord> right(arr.begin()+m+1, arr.begin()+r+1);
        int i=0, j=0, k=l;
        while (i<(int)left.size() && j<(int)right.size())
            arr[k++] = (left[i].recordId <= right[j].recordId) ? left[i++] : right[j++];
        while (i<(int)left.size())  arr[k++] = left[i++];
        while (j<(int)right.size()) arr[k++] = right[j++];
    }

    void mergeSort(Array<BorrowRecord>& arr, int l, int r) {
        if (l >= r) return;
        int m = (l+r)/2;
        mergeSort(arr, l, m);
        mergeSort(arr, m+1, r);
        merge(arr, l, m, r);
    }

public:
    void add(const BorrowRecord& r) { records.push_back(r); }

    void sortByDate() {
        if (records.size() > 1)
            mergeSort(records, 0, (int)records.size()-1);
    }

    BorrowRecord* findActive(int userId, int bookId) {
        for (auto& r : records)
            if (r.userId == userId && r.bookId == bookId && !r.returned)
                return &r;
        return nullptr;
    }

    BorrowRecord* findById(int recId) {
        for (auto& r : records)
            if (r.recordId == recId) return &r;
        return nullptr;
    }

    void printAll() const {
        if (records.empty()) { cout << "  No borrow history.\n"; return; }
        cout << left
             << setw(6)  << "RecID"
             << setw(8)  << "User"
             << setw(8)  << "Book"
             << setw(28) << "Title"
             << setw(12) << "Borrowed"
             << setw(12) << "Returned"
             << "Due Fine\n";
        printLine();
        for (auto& r : records) r.print();
    }

    void printForUser(int uid) const {
        bool found = false;
        cout << left
             << setw(6)  << "RecID"
             << setw(8)  << "User"
             << setw(8)  << "Book"
             << setw(28) << "Title"
             << setw(12) << "Borrowed"
             << setw(12) << "Returned"
             << "Due Fine\n";
        printLine();
        for (auto& r : records)
            if (r.userId == uid) { r.print(); found = true; }
        if (!found) cout << "  No history for this user.\n";
    }

    double totalFineForUser(int uid) const {
        double total = 0;
        for (auto& r : records)
            if (r.userId == uid) total += r.dueFine();
        return total;
    }

    double unpaidFine(int uid) const {
        double total = 0;
        for (auto& r : records)
            if (r.userId == uid) total += r.dueFine();
        return total;
    }

    int activeCount(int uid) const {
        int c = 0;
        for (auto& r : records)
            if (r.userId == uid && !r.returned) c++;
        return c;
    }

    Array<BorrowRecord>& getAll() { return records; }

    int nextId() const {
        int mx = 0;
        for (auto& r : records) mx = max(mx, r.recordId);
        return mx + 1;
    }
};
