#pragma once
#include "globals.h"
#include "dynamic_array.h"

// ══════════════════════════════════════════════════════════════════════════════
//  User  –  data entity
// ══════════════════════════════════════════════════════════════════════════════
class User {
public:
    int    id;
    string name;
    string email;
    string password;   // plain-text (demo scope)
    bool   isAdmin;
    int    borrowCount;

    User() : id(0), isAdmin(false), borrowCount(0) {}
    User(int id, const string& n, const string& e, const string& p, bool admin)
        : id(id), name(n), email(e), password(p), isAdmin(admin), borrowCount(0) {}

    static bool isEncoded(const string& s) {
        return !s.empty() && (s[0] == '@' || s[0] == '#');
    }

    static string restoreDomain(const string& compactDomain) {
        if (compactDomain.empty()) return compactDomain;

        auto endsWith = [&](const string& suffix) {
            return compactDomain.size() > suffix.size() &&
                   compactDomain.compare(compactDomain.size() - suffix.size(),
                                         suffix.size(), suffix) == 0;
        };

        const Array<string> commonTlds = {
            "com", "net", "org", "edu", "gov", "io", "co", "uk", "us", "bd"
        };

        for (auto& tld : commonTlds) {
            if (endsWith(tld) && compactDomain.size() > tld.size()) {
                return compactDomain.substr(0, compactDomain.size() - tld.size()) + "." + tld;
            }
        }

        if (compactDomain.size() > 3) {
            return compactDomain.substr(0, compactDomain.size() - 3) + "." +
                   compactDomain.substr(compactDomain.size() - 3);
        }
        return compactDomain;
    }

    static string encodeCredential(const string& s) {
        if (isEncoded(s)) return s; // already encoded
        if (s.empty()) return "#";

        size_t atPos = s.find('@');
        if (atPos != string::npos) {
            string local = s.substr(0, atPos);
            string domain = s.substr(atPos + 1);
            string compact;
            for (char c : domain) {
                if (c != '.') compact.push_back(c);
            }
            return string("@") + local + "." + compact;
        }
        return string("#") + s;
    }

    static string decodeCredential(const string& s) {
        if (s.empty()) return s;
        if (s[0] == '@') {
            size_t delim = s.find('.', 1);
            if (delim == string::npos) return s.substr(1);
            string local = s.substr(1, delim - 1);
            string compactDomain = s.substr(delim + 1);
            return local + "@" + restoreDomain(compactDomain);
        }
        if (s[0] == '#') {
            return s.substr(1);
        }
        return s;
    }

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
        return to_string(id) + "|" + name + "|" +
               encodeCredential(email) + "|" +
               encodeCredential(password) + "|" +
               (isAdmin ? "1" : "0") + "|" +
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

        u.email    = decodeCredential(u.email);
        u.password = decodeCredential(u.password);
        return u;
    }
};

// ══════════════════════════════════════════════════════════════════════════════
//  Singly Linked List node for users
// ══════════════════════════════════════════════════════════════════════════════
class UserNode {
public:
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

    Array<User> getAll() const {
        Array<User> v;
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
