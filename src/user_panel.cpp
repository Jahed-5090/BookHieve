#include "../include/library.h"

// ══════════════════════════════════════════════════════════════════════════════
//  User Panel
// ══════════════════════════════════════════════════════════════════════════════

static void userViewCatalogue(Library &lib)
{
    printTitle("Browse Catalogue");
    lib.catalogue.printAll();
    recEngine.displayRecommendations(to_string(lib.currentUser->id));
    pauseScreen();
}

static void userSearchBook(Library &lib)
{
    while (true)
    {
        printTitle("Search Book");
        cout << "  1. By Title\n  2. By Author\n  3. By ID\n  4. By Genre\n  0. Go Back\n  Choice: ";
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

        recEngine.displayRecommendations(to_string(lib.currentUser->id));
        pauseScreen();
    }
}

static void userBorrowBook(Library &lib)
{
    printTitle("Borrow a Book");
    int uid = lib.currentUser->id;
    int active = lib.history.activeCount(uid);

    limitMgr.showUsage(to_string(uid));
    cout << "  Active borrows: " << active << " / " << MAX_BORROW << "\n\n";

    cout << "  Enter Book ID to borrow (0 to cancel): ";
    int bid = getIntInput();
    if (bid == 0) return;

    BSTNode *node = lib.catalogue.search(bid);
    if (node)
    {
        // E2: Waitlist Queue logic
        if (node->data.availableCopies <= 0)
        {
            int qSize = waitMgr.queueSize(to_string(bid));
            cout << "  This book is currently borrowed."
                 << " (" << qSize << " in queue)\n"
                 << "  Add yourself to the waitlist? (y/n): ";
            char ch;
            cin >> ch;
            cin.ignore();
            if (ch == 'y' || ch == 'Y')
                waitMgr.enqueue(to_string(uid), to_string(bid));
        }
        // E7: Borrow Limit check
        else if (!limitMgr.canBorrow(to_string(uid), node->data.genre))
        {
            // limitMgr.canBorrow already printed the reason/error msg
        }
        // Proceed with standard borrow
        else
        {
            lib.borrowBook(uid, bid);
        }
    }
    else
    {
        cout << "  Book not found.\n"
            ;
    }
    pauseScreen();
}

static void userViewBorrowedHistory(Library &lib)
{
    printTitle("My Borrow History");
    ReadingStreakTracker::display(to_string(lib.currentUser->id));
    lib.history.sortByDate();
    lib.history.printForUser(lib.currentUser->id);
    pauseScreen();
}

static void userViewProfile(Library &lib)
{
    printTitle("My Profile");
    User *u = lib.currentUser;
    cout << "\n"
         << "  Name      : " << u->name << "\n"
         << "  Email     : " << u->email << "\n"
         << "  User ID   : " << u->id << "\n"
         << "  Role      : " << (u->isAdmin ? "Admin" : "Member") << "\n"
        
         << "  Borrows   : " << u->borrowCount << " / " << MAX_BORROW << "\n";

    double fine = lib.currentFine(u->id);
    cout << "  Fine Due  : " << (fine > 0 ? "Pending" : "None")
         << " - BDT " << fixed << setprecision(2) << fine << "\n";
        ;
    pauseScreen();
}

static void userReturnBook(Library &lib)
{
    printTitle("Return a Book");
    cout << "  Enter Book ID to return (0 to cancel): ";
    int bid = getIntInput();
    if (bid == 0) return;

    BSTNode *node = lib.catalogue.search(bid);
    lib.returnBook(lib.currentUser->id, bid);

    // E2: Notify next user in waitlist after successful return
    if (node)
    {
        waitMgr.notifyNext(to_string(bid), node->data.title);
    }
    pauseScreen();
}

static void userPayFine(Library &lib)
{
    printTitle("Pay Fine");
    double fine = lib.currentFine(lib.currentUser->id);
    if (fine <= 0)
    {
        cout << "  You have no outstanding fines!\n"
            ;
    }
    else
    {
        cout << "  Outstanding Fine: BDT " << fixed << setprecision(2) << fine << "\n"
            ;
        cout << "  Pay now? (y/n): ";
        char c;
        cin >> c;
        cin.ignore();
        if (c == 'y' || c == 'Y')
        {
            double previousFine = fine;
            double charged = lib.payFine(lib.currentUser->id);
            double remainingFine = lib.currentFine(lib.currentUser->id);
            if (charged > 0)
            {
                cout << "  Fine paid. Amount deducted: BDT " << charged << "!\n"
                    ;
            }
            else if (remainingFine < previousFine)
            {
                cout << "  Fine cleared!\n"
                    ;
            }
            else
            {
                cout << "  No payment was made.\n"
                    ;
            }
        }
    }
    pauseScreen();
}

void runUserPanel(Library &lib)
{
    while (true)
    {
        printTitle("User Panel  [" + lib.currentUser->name + "]");
        cout << "  1. View Catalogue\n"
             << "  2. Search Book\n"
             << "  3. Borrow Book\n"
             << "  4. View My Borrow History\n"
             << "  5. My Profile\n"
             << "  6. Return Book\n"
             << "  7. Pay Fine\n"
             << "  0. Sign Out\n";
        printLine();
        cout << "  Choice: ";
        int ch = getMenuChoice(0, 7);
        switch (ch)
        {
        case 1:
            userViewCatalogue(lib);
            break;
        case 2:
            userSearchBook(lib);
            break;
        case 3:
            userBorrowBook(lib);
            break;
        case 4:
            userViewBorrowedHistory(lib);
            break;
        case 5:
            userViewProfile(lib);
            break;
        case 6:
            userReturnBook(lib);
            break;
        case 7:
            userPayFine(lib);
            break;
        case 0:
            lib.currentUser = nullptr;
            return;
        }
    }
}