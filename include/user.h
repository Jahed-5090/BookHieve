#pragma once
#include "globals.h"
#include <vector>

// ══════════════════════════════════════════════════════════════════════════════
//  User  –  data entity
// ══════════════════════════════════════════════════════════════════════════════
struct User {
    int    id;
    string name;
    string email;
    string password;   // plain-text (demo scope)
    bool   isAdmin;
    int    borrowCount;

    User() : id(0), isAdmin(false), borrowCount(0) {}
    User(int id, const string& n, const string& e, const string& p, bool admin)
        : id(id), name(n), email(e), password(p), isAdmin(admin), borrowCount(0) {}

    void print() const {
        cout << CYAN << left
             << setw(6)  << id
             << setw(24) << name.substr(0,23)
             << setw(30) << email.substr(0,29)
             << (isAdmin ? GREEN"Admin" : WHITE"User ")
             << RESET
             << "  borrows=" << borrowCount << "\n";
    }

    string serialize() const {
        return to_string(id) + "|" + name + "|" + email + "|" +
               password + "|" + (isAdmin ? "1" : "0") + "|" +
               to_string(borrowCount);
    }

    static User deserialize(const string& line) {
        User u;
        stringstream ss(line);
        string tok;
        getline(ss, tok,  '|'); u.id          = stoi(tok);
        getline(ss, u.name,    '|');
        getline(ss, u.email,   '|');
        getline(ss, u.password,'|');
        getline(ss, tok,  '|'); u.isAdmin     = tok == "1";
        getline(ss, tok,  '|'); u.borrowCount = stoi(tok);
        return u;
    }
};

// ══════════════════════════════════════════════════════════════════════════════
//  Singly Linked List node for users
// ══════════════════════════════════════════════════════════════════════════════
struct UserNode {
    User      data;
    UserNode* next;
    UserNode(const User& u) : data(u), next(nullptr) {}
};

// ══════════════════════════════════════════════════════════════════════════════
//  UserList  –  Singly Linked List
//  Used for membership management (insert, delete, traverse)
// ══════════════════════════════════════════════════════════════════════════════
class UserList {
    UserNode* head;
    int       size_;

public:
    UserList() : head(nullptr), size_(0) {}

    ~UserList() {
        while (head) {
            UserNode* tmp = head;
            head = head->next;
            delete tmp;
        }
    }

    // Insert at end
    void insertEnd(const User& u) {
        UserNode* node = new UserNode(u);
        if (!head) { head = node; }
        else {
            UserNode* cur = head;
            while (cur->next) cur = cur->next;
            cur->next = node;
        }
        size_++;
    }

    // Delete by user id
    bool remove(int id) {
        if (!head) return false;
        if (head->data.id == id) {
            UserNode* tmp = head;
            head = head->next;
            delete tmp;
            size_--;
            return true;
        }
        UserNode* cur = head;
        while (cur->next && cur->next->data.id != id)
            cur = cur->next;
        if (!cur->next) return false;
        UserNode* tmp = cur->next;
        cur->next = tmp->next;
        delete tmp;
        size_--;
        return true;
    }

    // Search by email → returns pointer or nullptr
    User* findByEmail(const string& email) {
        UserNode* cur = head;
        while (cur) {
            if (cur->data.email == email) return &cur->data;
            cur = cur->next;
        }
        return nullptr;
    }

    // Search by id
    User* findById(int id) {
        UserNode* cur = head;
        while (cur) {
            if (cur->data.id == id) return &cur->data;
            cur = cur->next;
        }
        return nullptr;
    }

    void printAll() const {
        if (!head) { cout << RED << "  No members found.\n" << RESET; return; }
        cout << BOLD << left
             << setw(6)  << "ID"
             << setw(24) << "Name"
             << setw(30) << "Email"
             << "Role       Borrows\n" << RESET;
        printLine();
        UserNode* cur = head;
        while (cur) { cur->data.print(); cur = cur->next; }
    }

    vector<User> getAll() const {
        vector<User> v;
        UserNode* cur = head;
        while (cur) { v.push_back(cur->data); cur = cur->next; }
        return v;
    }

    int  size()  const { return size_; }
    bool empty() const { return head == nullptr; }

    // Get next available user ID
    int nextId() const {
        int mx = 0;
        UserNode* cur = head;
        while (cur) { mx = max(mx, cur->data.id); cur = cur->next; }
        return mx + 1;
    }
};
