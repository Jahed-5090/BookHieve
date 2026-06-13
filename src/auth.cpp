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
    cout << "  Email (0 to cancel): ";
    getline(cin, email);
    if (email == "0") return false;
    cout << "  Password : ";
    pass = readPassword();

    User *u = lib.members.findByEmail(email);
    if (!u || u->password != pass)
    {
        cout << "\n  Invalid email or password.\n";
        pauseScreen();
        return false;
    }
    lib.currentUser = u;
    cout << "\n  Welcome, " << u->name << "!\n";

    double fine = lib.currentFine(u->id);
    if (fine > 0)
    {
        cout << "  Outstanding fine: BDT " << fixed << setprecision(2)
             << fine << "\n";
        // E2 & E5: Show Notifications & Overdue Warnings upon Login
        waitMgr.showNotifications(to_string(u->id));
        OverdueWarningSystem::checkOnLogin(to_string(u->id));
    }
    else
    {
        // Show only notifications when there is no outstanding fine.
        waitMgr.showNotifications(to_string(u->id));
    }

    pauseScreen();
    return true;
}

// ── Password prototype suggestion ─────────────────────────────────────────────
static string passwordPrototype()
{
    return "Ab#4xY7!";
}

// ── Sign Up ───────────────────────────────────────────────────────────────────
static bool signUp(Library &lib)
{
    printTitle("Sign Up");
    string name, email, pass, confirm;
    cout << "  Full Name (0 to cancel): ";
    getline(cin, name);
    if (name == "0") return false;
    cout << "  Email     : ";
    getline(cin, email);

    if (lib.members.findByEmail(email))
    {
        cout << "\n  Email already registered.\n";
        pauseScreen();
        return false;
    }

    cout << "  Strong password suggestion: letters + special symbols + numbers\n"
         << "  Example : " << passwordPrototype() << "\n";
    cout << "  Password  : ";
    pass = readPassword();
    cout << "  Confirm   : ";
    confirm = readPassword();
    if (pass != confirm)
    {
        cout << "\n  Passwords do not match.\n";
        pauseScreen();
        return false;
    }

    int id = lib.members.nextId();
    lib.members.insertEnd(User(id, name, email, pass, false));
    lib.save();
    cout << "\n  Account created! You can now sign in.\n";
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
             << "  Welcome to BookHieve!\n"
             << "  Your Digital Library Management System\n\n"
             << "  1. Sign In\n"
             << "  2. Sign Up\n"
             << "  0. Exit\n\n";
        printLine();
        cout << "  Choice: ";
        int ch = getMenuChoice(0, 2);

        if (ch == 0)
        {
            cout << "\n  Goodbye! Data saved.\n";
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

    }
}