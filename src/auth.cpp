#include "../include/library.h"

// Forward declarations
void runAdminPanel(Library &lib);
void runUserPanel(Library &lib);

// ── Password masking ─────────────────────────────────────────────────────────
#ifdef _WIN32
#include <conio.h>
static string readPassword()
{
    string pass;
    char ch;
    while ((ch = _getch()) != '\r')
    {
        if (ch == '\b')
        {
            if (!pass.empty())
            {
                pass.pop_back();
                cout << "\b \b";
            }
        }
        else
        {
            pass += ch;
            cout << '*';
        }
    }
    cout << "\n";
    return pass;
}
#else
#include <termios.h>
#include <unistd.h>
static string readPassword()
{
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    string pass;
    getline(cin, pass);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    cout << "\n";
    return pass;
}
#endif

// ── Sign In ───────────────────────────────────────────────────────────────────
static bool signIn(Library &lib)
{
    printTitle("Sign In");
    string email, pass;
    cout << "  Email    : ";
    getline(cin, email);
    cout << "  Password : ";
    pass = readPassword();

    User *u = lib.members.findByEmail(email);
    if (!u || u->password != pass)
    {
        cout << RED << "\n  Invalid email or password.\n"
             << RESET;
        pauseScreen();
        return false;
    }
    lib.currentUser = u;
    cout << GREEN << "\n  Welcome, " << u->name << "!\n"
         << RESET;

    double fine = lib.currentFine(u->id);
    if (fine > 0)
    {
        cout << YELLOW << "  Outstanding fine: BDT " << fixed << setprecision(2)
             << fine << "\n" << RESET;
        // E2 & E5: Show Notifications & Overdue Warnings upon Login
        waitMgr.showNotifications(std::to_string(u->id));
        OverdueWarningSystem::checkOnLogin(std::to_string(u->id));
    }
    else
    {
        // Show only notifications when there is no outstanding fine.
        waitMgr.showNotifications(std::to_string(u->id));
    }

    pauseScreen();
    return true;
}

// ── Sign Up ───────────────────────────────────────────────────────────────────
static bool signUp(Library &lib)
{
    printTitle("Sign Up");
    string name, email, pass, confirm;
    cout << "  Full Name : ";
    getline(cin, name);
    cout << "  Email     : ";
    getline(cin, email);

    if (lib.members.findByEmail(email))
    {
        cout << RED << "\n  Email already registered.\n"
             << RESET;
        pauseScreen();
        return false;
    }

    cout << "  Password  : ";
    pass = readPassword();
    cout << "  Confirm   : ";
    confirm = readPassword();
    if (pass != confirm)
    {
        cout << RED << "\n  Passwords do not match.\n"
             << RESET;
        pauseScreen();
        return false;
    }

    int id = lib.members.nextId();
    lib.members.insertEnd(User(id, name, email, pass, false));
    lib.save();
    cout << GREEN << "\n  Account created! You can now sign in.\n"
         << RESET;
    pauseScreen();
    return true;
}

// ── Main Menu ─────────────────────────────────────────────────────────────────
void runHomepage(Library &lib)
{
    while (true)
    {
        printTitle("Homepage");
        cout << "\n"
             << "  " << BOLD << YELLOW << "Welcome to BookHieve!" << RESET << "\n"
             << "  Your Digital Library Management System\n\n"
             << "  1. Sign In\n"
             << "  2. Sign Up\n"
             << "  0. Exit\n\n";
        printLine();
        cout << "  Choice: ";
        int ch;
        cin >> ch;
        cin.ignore();

        if (ch == 0)
        {
            cout << CYAN << "\n  Goodbye! Data saved.\n"
                 << RESET;
            break;
        }
        if (ch == 1)
        {
            if (signIn(lib))
            {
                if (lib.currentUser->isAdmin)
                    runAdminPanel(lib);
                else
                    runUserPanel(lib);
            }
        }
        else if (ch == 2)
        {
            signUp(lib);
        }
        else
        {
            cout << RED << "  Invalid choice.\n"
                 << RESET;
        }
    }
}