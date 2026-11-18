#include "LibraryManager.hpp"
#include <cstdint>
#include <iostream>
#include <new>
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

void test_T02_T03_borrow_return_and_repeat_queue() {
    std::cout << "\n=== Manager Tests (T02, T03) ===\n";
    LibraryManager mgr(10, 10, 10);
    mgr.addBook(101, "Data Structures", "Seymour Lipschutz");

    BorrowAction action = BorrowAction::None;
    std::uint64_t tx1 = 0;
    TEST_ASSERT(mgr.borrowBook(101, action, &tx1) == Status::Ok && action == BorrowAction::Borrowed,
                "T02: First borrow of 101 succeeded");

    Book b;
    mgr.searchBook(101, b);
    TEST_ASSERT(!b.available, "T02: Book 101 status changed to Borrowed");

    // T03: Second borrow of unavailable 101 -> queued
    BorrowAction action2 = BorrowAction::None;
    TEST_ASSERT(mgr.borrowBook(101, action2) == Status::Ok && action2 == BorrowAction::Queued,
                "T03: Second borrow of 101 queued");
    TEST_ASSERT(mgr.getRequestQueue().getCount() == 1, "T03: Queue has 1 waiting request");
    TEST_ASSERT(mgr.getHistory().getCount() == 1, "T03: Queueing did not add history entry");

    // Cancel queued request so we can cleanly test T02 return
    int cancelledId = 0;
    mgr.cancelHeadRequest(cancelledId);
    TEST_ASSERT(cancelledId == 101 && mgr.getRequestQueue().isEmpty(), "Cancelled head request 101");

    // Return 101
    std::uint64_t tx2 = 0;
    TEST_ASSERT(mgr.returnBook(101, &tx2) == Status::Ok, "T02: Return of 101 succeeded");
    mgr.searchBook(101, b);
    TEST_ASSERT(b.available, "T02: Book 101 status is Available again");
    TEST_ASSERT(mgr.getHistory().getCount() == 2, "T02: Two chronological history nodes exist");

    int topStack = 0;
    TEST_ASSERT(mgr.getReturnStack().peek(topStack) == Status::Ok && topStack == 101, "T02: Stack top is 101");
}

void test_T10_T19_T20_delete_and_return_guards() {
    std::cout << "\n=== Manager Tests (T10, T19, T20) ===\n";
    LibraryManager mgr(10, 10, 10);
    mgr.addBook(101, "Algorithms", "CLRS");
    mgr.addBook(102, "Operating Systems", "Silberschatz");

    // T19: Return already available or missing book
    TEST_ASSERT(mgr.returnBook(101) == Status::Blocked, "T19: Returning available book rejected with Blocked");
    TEST_ASSERT(mgr.returnBook(999) == Status::NotFound, "T19: Returning missing book rejected with NotFound");
    TEST_ASSERT(mgr.getHistory().isEmpty() && mgr.getReturnStack().isEmpty(), "T19: History and stack unchanged");

    // Borrow 101 -> try delete borrowed book (T10)
    BorrowAction act;
    mgr.borrowBook(101, act);
    TEST_ASSERT(mgr.deleteBook(101) == Status::Blocked, "T10: Cannot delete currently borrowed book");

    // Request 101 again, then return 101 -> 101 is available, BUT still has pending request (T10)
    mgr.borrowBook(101, act);
    mgr.returnBook(101);
    TEST_ASSERT(mgr.deleteBook(101) == Status::Blocked, "T10: Cannot delete book with pending request in queue");

    // Cancel pending request -> now delete 101 (T20)
    int cancelled = 0;
    mgr.cancelHeadRequest(cancelled);
    TEST_ASSERT(mgr.deleteBook(101) == Status::Ok, "T20: Delete available book with no pending requests succeeded");

    // T20: Historical snapshots remain readable after catalogue deletion
    HistoryRecord hr;
    TEST_ASSERT(mgr.findHistory(1, hr) == Status::Ok && hr.bookId == 101 && hr.title == "Algorithms",
                "T20: Historical record remains intact after book deletion");
}

