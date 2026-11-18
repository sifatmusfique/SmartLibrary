#include "JsonApiAdapter.hpp"
#include "LibraryManager.hpp"
#include <iostream>
#include <string>

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        ++g_tests_run; \
        if (cond) { \
            ++g_tests_passed; \
            std::cout << "  [PASS] " << msg << "\n"; \
        } else { \
            std::cerr << "  [FAIL] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
        } \
    } while (0)

void test_T38_gui_api_parity() {
    std::cout << "\n=== GUI Parity Tests (T38: JsonApiAdapter vs LibraryManager) ===\n";
    LibraryManager mgr(5, 3, 3);
    JsonApiAdapter api(mgr);

    // 1. Add book 101 via API
    ApiResponse r1 = api.handleRequest("POST", "/api/books/add",
        R"({"id":101,"title":"Data Structures","author":"Seymour Lipschutz"})");
    TEST_ASSERT(r1.httpStatus == 200 && r1.jsonBody.find("\"ok\":true") != std::string::npos,
                "T38: Add book 101 via API succeeds");

    // Verify underlying C++ engine matches
    Book b;
    TEST_ASSERT(mgr.searchBook(101, b) == Status::Ok && b.title == "Data Structures",
                "T38: Authoritative C++ LibraryManager holds book 101");

    // 2. Search via GET query
    ApiResponse rSearch = api.handleRequest("GET", "/api/books/search?id=101", "");
    TEST_ASSERT(rSearch.httpStatus == 200 && rSearch.jsonBody.find("Seymour Lipschutz") != std::string::npos,
                "T38: Search 101 returns JSON book payload");

    ApiResponse rMissing = api.handleRequest("GET", "/api/books/search?id=999", "");
    TEST_ASSERT(rMissing.httpStatus == 404 && rMissing.jsonBody.find("Book not found.") != std::string::npos,
                "T38: Search missing 999 returns 404 Book not found.");

    // 3. Borrow twice -> borrow then queue
    ApiResponse rBorrow1 = api.handleRequest("POST", "/api/circulation/borrow", R"({"id":101})");
    TEST_ASSERT(rBorrow1.httpStatus == 200 && rBorrow1.jsonBody.find("Borrowed") != std::string::npos,
                "T38: First borrow succeeds");

    ApiResponse rBorrow2 = api.handleRequest("POST", "/api/circulation/borrow", R"({"id":101})");
    TEST_ASSERT(rBorrow2.httpStatus == 200 && rBorrow2.jsonBody.find("Queued") != std::string::npos,
                "T38: Repeat borrow enqueues 101");

    // 4. Delete blocked while borrowed/pending
    ApiResponse rDelBlocked = api.handleRequest("POST", "/api/books/delete", R"({"id":101})");
    TEST_ASSERT(rDelBlocked.httpStatus == 409 && rDelBlocked.jsonBody.find("Blocked") != std::string::npos,
                "T38: Delete blocked while borrowed/queued");

    // 5. Process blocked head while borrowed
    ApiResponse rProcBlocked = api.handleRequest("POST", "/api/circulation/process", "{}");
    TEST_ASSERT(rProcBlocked.httpStatus == 409 && rProcBlocked.jsonBody.find("Blocked") != std::string::npos,
                "T38: Process unavailable head blocked");

    // 6. Return 101 -> stack top 101, then process head -> re-borrows 101
    ApiResponse rRet = api.handleRequest("POST", "/api/circulation/return", R"({"id":101})");
    TEST_ASSERT(rRet.httpStatus == 200 && mgr.getReturnStack().getCount() == 1,
                "T38: Return updates stack and availability");

    ApiResponse rProcOk = api.handleRequest("POST", "/api/circulation/process", "{}");
    TEST_ASSERT(rProcOk.httpStatus == 200 && mgr.getRequestQueue().isEmpty(),
                "T38: Process head dequeues and borrows 101");

    // 7. Pop stack then pop empty -> Stack Underflow!
    ApiResponse rPop1 = api.handleRequest("POST", "/api/circulation/pop-stack", "{}");
    TEST_ASSERT(rPop1.httpStatus == 200, "T38: Pop 101 succeeds");
    ApiResponse rPopEmpty = api.handleRequest("POST", "/api/circulation/pop-stack", "{}");
    TEST_ASSERT(rPopEmpty.httpStatus == 409 && rPopEmpty.jsonBody.find("Stack Underflow!") != std::string::npos,
                "T38: Empty pop returns Stack Underflow!");

    // 8. State JSON includes all 4 structures
    ApiResponse rState = api.handleRequest("GET", "/api/state", "");
    TEST_ASSERT(rState.httpStatus == 200 &&
                rState.jsonBody.find("\"catalogue\"") != std::string::npos &&
                rState.jsonBody.find("\"history\"") != std::string::npos &&
                rState.jsonBody.find("\"returnStack\"") != std::string::npos &&
                rState.jsonBody.find("\"requestQueue\"") != std::string::npos,
                "T38: GET /api/state serializes all 4 structures");

    // 9. JSON key-inside-string safety, trimming, and overflow handling
    ApiResponse rNestedStr = api.handleRequest("POST", "/api/books/add",
        R"({"title":"Learn \"id\": 999","author":"  Clean Author  ","id":202})");
    TEST_ASSERT(rNestedStr.httpStatus == 200, "T38: JSON parser skips quoted keys inside string values");
    Book b202;
    TEST_ASSERT(mgr.searchBook(202, b202) == Status::Ok && b202.author == "Clean Author",
                "T38: Book 202 stored with trimmed author");

    ApiResponse rOverflow = api.handleRequest("GET", "/api/books/search?id=99999999999999999999999", "");
    TEST_ASSERT(rOverflow.httpStatus == 400, "T38: Query integer overflow returns 400 Invalid");

    // 10. Maintenance search endpoints (history, stack, queue)
    ApiResponse rHistSearch = api.handleRequest("GET", "/api/maintenance/history/search?txId=1", "");
    TEST_ASSERT(rHistSearch.httpStatus == 200 && rHistSearch.jsonBody.find("\"bookId\":101") != std::string::npos,
                "T38: Maintenance history search by TxID works");

    mgr.returnBook(101); // puts 101 on stack
    BorrowAction tmpAct = BorrowAction::None;
    mgr.borrowBook(101, tmpAct); // borrows 101
    mgr.borrowBook(101, tmpAct); // queues 101

    ApiResponse rStackSearch = api.handleRequest("GET", "/api/maintenance/stack/search?id=101", "");
    TEST_ASSERT(rStackSearch.httpStatus == 200 && rStackSearch.jsonBody.find("\"offset\":0") != std::string::npos,
                "T38: Maintenance stack search returns top offset 0");

    ApiResponse rQueueSearch = api.handleRequest("GET", "/api/maintenance/queue/search?id=101", "");
    TEST_ASSERT(rQueueSearch.httpStatus == 200 && rQueueSearch.jsonBody.find("\"offset\":0") != std::string::npos,
                "T38: Maintenance queue search returns front offset 0");

    TEST_ASSERT(mgr.verifyAllInvariants(), "T38: All invariants hold after GUI adapter operations");
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "  SmartLibrary SL-007 GUI API Parity Test Suite     \n";
    std::cout << "====================================================\n";

    test_T38_gui_api_parity();

    std::cout << "\n====================================================\n";
    std::cout << "Test Summary: " << g_tests_passed << " / " << g_tests_run << " passed.\n";
    std::cout << "====================================================\n";

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}
