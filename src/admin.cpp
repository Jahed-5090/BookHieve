#include "../include/library.h"

// ══════════════════════════════════════════════════════════════════════════════
//  Admin Panel
// ══════════════════════════════════════════════════════════════════════════════

static void addBook(Library &lib)
{
    printTitle("Add Book");
    string title, author, genre;
    int year, copies;
    cin.ignore();
    cout << "  Title   : ";
    getline(cin, title);
    cout << "  Author  : ";
    getline(cin, author);
    cout << "  Genre   : ";
    getline(cin, genre);
    cout << "  Year    : ";
    cin >> year;
    cin.ignore();
    cout << "  Copies  : ";
    cin >> copies;
    cin.ignore();

    int id = lib.nextBookId();

    // E6: Prompt and set condition when a new book is added
    BookCondition cond = condMgr.promptCondition();
    condMgr.setCondition(std::to_string(id), cond);
    lib.catalogue.insert(Book(id, title, author, genre, year, copies));
    lib.save();
    cout << GREEN << "\n  ✓ Book added with ID " << id << "\n"
         << RESET;
    pauseScreen();
}

static void removeBook(Library &lib)
{
    printTitle("Remove Book");
    int id;
    cout << "  Enter Book ID to remove: ";
    cin >> id;
    cin.ignore();
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
    cout << GREEN << "  ✓ Book removed.\n"
         << RESET;
    pauseScreen();
}

static void searchBook(Library &lib)
{
    printTitle("Search Book");
    cout << "  1. Search by Title\n"
         << "  2. Search by Author\n"
         << "  3. Search by ID\n"
         << "  Choice: ";
    int ch;
    cin >> ch;
    cin.ignore();
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
    else
    {
        cout << "  Book ID: ";
        int id;
        cin >> id;
        cin.ignore();
        BSTNode *n = lib.catalogue.search(id);
        if (n)
        {
            cout << BOLD << left << setw(6) << "ID" << setw(32) << "Title"
                 << setw(22) << "Author" << setw(14) << "Genre"
                 << setw(6) << "Year" << "Avail/Total\n"
                 << RESET;
            printLine();
            n->data.print();
        }
        else
            cout << RED << "  Not found.\n"
                 << RESET;
    }
    pauseScreen();
}

// Data Structures: BST (inorder traversal for default view), Bubble Sort, Insertion Sort,
//                  Selection Sort, Quick Sort, Merge Sort (user-selectable for sorted views)
static void viewCatalogue(Library &lib)
{
    printTitle("View Catalogue");
    cout << "  Sort by:\n"
         << "  1. Title (Bubble Sort)\n"
         << "  2. Year  (Insertion Sort)\n"
         << "  3. Author (Selection Sort)\n"
         << "  4. Title (Quick Sort)\n"
         << "  5. Availability (Merge Sort)\n"
         << "  6. Default (BST inorder by ID)\n"
         << "  Choice: ";
    int ch;
    cin >> ch;
    cin.ignore();
    if (ch == 6 || ch < 1 || ch > 6)
    {
        lib.catalogue.printAll();
    }
    else
    {
        auto books = lib.catalogue.getAll();
        printSortedCatalogue(books, ch);
    }
    pauseScreen();
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
    cout << "  User ID to remove: ";
    cin >> id;
    cin.ignore();
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
    cout << GREEN << "  ✓ Member removed.\n"
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
    printTitle("Fine / Jorimana History");

    // E3 & E5: Show system overviews for the admin
    fineSys.showAdminFineSummary();
    OverdueWarningSystem::showAdminWarnings(2);

    cout << "  1. View top fines (Max-Heap)\n"
         << "  2. View all fine cumulative stats (BIT)\n"
         << "  3. Heap-sort all fines\n"
         << "  Choice: ";
    int ch;
    cin >> ch;
    cin.ignore();
    if (ch == 1)
    {
        lib.fineHeap.printTopFines(10);
    }
    else if (ch == 2)
    {
        lib.fineBIT.printStats(lib.members.nextId() - 1);
    }
    else if (ch == 3)
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
            cout << YELLOW << "  Fine list (Heap Sort – ascending):\n"
                 << RESET;
            lib.fineHeap.heapSort(all);
        }
    }
    pauseScreen();
}

// Data Structures: Directed Graph (adjacency list); algorithms: BFS, DFS, Topological Sort
static void genreGraphMenu(Library &lib)
{
    printTitle("Genre Graph");
    lib.genreGraph.printGraph();
    cout << "\n  1. BFS from a genre\n"
         << "  2. DFS from a genre\n"
         << "  3. Topological Sort\n"
         << "  4. Back\n"
         << "  Choice: ";
    int ch;
    cin >> ch;
    cin.ignore();
    if (ch == 1)
    {
        cout << "  Start genre: ";
        string g;
        getline(cin, g);
        lib.genreGraph.bfs(g);
    }
    else if (ch == 2)
    {
        cout << "  Start genre: ";
        string g;
        getline(cin, g);
        lib.genreGraph.dfs(g);
    }
    else if (ch == 3)
    {
        lib.genreGraph.topoSort();
    }
    pauseScreen();
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
             << "  8.  Fine / Jorimana History\n"
             << "  9.  Genre Graph\n"
             << "  10. Books Due Soon\n"
             << "  11. Wait Queue\n"
             << "  12. Manage Worn Books\n"
             << "  13. Update Borrow Limits\n"
             << "  0.  Sign Out\n";
        printLine();
        cout << "  Choice: ";
        int ch;
        cin >> ch;
        cin.ignore();
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
            genreGraphMenu(lib);
            break;
        case 10:
            dueSoonMenu(lib);
            break;
        case 11:
            waitQueueMenu(lib);
            break;
        case 12:
            condMgr.showWornBooks();
            break; // E6
        case 13:
            limitMgr.adminUpdateLimit();
            break; // E7
        case 0:
            lib.currentUser = nullptr;
            return;
        default:
            cout << RED << "  Invalid choice.\n"
                 << RESET;
        }
    }
}