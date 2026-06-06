#pragma once
#include "globals.h"
#include <vector>

// ─── ADDED THESE TWO LINES HERE ───────────────────────────────────────────────
#include "book_condition.h"
extern ConditionManager condMgr;
// ──────────────────────────────────────────────────────────────────────────────

// ══════════════════════════════════════════════════════════════════════════════
//  Book  –  core data entity
// ══════════════════════════════════════════════════════════════════════════════
struct Book
{
    int id;
    string title;
    string author;
    string genre;
    int year;
    int totalCopies;
    int availableCopies;

    Book() : id(0), year(0), totalCopies(0), availableCopies(0) {}
    Book(int id, const string &t, const string &a, const string &g, int y, int tc)
        : id(id), title(t), author(a), genre(g), year(y),
          totalCopies(tc), availableCopies(tc) {}

    void print()
    {
        std::cout << std::left << std::setw(6) << id
                  << condMgr.getBadge(std::to_string(id)) << " " << std::setw(28) << title << std::setw(22) << author
                  << std::setw(14) << genre
                  << std::setw(6) << year
                  << availableCopies << "/" << totalCopies << "\n";
    }

    string serialize() const
    {
        return to_string(id) + "|" + title + "|" + author + "|" +
               genre + "|" + to_string(year) + "|" +
               to_string(totalCopies) + "|" + to_string(availableCopies);
    }

    static Book deserialize(const string &line)
    {
        Book b;
        stringstream ss(line);
        string tok;
        getline(ss, tok, '|');
        b.id = stoi(tok);
        getline(ss, b.title, '|');
        getline(ss, b.author, '|');
        getline(ss, b.genre, '|');
        getline(ss, tok, '|');
        b.year = stoi(tok);
        getline(ss, tok, '|');
        b.totalCopies = stoi(tok);
        getline(ss, tok, '|');
        b.availableCopies = stoi(tok);
        return b;
    }
};

// ══════════════════════════════════════════════════════════════════════════════
//  BST Node
// ══════════════════════════════════════════════════════════════════════════════
struct BSTNode
{
    Book data;
    BSTNode *left;
    BSTNode *right;
    BSTNode(const Book &b) : data(b), left(nullptr), right(nullptr) {}
};

// ══════════════════════════════════════════════════════════════════════════════
//  BookBST  –  keyed on book ID
//  Used for O(log n) search/insert/delete in catalogue
// ══════════════════════════════════════════════════════════════════════════════
class BookBST
{
    BSTNode *root;

    BSTNode *insert(BSTNode *node, const Book &b)
    {
        if (!node)
            return new BSTNode(b);
        if (b.id < node->data.id)
            node->left = insert(node->left, b);
        else if (b.id > node->data.id)
            node->right = insert(node->right, b);
        else
            node->data = b; // update
        return node;
    }

    BSTNode *minNode(BSTNode *node)
    {
        while (node->left)
            node = node->left;
        return node;
    }

    BSTNode *remove(BSTNode *node, int id)
    {
        if (!node)
            return nullptr;
        if (id < node->data.id)
            node->left = remove(node->left, id);
        else if (id > node->data.id)
            node->right = remove(node->right, id);
        else
        {
            if (!node->left)
            {
                BSTNode *r = node->right;
                delete node;
                return r;
            }
            if (!node->right)
            {
                BSTNode *l = node->left;
                delete node;
                return l;
            }
            BSTNode *succ = minNode(node->right);
            node->data = succ->data;
            node->right = remove(node->right, succ->data.id);
        }
        return node;
    }

    BSTNode *search(BSTNode *node, int id) const
    {
        if (!node || node->data.id == id)
            return node;
        if (id < node->data.id)
            return search(node->left, id);
        return search(node->right, id);
    }

    void inorder(BSTNode *node) const
    {
        if (!node)
            return;
        inorder(node->left);
        node->data.print();
        inorder(node->right);
    }

    void searchByTitle(BSTNode *node, const string &kw, bool &found) const
    {
        if (!node)
            return;
        searchByTitle(node->left, kw, found);
        string t = node->data.title;
        // case-insensitive contains
        string tl = t, kl = kw;
        for (char &c : tl)
            c = tolower(c);
        for (char &c : kl)
            c = tolower(c);
        if (tl.find(kl) != string::npos)
        {
            node->data.print();
            found = true;
        }
        searchByTitle(node->right, kw, found);
    }

    void searchByAuthor(BSTNode *node, const string &kw, bool &found) const
    {
        if (!node)
            return;
        searchByAuthor(node->left, kw, found);
        string a = node->data.author, kl = kw;
        for (char &c : a)
            c = tolower(c);
        for (char &c : kl)
            c = tolower(c);
        if (a.find(kl) != string::npos)
        {
            node->data.print();
            found = true;
        }
        searchByAuthor(node->right, kw, found);
    }

    void collectAll(BSTNode *node, vector<Book> &out) const
    {
        if (!node)
            return;
        collectAll(node->left, out);
        out.push_back(node->data);
        collectAll(node->right, out);
    }

    void destroyTree(BSTNode *node)
    {
        if (!node)
            return;
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }

public:
    BookBST() : root(nullptr) {}
    ~BookBST() { destroyTree(root); }

    void insert(const Book &b) { root = insert(root, b); }
    void remove(int id) { root = remove(root, id); }
    BSTNode *search(int id) const { return search(root, id); }

    void printAll() const
    {
        if (!root)
        {
            cout << RED << "  (catalogue is empty)\n"
                 << RESET;
            return;
        }
        cout << BOLD << left
             << setw(6) << "ID"
             << setw(32) << "Title"
             << setw(22) << "Author"
             << setw(14) << "Genre"
             << setw(6) << "Year"
             << "Avail/Total\n"
             << RESET;
        printLine();
        inorder(root);
    }

    void searchTitle(const string &kw) const
    {
        bool found = false;
        cout << BOLD << left
             << setw(6) << "ID"
             << setw(32) << "Title"
             << setw(22) << "Author"
             << setw(14) << "Genre"
             << setw(6) << "Year"
             << "Avail/Total\n"
             << RESET;
        printLine();
        searchByTitle(root, kw, found);
        if (!found)
            cout << RED << "  No books found.\n"
                 << RESET;
    }

    void searchAuthor(const string &kw) const
    {
        bool found = false;
        cout << BOLD << left
             << setw(6) << "ID"
             << setw(32) << "Title"
             << setw(22) << "Author"
             << setw(14) << "Genre"
             << setw(6) << "Year"
             << "Avail/Total\n"
             << RESET;
        printLine();
        searchByAuthor(root, kw, found);
        if (!found)
            cout << RED << "  No books found.\n"
                 << RESET;
    }

    vector<Book> getAll() const
    {
        vector<Book> v;
        collectAll(root, v);
        return v;
    }

    bool empty() const { return root == nullptr; }
};