void test_T14_stack_full_rejects_return() {
    std::cout << "\n=== Manager Tests (T14: Full return stack blocks return) ===\n";
    LibraryManager mgr(10, 2, 10); // Stack capacity 2
    mgr.addBook(1, "B1", "A1");
    mgr.addBook(2, "B2", "A2");
    mgr.addBook(3, "B3", "A3");

    BorrowAction act;
    mgr.borrowBook(1, act);
    mgr.borrowBook(2, act);
    mgr.borrowBook(3, act);

    TEST_ASSERT(mgr.returnBook(1) == Status::Ok, "T14: Return 1 fills slot 1");
    TEST_ASSERT(mgr.returnBook(2) == Status::Ok, "T14: Return 2 fills slot 2 (stack full)");
    TEST_ASSERT(mgr.getReturnStack().isFull(), "T14: Return stack is now full");

    size_t histBefore = mgr.getHistory().getCount();
    TEST_ASSERT(mgr.returnBook(3) == Status::Full, "T14: Return 3 rejected when stack is full");

    Book b3;
    mgr.searchBook(3, b3);
    TEST_ASSERT(!b3.available, "T14: Book 3 remains Borrowed after rejected return");
    TEST_ASSERT(mgr.getHistory().getCount() == histBefore, "T14: History count unchanged");

    // Pop one slot and retry return 3
    int popped = 0;
    mgr.popReturnStack(popped);
    TEST_ASSERT(popped == 2 && mgr.returnBook(3) == Status::Ok, "T14: After popping, return 3 succeeds");
}

void test_T15_T16_process_and_cancel_requests() {
    std::cout << "\n=== Manager Tests (T15, T16: Process & Cancel FIFO Requests) ===\n";
    LibraryManager mgr(10, 10, 5);
    mgr.addBook(101, "Book 101", "Author 1");
    mgr.addBook(102, "Book 102", "Author 2");

    BorrowAction act;
    mgr.borrowBook(101, act); // Borrowed
    mgr.borrowBook(101, act); // Queued head (101)
    mgr.borrowBook(102, act); // Borrowed
    mgr.borrowBook(102, act); // Queued second (102)

    // T16: Process when head 101 is still borrowed -> Blocked, no mutation
    int processedId = 0;
    size_t qBefore = mgr.getRequestQueue().getCount();
    size_t hBefore = mgr.getHistory().getCount();
    TEST_ASSERT(mgr.processNextRequest(processedId) == Status::Blocked,
                "T16: Processing unavailable head 101 returns Blocked");
    TEST_ASSERT(mgr.getRequestQueue().getCount() == qBefore && mgr.getHistory().getCount() == hBefore,
                "T16: Queue and history unchanged when blocked");

    // Explicit cancel removes head 101, exposing 102
    int cancelledId = 0;
    TEST_ASSERT(mgr.cancelHeadRequest(cancelledId) == Status::Ok && cancelledId == 101,
                "T16: Explicit cancel removed head 101");

    // Return 102 -> now head 102 is available -> T15 process works
    mgr.returnBook(102);
    std::uint64_t txProc = 0;
    TEST_ASSERT(mgr.processNextRequest(processedId, &txProc) == Status::Ok && processedId == 102,
                "T15: Processing available head 102 succeeded");
    Book b102;
    mgr.searchBook(102, b102);
    TEST_ASSERT(!b102.available && mgr.getRequestQueue().isEmpty(),
                "T15: Book 102 is now Borrowed and queue is empty");
}

void test_T29_maintenance_validation() {
    std::cout << "\n=== Manager Tests (T29: Maintenance validation) ===\n";
    LibraryManager mgr(10, 10, 10);
    mgr.addBook(101, "B101", "A1");
    mgr.addBook(102, "B102", "A2");

    BorrowAction act;
    mgr.borrowBook(101, act);
    mgr.borrowBook(101, act); // Queue has [101]
    mgr.returnBook(101);      // Stack has [101]

    // Valid maintenance update on stack & queue to existing book 102
    TEST_ASSERT(mgr.updateReturnStackSlot(0, 102) == Status::Ok, "T29: Valid stack maintenance update to 102");
    TEST_ASSERT(mgr.updateRequestQueueSlot(0, 102) == Status::Ok, "T29: Valid queue maintenance update to 102");

    // Missing book ID 999 or invalid ID -1 -> rejected without mutation
    TEST_ASSERT(mgr.updateReturnStackSlot(0, 999) == Status::NotFound, "T29: Stack maintenance missing ID 999 rejected");
    TEST_ASSERT(mgr.updateReturnStackSlot(0, -1) == Status::Invalid, "T29: Stack maintenance invalid ID -1 rejected");
    TEST_ASSERT(mgr.updateRequestQueueSlot(0, 999) == Status::NotFound, "T29: Queue maintenance missing ID 999 rejected");
    TEST_ASSERT(mgr.updateRequestQueueSlot(0, 0) == Status::Invalid, "T29: Queue maintenance invalid ID 0 rejected");

    // Verify slots remained 102
    TEST_ASSERT(*mgr.getReturnStack().getAtTopOffset(0) == 102, "T29: Stack slot preserved as 102");
    TEST_ASSERT(*mgr.getRequestQueue().getAtFrontOffset(0) == 102, "T29: Queue slot preserved as 102");

    // Verify history edit does NOT change catalogue availability
    Book b101;
    mgr.searchBook(101, b101); // currently available
    TEST_ASSERT(mgr.editHistory(1, "Corrected Name", "RETURN") == Status::Ok, "T29: History edit succeeded");
    mgr.searchBook(101, b101);
    TEST_ASSERT(b101.available == true, "T29: History edit never alters live book availability");
}

