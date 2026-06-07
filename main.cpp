// ╔══════════════════════════════════════════════════════════════════════════╗
// ║       BookHieve – Digital Library Management System                     ║
// ║       DSA Project in C++                                                ║
// ║                                                                          ║
// ║  DSA Structures Used:                                                    ║
// ║   • Array Operations      – BorrowHistory (vector + merge sort)          ║
// ║   • Stack                 – BorrowStack (undo last borrow)               ║
// ║   • Queue                 – BorrowQueue (waitlist FIFO)                  ║
// ║   • Infix→Postfix/Prefix  – Expression evaluator (fine formula)          ║
// ║   • Singly Linked List    – UserList membership                          ║
// ║   • BST                   – BookBST catalogue search/insert/delete       ║
// ║   • Sorting (5 kinds)     – Bubble/Insertion/Selection/Quick/Merge       ║
// ║   • Binary Indexed Tree   – Fine prefix-sum queries                      ║
// ║   • Heap sort/min/max     – FineMaxHeap, DueMinHeap                      ║
// ║   • BFS / DFS / Topo Sort – GenreGraph recommendations                   ║
// ╚══════════════════════════════════════════════════════════════════════════╝

#include "include/library.h" 

RecommendationEngine recEngine;
WaitlistManager      waitMgr;
FineSystem           fineSys;
ConditionManager     condMgr;
BorrowLimitManager   limitMgr;

void runHomepage(Library& lib);

int main() {
    enableAnsiColors();
    Library lib;
    runHomepage(lib);
    return 0;
}

