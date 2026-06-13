#include "../include/library.h"

// ------------------------------------------------------------------------------
//  Admin Panel
// ------------------------------------------------------------------------------

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
                cout << "\n  Book already exists. Added " << copies 
                     << " copies to existing book (ID: " << b.id << ").\n";
                pauseScreen();
                return;
            }
        }
    }

    int id = lib.nextBookId();

    // E6: Prompt and set condition when a new book is added
    BookCondition cond = condMgr.promptCondition();
    condMgr.setCondition(to_string(id), cond);
    lib.catalogue.insert(Book(id, title, author, genre, year, copies));
    lib.save();
    cout << "\n  Book added with ID " << id << "\n"
        ;
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
        cout << "  Book not found.\n"
            ;
        pauseScreen();
        return;
    }
    if (node->data.availableCopies < node->data.totalCopies)
    {
        cout << "  Warning: some copies are currently borrowed!\n"
            ;
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
    cout << "  Book removed.\n"
        ;
    pauseScreen();
}

static void editBook(Library &lib)
{
    printTitle("Edit Book");
    int id;
    cout << "  Enter Book ID to edit (0 to cancel): ";
    id = getIntInput();
    if (id == 0) return;

    BSTNode *node = lib.catalogue.search(id);
    if (!node)
    {
        cout << "  Book not found.\n"
            ;
        pauseScreen();
        return;
    }

    // Display current book information
    cout << "\n  Current Book Information:" << "\n";
    cout << left << setw(6) << "ID" << setw(11) << "" << setw(28) << "Title"
         << setw(22) << "Author" << setw(18) << "Genre"
         << setw(6) << "Year" << "Avail/Total\n";
    printLine();
    node->data.print();

    // Show edit options
    while (true)
    {
        cout << "\n  What would you like to edit?\n"
             << "  1. Title\n"
             << "  2. Author\n"
             << "  3. Year\n"
             << "  4. Genre\n"
             << "  5. Quantity\n"
             << "  0. Done\n"
             << "  Choice: ";
        int ch = getMenuChoice(0, 5);
        if (ch == 0) break;

        if (ch == 1)
        {
            cout << "  New Title (current: " << node->data.title << "): ";
            string newTitle;
            getline(cin, newTitle);
            if (!newTitle.empty())
            {
                node->data.title = newTitle;
                cout << "  Title updated.\n";
            }
        }
        else if (ch == 2)
        {
            cout << "  New Author (current: " << node->data.author << "): ";
            string newAuthor;
            getline(cin, newAuthor);
            if (!newAuthor.empty())
            {
                node->data.author = newAuthor;
                cout << "  Author updated.\n";
            }
        }
        else if (ch == 3)
        {
            cout << "  New Year (current: " << node->data.year << "): ";
            int newYear = getIntInput();
            if (newYear > 0)
            {
                node->data.year = newYear;
                cout << "  Year updated.\n";
            }
        }
        else if (ch == 4)
        {
            cout << "  New Genre (current: " << node->data.genre << "): ";
            string newGenre;
            getline(cin, newGenre);
            if (!newGenre.empty())
            {
                node->data.genre = newGenre;
                cout << "  Genre updated.\n";
            }
        }
        else if (ch == 5)
        {
            cout << "\n  Current Quantity:\n"
                 << "    Total Copies    : " << node->data.totalCopies << "\n"
                 << "    Available Copies: " << node->data.availableCopies << "\n";
            cout << "\n  How would you like to adjust quantity?\n"
                 << "  1. Set total copies\n"
                 << "  2. Set available copies\n"
                 << "  3. Add copies\n"
                 << "  4. Remove copies\n"
                 << "  0. Back\n"
                 << "  Choice: ";
            int qch = getMenuChoice(0, 4);
            if (qch == 1)
            {
                cout << "  New Total Copies (current: " << node->data.totalCopies << "): ";
                int newTotal = getIntInput();
                if (newTotal > 0 && newTotal >= node->data.availableCopies)
                {
                    node->data.totalCopies = newTotal;
                    cout << "  Total copies updated.\n";
                }
                else if (newTotal < node->data.availableCopies)
                {
                    cout << "  Error: Total copies cannot be less than available copies.\n";
                }
            }
            else if (qch == 2)
            {
                cout << "  New Available Copies (current: " << node->data.availableCopies << "): ";
                int newAvail = getIntInput();
                if (newAvail >= 0 && newAvail <= node->data.totalCopies)
                {
                    node->data.availableCopies = newAvail;
                    cout << "  Available copies updated.\n";
                }
                else if (newAvail > node->data.totalCopies)
                {
                    cout << "  Error: Available copies cannot exceed total copies.\n";
                }
            }
            else if (qch == 3)
            {
                cout << "  How many copies to add? ";
                int add = getIntInput();
                if (add > 0)
                {
                    node->data.totalCopies += add;
                    node->data.availableCopies += add;
                    cout << "  Added " << add << " copies. New total: " << node->data.totalCopies << "\n";
                }
            }
            else if (qch == 4)
            {
                cout << "  How many copies to remove? ";
                int remove = getIntInput();
                if (remove > 0 && remove <= node->data.totalCopies && remove <= node->data.availableCopies)
                {
                    node->data.totalCopies -= remove;
                    node->data.availableCopies -= remove;
                    cout << "  Removed " << remove << " copies. New total: " << node->data.totalCopies << "\n";
                }
                else if (remove > node->data.totalCopies || remove > node->data.availableCopies)
                {
                    cout << "  Error: Cannot remove more copies than available.\n";
                }
            }
        }
    }

    lib.save();
    cout << "\n  Book information saved.\n";
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
                cout << left << setw(6) << "ID" << setw(11) << "" << setw(28) << "Title"
                     << setw(22) << "Author" << setw(18) << "Genre"
                     << setw(6) << "Year" << "Avail/Total\n"
                    ;
                printLine();
                n->data.print();
            }
            else
                cout << "  Not found.\n"
                    ;
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
            cout << "  Invalid choice.\n";
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
            cout << "  Invalid choice.\n";
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
        cout << "  User not found.\n"
            ;
        pauseScreen();
        return;
    }
    if (u->isAdmin)
    {
        cout << "  Cannot remove admin.\n"
            ;
        pauseScreen();
        return;
    }

    if (u->borrowCount > 0)
    {
        cout << "  Cannot remove member: User has " << u->borrowCount << " active borrowed book(s).\n"
            ;
        pauseScreen();
        return;
    }

    double fine = lib.currentFine(id);
    if (fine > 0)
    {
        cout << "  Cannot remove member: User has an outstanding fine of BDT " << fixed << setprecision(2) << fine << ".\n"
            ;
        pauseScreen();
        return;
    }

    lib.members.remove(id);
    lib.save();
    cout << "  Member removed.\n"
        ;
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

        lib.rebuildHeaps();
        // E3 & E5: Show system overviews for the admin
        fineSys.showAdminFineSummary();
        OverdueWarningSystem::showAdminWarnings(2);

        cout << "  1. View top fines\n"
             << "  2. View all fines\n" /*heap sort*/
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
            Array<FineEntry> all;
            for (auto &u : lib.members.getAll())
            {
                double fine = lib.currentFine(u.id);
                if (fine > 0)
                    all.push_back(FineEntry(u.id, u.name, fine));
            }
            if (all.empty())
            {
                cout << "  No outstanding fines.\n"
                    ;
            }
            else
            {
                cout << "  Fine list:\n"
                    ;
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
            cout << "  Processing request for book ID " << req.bookId
                 << " by user " << req.userId << "\n"
                ;
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
             << "  3.  Edit Book\n"
             << "  4.  Search Book\n"
             << "  5.  View Catalogue\n"
             << "  6.  View Memberships\n"
             << "  7.  Remove Member\n"
             << "  8.  View Borrow History\n"
             << "  9.  Fine History\n"
             << "  10. Books Due Soon\n"
             << "  11. Wait Queue\n"
             << "  12. Manage Worn Books\n"
             << "  13. Update Borrow Limits\n"
             << "  0.  Sign Out\n";
        printLine();
        cout << "  Choice: ";
        int ch = getMenuChoice(0, 13);
        switch (ch)
        {
        case 1:
            addBook(lib);
            break;
        case 2:
            removeBook(lib);
            break;
        case 3:
            editBook(lib);
            break;
        case 4:
            searchBook(lib);
            break;
        case 5:
            viewCatalogue(lib);
            break;
        case 6:
            viewMemberships(lib);
            break;
        case 7:
            removeMember(lib);
            break;
        case 8:
            viewBorrowHistory(lib);
            break;
        case 9:
            viewFineHistory(lib);
            break;

        case 10:
            dueSoonMenu(lib);
            break;
        case 11:
            waitQueueMenu(lib);
            break;
        case 12:
            condMgr.showWornBooks();
            pauseScreen();
            break; // E6
        case 13:
            limitMgr.adminUpdateLimit();
            pauseScreen();
            break; // E7
        case 0:
            lib.currentUser = nullptr;
            return;
        }
    }
}