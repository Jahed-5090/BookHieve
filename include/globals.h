#pragma once
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
// Older MinGW SDKs may lack this constant (added for Windows 10 VT100 support)
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

using namespace std;

// Enable ANSI escape codes on Windows 10+ terminals
inline void enableAnsiColors() {
#ifdef _WIN32
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut != INVALID_HANDLE_VALUE) {
    DWORD mode = 0;
    if (GetConsoleMode(hOut, &mode)) {
      mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      SetConsoleMode(hOut, mode);
    }
  }
  // Set console output code page to UTF-8 for emoji/unicode support
  SetConsoleOutputCP(CP_UTF8);
#endif
}

// ─── Constants
// ────────────────────────────────────────────────────────────────
const string DATA_DIR = "data/";
const string BOOKS_FILE = DATA_DIR + "books.txt";
const string USERS_FILE = DATA_DIR + "users.txt";
const string BORROWS_FILE = DATA_DIR + "borrows.txt";
const string FINES_FILE = DATA_DIR + "fines.txt";

const int MAX_BORROW = 3;        // max books a user can borrow
const double FINE_PER_DAY = 5.0; // taka per day overdue
const int BORROW_DAYS = 14;      // allowed borrow period

// ─── Colour helpers (ANSI) ───────────────────────────────────────────────────
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define MAGENTA "\033[35m"
#define WHITE "\033[37m"

// ─── Utility helpers ─────────────────────────────────────────────────────────
inline void clearScreen() {
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}

// ─── Portable directory creation ─────────────────────────────────────────────
// Handles nested paths and converts forward slashes to backslashes on Windows
inline void portableMkdir(const std::string &path) {
#ifdef _WIN32
  // Convert forward slashes to backslashes for Windows mkdir
  std::string winPath = path;
  for (char &c : winPath)
    if (c == '/')
      c = '\\';
  // Remove trailing backslash if present (mkdir doesn't like it)
  while (!winPath.empty() && winPath.back() == '\\')
    winPath.pop_back();
  std::string cmd =
      "if not exist \"" + winPath + "\" mkdir \"" + winPath + "\"";
  system(cmd.c_str());
#else
  std::string cmd = "mkdir -p \"" + path + "\"";
  system(cmd.c_str());
#endif
}

inline void pauseScreen() {
  cout << "\n" << YELLOW << "Press Enter to continue..." << RESET;
  cin.ignore(numeric_limits<streamsize>::max(), '\n');
  cin.get();
}

inline string currentDate() {
  time_t t = time(nullptr);
  tm *lt = localtime(&t);
  char buf[11];
  strftime(buf, sizeof(buf), "%Y-%m-%d", lt);
  return string(buf);
}

// Days between two "YYYY-MM-DD" strings (d2 - d1)
inline int daysBetween(const string &d1, const string &d2) {
  tm t1{}, t2{};
  sscanf(d1.c_str(), "%d-%d-%d", &t1.tm_year, &t1.tm_mon, &t1.tm_mday);
  sscanf(d2.c_str(), "%d-%d-%d", &t2.tm_year, &t2.tm_mon, &t2.tm_mday);
  t1.tm_year -= 1900;
  t1.tm_mon -= 1;
  t2.tm_year -= 1900;
  t2.tm_mon -= 1;
  time_t s1 = mktime(&t1), s2 = mktime(&t2);
  return (int)difftime(s2, s1) / 86400;
}

inline void printLine(char c = '-', int n = 60) {
  cout << string(n, c) << "\n";
}

inline void printTitle(const string &title) {
  clearScreen();
  printLine('=');
  cout << BOLD << CYAN << "  BookHieve – Digital Library Management System\n"
       << RESET;
  printLine('=');
  cout << BOLD << YELLOW << "  " << title << "\n" << RESET;
  printLine('-');
}

// Helper for robust integer input to prevent cin crashes
inline int getIntInput() {
    int val;
    while (!(cin >> val)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << RED << "  Invalid input. Please enter a valid number: " << RESET;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return val;
}

// Helper for menus to enforce valid range without returning to main loop
inline int getMenuChoice(int minVal, int maxVal) {
    while (true) {
        int val = getIntInput();
        if (val >= minVal && val <= maxVal) {
            return val;
        }
        cout << RED << "  Invalid choice. Please enter a number between " 
             << minVal << " and " << maxVal << ": " << RESET;
    }
}
