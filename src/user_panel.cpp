#include "../include/library.h"

// ══════════════════════════════════════════════════════════════════════════════
//  User Panel
// ══════════════════════════════════════════════════════════════════════════════

static void userViewCatalogue(Library &lib)
{
    printTitle("Browse Catalogue");
    lib.catalogue.printAll();
    recEngine.displayRecommendations(std::to_string(lib.currentUser->id));
    pauseScreen();
}

static void userSearchBook(Library &lib)
{
    printTitle("Search Book");
    cout << "  1. By Title\n  2. By Author\n  3. By ID\n  Choice: ";
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
    recEngine.displayRecommendations(std::to_string(lib.currentUser->id));
    pauseScreen();
}

static void userBorrowBook(Library &lib)
{
    printTitle("Borrow a Book");
    int uid = lib.currentUser->id;
    int active = lib.history.activeCount(uid);

    limitMgr.showUsage(std::to_string(uid));
    cout << "  Active borrows: " << active << " / " << MAX_BORROW << "\n\n";

    cout << "  Enter Book ID to borrow: ";
    int bid;
    cin >> bid;
    cin.ignore();

    BSTNode *node = lib.catalogue.search(bid);
    if (node)
    {
        // E2: Waitlist Queue logic
        if (node->data.availableCopies <= 0)
        {
            int qSize = waitMgr.queueSize(std::to_string(bid));
            cout << "  This book is currently borrowed."
                 << " (" << qSize << " in queue)\n"
                 << "  Add yourself to the waitlist? (y/n): ";
            char ch;
            cin >> ch;
            cin.ignore();
            if (ch == 'y' || ch == 'Y')
                waitMgr.enqueue(std::to_string(uid), std::to_string(bid));
        }
        // E7: Borrow Limit check
        else if (!limitMgr.canBorrow(std::to_string(uid), node->data.genre))
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
        cout << RED << "  Book not found.\n"
             << RESET;
    }
    pauseScreen();
}

static void userViewBorrowedHistory(Library &lib)
{
    printTitle("My Borrow History");
    ReadingStreakTracker::display(std::to_string(lib.currentUser->id));
    lib.history.sortByDate();
    lib.history.printForUser(lib.currentUser->id);
    pauseScreen();
}

static void userViewProfile(Library &lib)
{
    printTitle("My Profile");
    User *u = lib.currentUser;
    cout << "\n"
         << CYAN << "  Name      : " << WHITE << u->name << "\n"
         << CYAN << "  Email     : " << WHITE << u->email << "\n"
         << CYAN << "  User ID   : " << WHITE << u->id << "\n"
         << CYAN << "  Role      : " << (u->isAdmin ? GREEN "Admin" : YELLOW "Member") << "\n"
         << RESET
         << CYAN << "  Borrows   : " << WHITE << u->borrowCount << " / " << MAX_BORROW << "\n";

    double fine = lib.currentFine(u->id);
    cout << CYAN << "  Fine Due  : " << (fine > 0 ? RED : GREEN)
         << "BDT " << fixed << setprecision(2) << fine << "\n"
         << RESET;
    pauseScreen();
}

static void userReturnBook(Library &lib)
{
    printTitle("Return a Book");
    cout << "  Enter Book ID to return: ";
    int bid;
    cin >> bid;
    cin.ignore();

    BSTNode *node = lib.catalogue.search(bid);
    lib.returnBook(lib.currentUser->id, bid);

    // E2: Notify next user in waitlist after successful return
    if (node)
    {
        waitMgr.notifyNext(std::to_string(bid), node->data.title);
    }
    pauseScreen();
}

static void userPayFine(Library &lib)
{
    printTitle("Pay Fine");
    double fine = lib.currentFine(lib.currentUser->id);
    if (fine <= 0)
    {
        cout << GREEN << "  You have no outstanding fines!\n"
             << RESET;
    }
    else
    {
        cout << RED << "  Outstanding Fine: BDT " << fixed << setprecision(2) << fine << "\n"
             << RESET;
        cout << "  Pay now? (y/n): ";
        char c;
        cin >> c;
        cin.ignore();
        if (c == 'y' || c == 'Y')
        {
            // E3: Fine grace period logic
            cout << "  Apply grace period (1 free/year)? (y/n): ";
            char g;
            cin >> g;
            cin.ignore();
            bool useGrace = (g == 'y' || g == 'Y');

            double previousFine = fine;
            double charged = lib.payFine(lib.currentUser->id, useGrace);
            double remainingFine = lib.currentFine(lib.currentUser->id);
            if (charged > 0)
            {
                cout << GREEN << "  Fine paid. Amount deducted: BDT " << charged << "!\n"
                     << RESET;
            }
            else if (remainingFine < previousFine)
            {
                cout << GREEN << "  Fine reduced by grace or cleared!\n"
                     << RESET;
            }
            else
            {
                cout << YELLOW << "  No payment was made.\n"
                     << RESET;
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
        int ch;
        cin >> ch;
        cin.ignore();
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
        default:
            cout << RED << "  Invalid choice.\n"
                 << RESET;
        }
    }
}