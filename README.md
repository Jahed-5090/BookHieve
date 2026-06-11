# BookHieve

## Project Overview

BookHieve is a C++ console-based digital library management system designed as a data structures and algorithms project. It provides a complete library workflow with role-based access, book browsing, borrowing, returns, waitlists, fine management, and personalized recommendations.

## Key Features

- User login and registration with admin and reader roles
- Book catalogue stored in a binary search tree (BST) for efficient lookup, insertion, and removal
- Borrow history tracked with sorting utilities and per-user history files
- Undo last borrow using a stack-style rollback mechanism
- Waitlist support using FIFO queues for unavailable books
- Fine calculation, grace period checks, and overdue tracking
- Book condition tracking with tags such as New, Good, Fair, and Worn
- Genre-based borrow limits and admin-configurable limits
- Recommendation engine using genre graph traversal (BFS/DFS/topological ordering)
- Persistent storage via text files under `data/`

## Project Structure

- `main.cpp` - application entry point and startup logic
- `src/auth.cpp` - login, registration, and homepage navigation
- `src/admin.cpp` - admin dashboard, book management, user management, reports, and settings
- `src/user_panel.cpp` - user dashboard, borrow/return flows, history, and recommendations
- `include/` - headers for core systems including the BST catalogue, user list, borrowing, file management, fines, recommendations, and more
- `data/` - persistent storage files used at runtime for books, users, borrows, conditions, limits, active borrows, waitlists, and history

## Data Files

The application uses plain text data files in the `data/` directory:

- `data/books.txt` - stored catalogue of books
- `data/users.txt` - registered user records
- `data/borrows.txt` - borrow history records
- `data/conditions.txt` - book condition tags
- `data/borrow_limits.txt` - borrow limits by genre
- `data/userlist.txt` - list of user IDs
- `data/active/` - active borrow records for currently borrowed books
- `data/history/` - per-user borrow history files
- `data/waitlists/` - waitlist entries for books with pending requests

## Build and Run

From the project root directory, compile and run using the included `Makefile`:

```bash
make
./BookHieve
```

To remove generated binaries:

```bash
make clean
```

## Default Credentials

On first run, the application seeds a default administrator account if one is not found.

- Email: `admin@bookhieve.com`
- Password: `admin123`

## Usage

1. Build the project with `make`
2. Run the executable with `./BookHieve`
3. Log in as the seeded admin or register a new user
4. Admin users can add, update, delete books, adjust borrow limits, and manage users
5. Regular users can browse books, borrow/return titles, view active borrows, and get recommendations

## Notes

- The project is written for C++17 and compiles with `g++`
- Data persists between runs in the `data/` directory
- Admin login is seeded automatically when no admin account exists

## Suggested Enhancements

Potential future improvements include:

- GUI interface or web front end
- database-backed persistence
- networked multi-user operation
- stronger authentication and password hashing
- expanded recommendation algorithms