void test_T33_allocation_failure_atomicity() {
    std::cout << "\n=== Manager Tests (T33: Allocation Failure Atomicity) ===\n";
    LibraryManager mgr(10, 10, 10);
    mgr.addBook(101, "Atomic Book", "Author");

    // 1. Fail allocation during borrowBook
    mgr.injectNextHistoryAllocationFailure(true);
    bool caughtBorrow = false;
    BorrowAction act = BorrowAction::None;
    try {
        mgr.borrowBook(101, act);
    } catch (const std::bad_alloc&) {
        caughtBorrow = true;
    }
    Book b;
    mgr.searchBook(101, b);
    TEST_ASSERT(caughtBorrow, "T33: std::bad_alloc thrown on simulated history allocation failure during borrow");
    TEST_ASSERT(b.available == true, "T33: Book 101 remained Available (no partial mutation)");
    TEST_ASSERT(mgr.getHistory().isEmpty() && mgr.getHistory().getNextTransactionId() == 1,
                "T33: History list and monotonic txId untouched");

    // Perform normal borrow
    TEST_ASSERT(mgr.borrowBook(101, act) == Status::Ok && act == BorrowAction::Borrowed, "Normal borrow succeeds");

    // 2. Fail allocation during returnBook
    mgr.injectNextHistoryAllocationFailure(true);
    bool caughtReturn = false;
    try {
        mgr.returnBook(101);
    } catch (const std::bad_alloc&) {
        caughtReturn = true;
    }
    mgr.searchBook(101, b);
    TEST_ASSERT(caughtReturn, "T33: std::bad_alloc thrown during return");
    TEST_ASSERT(b.available == false, "T33: Book 101 remained Borrowed");
    TEST_ASSERT(mgr.getReturnStack().isEmpty(), "T33: Return stack untouched");
    TEST_ASSERT(mgr.getHistory().getCount() == 1 && mgr.getHistory().getNextTransactionId() == 2,
                "T33: History count and txId untouched");
}

void test_T35_seeded_mixed_operations_invariants() {
    std::cout << "\n=== Manager Tests (T35: Repeated Seeded Mixed Operations) ===\n";
    LibraryManager mgr(5, 3, 3);
    std::uint32_t seed = 42u;
    auto nextRand = [&]() -> std::uint32_t {
        seed = seed * 1664525u + 1013904223u;
        return seed;
    };

    bool allInvariantsOk = true;
    for (int step = 0; step < 300; ++step) {
        std::uint32_t op = nextRand() % 8;
        int id = static_cast<int>((nextRand() % 6) + 1); // IDs 1..6 (includes missing/overflow tests)
        BorrowAction act;
        int outId = 0;

        switch (op) {
            case 0: mgr.addBook(id, "Title " + std::to_string(id), "Author " + std::to_string(id)); break;
            case 1: mgr.deleteBook(id); break;
            case 2: mgr.borrowBook(id, act); break;
            case 3: mgr.returnBook(id); break;
            case 4: mgr.processNextRequest(outId); break;
            case 5: mgr.cancelHeadRequest(outId); break;
            case 6: mgr.popReturnStack(outId); break;
            case 7: mgr.editBookMetadata(id, "Edited " + std::to_string(id), "Editor"); break;
        }

        if (!mgr.verifyAllInvariants()) {
            allInvariantsOk = false;
            break;
        }
    }

    TEST_ASSERT(allInvariantsOk, "T35: All cross-structure invariants held across 300 randomized mixed operations");
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "  SmartLibrary SL-003 LibraryManager Test Suite     \n";
    std::cout << "====================================================\n";

    test_T02_T03_borrow_return_and_repeat_queue();
    test_T10_T19_T20_delete_and_return_guards();
    test_T14_stack_full_rejects_return();
    test_T15_T16_process_and_cancel_requests();
    test_T29_maintenance_validation();
    test_T33_allocation_failure_atomicity();
    test_T35_seeded_mixed_operations_invariants();

    std::cout << "\n====================================================\n";
    std::cout << "Test Summary: " << g_tests_passed << " / " << g_tests_run << " passed.\n";
    std::cout << "====================================================\n";

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}
