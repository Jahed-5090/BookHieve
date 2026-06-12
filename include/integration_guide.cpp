// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  BookHieve – Enhancement Integration Guide                              ║
// ║  This file documents EXACTLY where and how to integrate each header.   ║
// ║  Copy the snippets below into admin.cpp / user_panel.cpp / auth.cpp.   ║
// ╚══════════════════════════════════════════════════════════════════════════╝

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// STEP 0 ─ Add these #includes to library.h (or each .cpp that uses them)
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
#include "recommendation.h"   // Enhancement 1
#include "waitlist.h"          // Enhancement 2
#include "fine_system.h"       // Enhancement 3
#include "reading_streak.h"    // Enhancement 4
#include "overdue_warning.h"   // Enhancement 5
#include "book_condition.h"    // Enhancement 6
#include "borrow_limits.h"     // Enhancement 7
*/

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// STEP 1 ─ Declare one global (or pass by ref) instance of each manager
//          Add these near the top of library.h or as Library member fields:
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
RecommendationEngine recEngine;   // E1 – auto-loads catalogue + histories
WaitlistManager      waitMgr;     // E2 – FIFO queue per book
FineSystem           fineSys;     // E3 – tiered fines + grace period
ConditionManager     condMgr;     // E6 – New/Good/Fair/Worn tags
BorrowLimitManager   limitMgr;    // E7 – per-genre borrow caps
*/

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ENHANCEMENT 1 – Smart Book Recommendation Engine
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// File: user_panel.cpp  ─  inside viewCatalog() and searchBook()
//
//   After printing the catalog / search results, add:
//
//     recEngine.displayRecommendations(currentUserId);
//
// That's the full integration. The engine reads from data/history/<uid>.txt
// and data/books.txt, builds frequency + co-borrow maps, and prints the top 5.

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ENHANCEMENT 2 – Borrow Waitlist Queue
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// File: user_panel.cpp  ─  inside borrowBook()
//
//   BEFORE your existing "book not available" error block, replace it with:
//
//     if (!book.isAvailable) {
//         int qSize = waitMgr.queueSize(book.id);
//         cout << "  This book is currently borrowed."
//                   << " (" << qSize << " in queue)\n"
//                   << "  Add yourself to the waitlist? (y/n): ";
//         char ch; cin >> ch;
//         if (ch == 'y' || ch == 'Y')
//             waitMgr.enqueue(currentUserId, book.id);
//         return;
//     }
//
// File: user_panel.cpp / admin.cpp  ─  inside returnBook()
//
//   AFTER successfully marking a book as returned, add:
//
//     waitMgr.notifyNext(book.id, book.title);
//
// File: auth.cpp / user_panel.cpp  ─  inside the post-login flow
//
//     waitMgr.showNotifications(currentUserId);   // shows + clears notifications

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ENHANCEMENT 3 – Progressive Fine System with Grace Period
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// File: user_panel.cpp  ─  inside viewFineHistory()
//
//   For each overdue entry, replace the flat fine display with:
//
//     int daysOD = FineSystem::daysBetween(dueDate, todayStr());
//     fineSys.showFineBreakdown(currentUserId, daysOD);
//
// File: user_panel.cpp  ─  inside payFine()
//
//   Replace the deduction line with:
//
//     cout << "  Apply grace period (1 free/year)? (y/n): ";
//     char g; cin >> g;
//     bool useGrace = (g == 'y' || g == 'Y');
//     double charged = fineSys.applyPayment(currentUserId, daysOD, useGrace);
//     if (charged > 0) {
//         // your existing deduct-balance logic, using `charged` as amount
//     }
//
// File: admin.cpp  ─  inside fineHistory() admin view
//
//     fineSys.showAdminFineSummary();

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ENHANCEMENT 4 – Reading Streak & Borrowing Milestones
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// File: user_panel.cpp  ─  inside viewBorrowHistory()
//
//   At the top of the function, add:
//
//     ReadingStreakTracker::display(currentUserId);
//
//   The tracker reads timestamps from data/history/<userId>.txt,
//   computes total borrows, monthly streak, and earned milestone badges.

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ENHANCEMENT 5 – Overdue Early Warning System
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// File: auth.cpp / user_panel.cpp  ─  right after successful user login
//
//     OverdueWarningSystem::checkOnLogin(currentUserId);
//
// File: admin.cpp  ─  inside fineHistory() or a new "Overdue Report" menu option
//
//     OverdueWarningSystem::showAdminWarnings(2);  // warn 2 days ahead
//
// Requires: data/active/<userId>.txt per user (bookId|bookTitle|dueDate|genre)
// This file should be maintained by your existing borrowBook/returnBook logic.

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ENHANCEMENT 6 – Book Condition Tagging
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// File: admin.cpp  ─  inside addBook()
//
//   After the usual fields are entered:
//
//     BookCondition cond = condMgr.promptCondition();
//     condMgr.setCondition(newBook.id, cond);
//
// File: user_panel.cpp / admin.cpp  ─  inside the catalog print loop
//
//   Replace your existing catalog row print with:
//
//     condMgr.printCatalogRow(book.id, book.title, book.author, book.genre);
//
//   Or inline the badge: cout << condMgr.getBadge(book.id) << " " << book.title;
//
// File: admin.cpp  ─  inside catalog management menu
//
//     condMgr.showWornBooks();   // flag worn books for removal

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ENHANCEMENT 7 – Borrow Limit by Book Category
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// File: user_panel.cpp  ─  inside borrowBook(), before issuing the borrow
//
//     if (!limitMgr.canBorrow(currentUserId, book.genre)) {
//         return;  // canBorrow() already printed the reason
//     }
//     // ... proceed with existing borrow logic ...
//
// File: user_panel.cpp  ─  inside the Borrow Book screen header
//
//     limitMgr.showUsage(currentUserId);   // shows current/max per category
//
// File: admin.cpp  ─  inside settings / configuration menu
//
//     limitMgr.adminUpdateLimit();   // interactive genre limit editor
//
// Note: when writing a new borrow to data/active/<userId>.txt, make sure
// you write the genre as the 4th pipe-separated field:
//   bookId|bookTitle|dueDate|genre
// This is needed by both Enhancement 5 and Enhancement 7.

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// DATA FILE SUMMARY (new files introduced by these enhancements)
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//  data/waitlists/<bookId>.txt    – one userId per line (FIFO queue)
//  data/notifications.txt         – userId|bookId|message
//  data/grace/<userId>.txt        – year grace was used (e.g. "2025")
//  data/conditions.txt            – bookId|condition (New/Good/Fair/Worn)
//  data/borrow_limits.txt         – genre|limit (including "GLOBAL|5")
//  data/active/<userId>.txt       – bookId|bookTitle|dueDate|genre  (UPDATED)
//  data/history/<userId>.txt      – bookId|borrowDate|returnDate  (EXISTING)
//  data/userlist.txt              – one userId per line  (EXISTING)
//  data/books.txt                 – id|title|author|genre|...  (EXISTING)
