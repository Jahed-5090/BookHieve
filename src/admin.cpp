#include "../include/library.h"

// ══════════════════════════════════════════════════════════════════════════════
//  Admin Panel
// ══════════════════════════════════════════════════════════════════════════════

static void addBook(Library &lib)
{
    printTitle("Add Book");
    string title, author, genre;
    int year, copies;
    cout << "  Title (0 to cancel): ";
    getline(cin, title);
    if (title == "0") return;
    cout << "  Author  : ";
    getline(cin, author);
    cout << "  Genre   : ";
    getline(cin, genre);
    cout << "  Year    : ";
    year = getIntInput();
    cout << "  Copies  : ";
    copies = getIntInput();

    // Check if book already exists (case-insensitive match on title and author)
    for (auto &b : lib.catalogue.getAll())
    {
        string t1 = b.title;
        string t2 = title;
        for (char &c : t1) c = tolower(c);
        for (char &c : t2) c = tolower(c);
        
        string a1 = b.author;
        string a2 = author;
        for (char &c : a1) c = tolower(c);
        for (char &c : a2) c = tolower(c);

        if (t1 == t2 && a1 == a2)
        {
            BSTNode *node = lib.catalogue.search(b.id);
            if (node)
            {
                node->data.totalCopies += copies;
                node->data.availableCopies += copies;
                lib.save();
                cout << GREEN << "\n  Book already exists. Added " << copies 
                     << " copies to existing book (ID: " << b.id << ").\n" << RESET;
                pauseScreen();
                return;
            }
        }
    }

    int id = lib.nextBookId();

    // E6: Prompt and set condition when a new book is added
    BookCondition cond = condMgr.promptCondition();
    condMgr.setCondition(std::to_string(id), cond);
    lib.catalogue.insert(Book(id, title, author, genre, year, copies));
    lib.save();
    cout << GREEN << "\n  Book added with ID " << id << "\n"
         << RESET;
    pauseScreen();
}

static void removeBook(Library &lib)
{
    printTitle("Remove Book");
    int id;
    cout << "  Enter Book ID to remove (0 to cancel): ";
    id = getIntInput();
    if (id == 0) return;
    BSTNode *node = lib.catalogue.search(id);
    if (!node)
    {
        cout << RED << "  Book not found.\n"
             << RESET;
        pauseScreen();
        return;
    }
    if (node->data.availableCopies < node->data.totalCopies)
    {
        cout << YELLOW << "  Warning: some copies are currently borrowed!\n"
             << RESET;
        cout << "  Continue? (y/n): ";
        char c;
        cin >> c;
        cin.ignore();
        if (c != 'y' && c != 'Y')
        {
            pauseScreen();
            return;
        }
    }
    lib.catalogue.remove(id);
    lib.save();
    cout << GREEN << "  Book removed.\n"
         << RESET;
    pauseScreen();
}

static void searchBook(Library &lib)
{
    while (true)
    {
        printTitle("Search Book");
        cout << "  1. By Title\n"
             << "  2. By Author\n"
             << "  3. By ID\n"
             << "  4. By Genre\n"
             << "  0. Go Back\n"
             << "  Choice: ";
        int ch = getMenuChoice(0, 4);
        if (ch == 0) return;
        if (ch == 1)
        {
            cout << "  Keyword: ";
            string kw;
            getline(cin, kw);
            lib.catalogue.searchTitle(kw);
        }
        else if (ch == 2)
        {
            cout << "  Keyword: ";
            string kw;
            getline(cin, kw);
            lib.catalogue.searchAuthor(kw);
        }
        else if (ch == 3)
        {
            cout << "  Book ID: ";
            int id = getIntInput();
            BSTNode *n = lib.catalogue.search(id);
            if (n)
            {
                cout << BOLD << left << setw(6) << "ID" << setw(11) << "" << setw(28) << "Title"
                     << setw(22) << "Author" << setw(18) << "Genre"
                     << setw(6) << "Year" << "Avail/Total\n"
                     << RESET;
                printLine();
                n->data.print();
            }
            else
                cout << RED << "  Not found.\n"
                     << RESET;
        }
        else if (ch == 4)
        {
            cout << "  Keyword: ";
            string kw;
            getline(cin, kw);
            lib.catalogue.searchGenre(kw);
        }
        else
        {
            cout << RED << "  Invalid choice.\n" << RESET;
        }
        pauseScreen();
    }
}

// Data Structures: BST (inorder traversal for default view), Bubble Sort, Insertion Sort,
//                  Selection Sort, Quick Sort, Merge Sort (user-selectable for sorted views)
static void viewCatalogue(Library &lib)
{
    while (true)
    {
        printTitle("View Catalogue");
        cout << "  Sort by:\n"
             << "  1. Title\n"
             << "  2. Year\n"
             << "  3. Author\n"
             << "  4. Availability\n"
             << "  5. Genre\n"
             << "  6. Default (by ID)\n"
             << "  0. Go Back\n"
             << "  Choice: ";
        int ch = getIntInput();
        if (ch == 0) return;
        if (ch == 6)
        {
            lib.catalogue.printAll();
        }
        else if (ch == 1)
        {
            auto books = lib.catalogue.getAll();
            printSortedCatalogue(books, 4);
        }
        else if (ch >= 2 && ch <= 3)
        {
            auto books = lib.catalogue.getAll();
            printSortedCatalogue(books, ch);
        }
        else if (ch == 4)
        {
            auto books = lib.catalogue.getAll();
            printSortedCatalogue(books, 5);
        }
        else if (ch == 5)
        {
            auto books = lib.catalogue.getAll();
            printSortedCatalogue(books, 6);
        }
        else
        {
            cout << RED << "  Invalid choice.\n" << RESET;
        }
        pauseScreen();
    }
}

