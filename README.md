# BookHieve

## Project Overview

BookHieve is a C++ console-based digital library management system built as a DSA project. It supports user authentication, admin controls, book borrowing and returning, waitlists, fines, book condition tracking, genre recommendations, and borrow history.

## Key Features

- User login and registration with admin/user roles
- Book catalogue stored in a binary search tree (BST) for fast search, insert, and delete
- Borrow history sorted via merge sort
- Undo last borrow using a stack structure
- Waitlist functionality using a FIFO queue
- Fine calculation and grace period management
- Book condition tagging (New / Good / Fair / Worn)
- Borrow limits per genre
- Recommendations via genre-based graph traversal

## Project Structure

- `main.cpp` - application entry point and program launch
- `src/auth.cpp` - authentication and homepage menu logic
- `src/admin.cpp` - admin panel actions and management features
- `src/user_panel.cpp` - user dashboard, borrow/return actions, recommendations
- `include/` - project headers for books, users, borrow system, file management, recommendations, and more
- `data/` - persistent text files for books, users, borrows, conditions, limits, and history

## Build and Run

This project includes a `Makefile` for easy compilation.

From the project root:

```bash
make
./BookHieve
```

To clean compiled output:

```bash
make clean
```

## Notes

- The project uses C++17 and compiles with `g++`
- Default data files are stored in the `data/` folder
- Admin user is seeded automatically if missing

## How to Use

1. Build the project with `make`
2. Run the executable with `./BookHieve`
3. Log in as admin or register as a new user
4. Use the menus to browse, borrow, return, view history, and manage books

## Contributions

Feel free to extend the project by adding features such as:

- GUI interface
- database backend
- enhanced recommendation engine
- networked multi-user support
