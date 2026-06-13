#include "include/library.h" 

RecommendationEngine recEngine;
WaitlistManager      waitMgr;
FineSystem           fineSys;
ConditionManager     condMgr;
BorrowLimitManager   limitMgr;

void runHomepage(Library &lib);

int main() {
    Library lib;
    runHomepage(lib);
    return 0;
}