static void viewMemberships(Library &lib)
{
    printTitle("View Memberships");
    lib.members.printAll();
    pauseScreen();
}

static void removeMember(Library &lib)
{
    printTitle("Remove Member");
    int id;
    cout << "  User ID to remove (0 to cancel): ";
    id = getIntInput();
    if (id == 0) return;
    User *u = lib.members.findById(id);
    if (!u)
    {
        cout << RED << "  User not found.\n"
             << RESET;
        pauseScreen();
        return;
    }
    if (u->isAdmin)
    {
        cout << RED << "  Cannot remove admin.\n"
             << RESET;
        pauseScreen();
        return;
    }
    lib.members.remove(id);
    lib.save();
    cout << GREEN << "  Member removed.\n"
         << RESET;
    pauseScreen();
}

static void viewBorrowHistory(Library &lib)
{
    printTitle("Borrow History");
    lib.history.sortByDate();
    lib.history.printAll();
    pauseScreen();
}

static void viewFineHistory(Library &lib)
{
    while (true)
    {
        printTitle("Fine History");

        // E3 & E5: Show system overviews for the admin
        fineSys.showAdminFineSummary();
        OverdueWarningSystem::showAdminWarnings(2);

        cout << "  1. View top fines\n"
             << "  2. Heap-sort all fines\n"
             << "  0. Go Back\n"
             << "  Choice: ";
        int ch = getMenuChoice(0, 2);
        if (ch == 0) return;
        if (ch == 1)
        {
            lib.fineHeap.printTopFines(10);
        }
        else if (ch == 2)
        {
            // Collect all user fines
            vector<FineEntry> all;
            for (auto &u : lib.members.getAll())
            {
                double fine = lib.currentFine(u.id);
                if (fine > 0)
                    all.push_back(FineEntry(u.id, u.name, fine));
            }
            if (all.empty())
            {
                cout << GREEN << "  No outstanding fines.\n"
                     << RESET;
            }
            else
            {
                cout << YELLOW << "  Fine list:\n"
                     << RESET;
                lib.fineHeap.heapSort(all);
            }
        }
        pauseScreen();
    }
}


// Data Structures: Min-Heap (priority queue ordered by due date)
static void dueSoonMenu(Library &lib)
{
    printTitle("Books Due Soon");
    lib.dueHeap.printUpcoming(10);
    pauseScreen();
}

// Data Structures: Queue (FIFO wait list for borrow requests)
static void waitQueueMenu(Library &lib)
{
    printTitle("Borrow Wait Queue");
    lib.waitQueue.printAll();
    if (!lib.waitQueue.empty())
    {
        cout << "\n  Process next request? (y/n): ";
        char c;
        cin >> c;
        cin.ignore();
        if (c == 'y' || c == 'Y')
        {
            BorrowRecord req = lib.waitQueue.dequeue();
            cout << GREEN << "  Processing request for book ID " << req.bookId
                 << " by user " << req.userId << "\n"
                 << RESET;
            lib.borrowBook(req.userId, req.bookId);
        }
    }
    pauseScreen();
}

void runAdminPanel(Library &lib)
{
    while (true)
    {
        printTitle("Admin Panel  [" + lib.currentUser->name + "]");
        cout << "  1.  Add Book\n"
             << "  2.  Remove Book\n"
             << "  3.  Search Book\n"
             << "  4.  View Catalogue\n"
             << "  5.  View Memberships\n"
             << "  6.  Remove Member\n"
             << "  7.  View Borrow History\n"
             << "  8.  Fine History\n"
             << "  9.  Books Due Soon\n"
             << "  10. Wait Queue\n"
             << "  11. Manage Worn Books\n"
             << "  12. Update Borrow Limits\n"
             << "  0.  Sign Out\n";
        printLine();
        cout << "  Choice: ";
        int ch = getMenuChoice(0, 12);
        switch (ch)
        {
        case 1:
            addBook(lib);
            break;
        case 2:
            removeBook(lib);
            break;
        case 3:
            searchBook(lib);
            break;
        case 4:
            viewCatalogue(lib);
            break;
        case 5:
            viewMemberships(lib);
            break;
        case 6:
            removeMember(lib);
            break;
        case 7:
            viewBorrowHistory(lib);
            break;
        case 8:
            viewFineHistory(lib);
            break;

        case 9:
            dueSoonMenu(lib);
            break;
        case 10:
            waitQueueMenu(lib);
            break;
        case 11:
            condMgr.showWornBooks();
            pauseScreen();
            break; // E6
        case 12:
            limitMgr.adminUpdateLimit();
            pauseScreen();
            break; // E7
        case 0:
            lib.currentUser = nullptr;
            return;
        }
    }
}