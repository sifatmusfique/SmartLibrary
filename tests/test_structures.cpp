#include "HistoryList.hpp"
#include "ReturnStack.hpp"
#include "RequestQueue.hpp"
#include <iostream>
#include <stdexcept>
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

void test_history_list_operations() {
    std::cout << "\n=== HistoryList Tests (T17, T22, T23, T24, T30) ===\n";
    HistoryList history;
    TEST_ASSERT(history.isEmpty(), "T17: New history list is empty");
    TEST_ASSERT(history.verifyInvariants(), "T17: Empty list invariants hold");

    // T30: Empty search/update/delete
    HistoryRecord rec;
    TEST_ASSERT(history.find(1, rec) == Status::Empty, "T30: Search on empty history returns Empty");
    TEST_ASSERT(history.update(1, "New Title", "BORROW") == Status::Empty, "T30: Update on empty history returns Empty");
    TEST_ASSERT(history.remove(1) == Status::Empty, "T30: Delete on empty history returns Empty");

    // Empty traversal
    int countVisited = 0;
    history.traverse([&](const HistoryRecord&) { ++countVisited; });
    TEST_ASSERT(countVisited == 0, "T17: Traversing empty history visits 0 nodes");

    // Append 4 records
    std::uint64_t tx1 = 0, tx2 = 0, tx3 = 0, tx4 = 0;
    TEST_ASSERT(history.append(101, "Data Structures", "BORROW", &tx1) == Status::Ok && tx1 == 1, "Append tx1");
    TEST_ASSERT(history.append(102, "Algorithms", "BORROW", &tx2) == Status::Ok && tx2 == 2, "Append tx2");
    TEST_ASSERT(history.append(101, "Data Structures", "RETURN", &tx3) == Status::Ok && tx3 == 3, "Append tx3");
    TEST_ASSERT(history.append(103, "OS Concepts", "BORROW", &tx4) == Status::Ok && tx4 == 4, "Append tx4");
    TEST_ASSERT(history.getCount() == 4 && history.verifyInvariants(), "History count is 4 and invariants hold");

    // T22: Search first, middle, last, missing
    TEST_ASSERT(history.find(tx1, rec) == Status::Ok && rec.bookId == 101 && rec.action == "BORROW", "T22: Find first node");
    TEST_ASSERT(history.find(tx2, rec) == Status::Ok && rec.bookId == 102, "T22: Find middle node");
    TEST_ASSERT(history.find(tx4, rec) == Status::Ok && rec.bookId == 103, "T22: Find last node");
    TEST_ASSERT(history.find(999, rec) == Status::NotFound, "T22: Missing txId returns NotFound");
    TEST_ASSERT(history.find(0, rec) == Status::Invalid, "T22: Zero txId returns Invalid");

    // T23: Update valid and invalid actions
    TEST_ASSERT(history.update(tx2, "Intro to Algorithms", "RETURN") == Status::Ok, "T23: Valid history update succeeds");
    history.find(tx2, rec);
    TEST_ASSERT(rec.title == "Intro to Algorithms" && rec.action == "RETURN" && rec.bookId == 102 && rec.transactionId == tx2,
                "T23: Updated fields match while IDs preserved");
    TEST_ASSERT(history.update(tx2, "Valid Title", "INVALID_ACT") == Status::Invalid, "T23: Invalid action rejected");
    TEST_ASSERT(history.update(tx2, "   ", "BORROW") == Status::Invalid, "T23: Blank title rejected");
    TEST_ASSERT(history.update(999, "Title", "BORROW") == Status::NotFound, "T23: Update missing txId returns NotFound");

    // T24: Delete middle, head, tail, only node; then append again
    TEST_ASSERT(history.remove(tx2) == Status::Ok && history.getCount() == 3 && history.verifyInvariants(), "T24: Delete middle tx2");
    TEST_ASSERT(history.remove(tx1) == Status::Ok && history.getCount() == 2 && history.verifyInvariants(), "T24: Delete head tx1");
    TEST_ASSERT(history.remove(tx4) == Status::Ok && history.getCount() == 1 && history.verifyInvariants(), "T24: Delete tail tx4");
    TEST_ASSERT(history.remove(tx3) == Status::Ok && history.isEmpty() && history.verifyInvariants(), "T24: Delete only remaining node tx3");

    // Append again after full deletion — must assign strictly monotonic txId (5)
    std::uint64_t tx5 = 0;
    TEST_ASSERT(history.append(105, "Compilers", "BORROW", &tx5) == Status::Ok && tx5 == 5,
                "T24: Re-append after full delete keeps monotonic txId = 5");
    TEST_ASSERT(history.getCount() == 1 && history.verifyInvariants(), "T24: Invariants intact after re-append");

    // Deep copy and cleanup check
    HistoryList copy(history);
    copy.append(106, "Networks", "BORROW");
    TEST_ASSERT(copy.getCount() == 2 && history.getCount() == 1, "History deep copy isolates mutations");
    history.clear();
    TEST_ASSERT(history.isEmpty() && history.verifyInvariants(), "T17: Clear resets history cleanly");
}

void test_return_stack_operations() {
    std::cout << "\n=== ReturnStack Tests (T04, T05, T13, T25, T26, T30, T31) ===\n";
    // T31: Invalid capacity & capacity 1
    bool caughtZero = false;
    try {
        ReturnStack invalidStack(0);
    } catch (const std::invalid_argument&) {
        caughtZero = true;
    }
    TEST_ASSERT(caughtZero, "T31: ReturnStack(0) throws invalid_argument");

    ReturnStack singleStack(1);
    TEST_ASSERT(singleStack.push(101) == Status::Ok && singleStack.isFull(), "T31: Capacity 1 push succeeds");
    TEST_ASSERT(singleStack.push(102) == Status::Full, "T31: Capacity 1 overflow returns Full");
    int popped = 0;
    TEST_ASSERT(singleStack.pop(&popped) == Status::Ok && popped == 101 && singleStack.isEmpty(), "T31: Capacity 1 pop returns 101");

    // T05 & T30: Underflow & empty operations
    ReturnStack stack(3);
    TEST_ASSERT(stack.pop(&popped) == Status::Empty, "T05: Pop empty stack returns Empty");
    int peeked = 0;
    TEST_ASSERT(stack.peek(peeked) == Status::Empty, "T30: Peek empty stack returns Empty");
    size_t offset = 99;
    TEST_ASSERT(stack.find(101, offset) == Status::Empty, "T30: Find on empty stack returns Empty");
    TEST_ASSERT(stack.updateAt(0, 101) == Status::Empty, "T30: Update on empty stack returns Empty");

    // T04 & T13: Push and LIFO pop
    TEST_ASSERT(stack.push(101) == Status::Ok, "Push 101");
    TEST_ASSERT(stack.push(202) == Status::Ok, "Push 202");
    TEST_ASSERT(stack.peek(peeked) == Status::Ok && peeked == 202, "T04: Top element is 202");

    // T25: Find duplicate ID from top
    TEST_ASSERT(stack.push(101) == Status::Ok, "Push duplicate 101 at top");
    // Stack top-to-bottom: [0]=101, [1]=202, [2]=101
    TEST_ASSERT(stack.find(101, offset) == Status::Ok && offset == 0, "T25: Find 101 matches top-relative offset 0 first");
    TEST_ASSERT(stack.find(202, offset) == Status::Ok && offset == 1, "T25: Find 202 matches top-relative offset 1");
    TEST_ASSERT(stack.find(999, offset) == Status::NotFound, "T25: Missing ID returns NotFound");

    // T26: Update valid and out-of-range offset
    TEST_ASSERT(stack.updateAt(1, 303) == Status::Ok, "T26: Update slot at topOffset 1 to 303");
    TEST_ASSERT(stack.updateAt(3, 404) == Status::Invalid, "T26: Out-of-range offset 3 rejected");
    TEST_ASSERT(stack.updateAt(0, -5) == Status::Invalid, "T26: Non-positive new ID rejected");

    // Verify traversal order: top-to-bottom should be 101, 303, 101
    int order[3] = {0, 0, 0};
    int idx = 0;
    stack.traverse([&](int id) { order[idx++] = id; });
    TEST_ASSERT(order[0] == 101 && order[1] == 303 && order[2] == 101, "T26: LIFO order preserved after update");

    // Pop in reverse order
    int p1 = 0, p2 = 0, p3 = 0;
    stack.pop(&p1);
    stack.pop(&p2);
    stack.pop(&p3);
    TEST_ASSERT(p1 == 101 && p2 == 303 && p3 == 101 && stack.isEmpty(), "T13: Popped in exact LIFO order");
}

void test_request_queue_operations() {
    std::cout << "\n=== RequestQueue Tests (T11, T12, T27, T28, T30, T31, T32) ===\n";
    // T31: Invalid capacity & capacity 1
    bool caughtZero = false;
    try {
        RequestQueue invalidQueue(0);
    } catch (const std::invalid_argument&) {
        caughtZero = true;
    }
    TEST_ASSERT(caughtZero, "T31: RequestQueue(0) throws invalid_argument");

    RequestQueue singleQ(1);
    TEST_ASSERT(singleQ.enqueue(10) == Status::Ok && singleQ.isFull(), "T31: Queue capacity 1 enqueue succeeds");
    TEST_ASSERT(singleQ.enqueue(20) == Status::Full, "T31: Queue capacity 1 overflow rejected");
    int dq = 0;
    TEST_ASSERT(singleQ.dequeue(&dq) == Status::Ok && dq == 10 && singleQ.isEmpty(), "T31: Queue capacity 1 dequeue succeeds");

    // T12 & T30: Empty queue handling
    RequestQueue q(3);
    TEST_ASSERT(q.dequeue(&dq) == Status::Empty, "T12: Dequeue empty queue returns Empty");
    size_t offset = 99;
    TEST_ASSERT(q.find(1, offset) == Status::Empty, "T30: Find on empty queue returns Empty");
    TEST_ASSERT(q.updateAt(0, 1) == Status::Empty, "T30: Update on empty queue returns Empty");

    // T11: Capacity 3 wraparound: enqueue 1,2,3 -> dequeue 1,2 -> enqueue 4,5 -> FIFO 3,4,5
    TEST_ASSERT(q.enqueue(1) == Status::Ok, "T11: Enqueue 1");
    TEST_ASSERT(q.enqueue(2) == Status::Ok, "T11: Enqueue 2");
    TEST_ASSERT(q.enqueue(3) == Status::Ok, "T11: Enqueue 3");
    TEST_ASSERT(q.enqueue(99) == Status::Full, "T12: Enqueue on full queue rejected");

    int d1 = 0, d2 = 0;
    TEST_ASSERT(q.dequeue(&d1) == Status::Ok && d1 == 1, "T11: Dequeue 1");
    TEST_ASSERT(q.dequeue(&d2) == Status::Ok && d2 == 2, "T11: Dequeue 2");
    TEST_ASSERT(q.getFrontIndex() == 2, "T11: Front wrapped to index 2");

    TEST_ASSERT(q.enqueue(4) == Status::Ok, "T11: Enqueue 4 wraps around to index 0");
    TEST_ASSERT(q.enqueue(5) == Status::Ok, "T11: Enqueue 5 wraps around to index 1");
    TEST_ASSERT(q.isFull() && q.verifyInvariants(), "T11: Queue full after wraparound");

    int fifo[3] = {0, 0, 0};
    int idx = 0;
    q.traverse([&](int id) { fifo[idx++] = id; });
    TEST_ASSERT(fifo[0] == 3 && fifo[1] == 4 && fifo[2] == 5, "T11: Traversal order is FIFO 3, 4, 5 after wraparound");

    // T27: Find after wraparound
    TEST_ASSERT(q.find(3, offset) == Status::Ok && offset == 0, "T27: Find 3 at frontOffset 0");
    TEST_ASSERT(q.find(4, offset) == Status::Ok && offset == 1, "T27: Find 4 at wrapped frontOffset 1");
    TEST_ASSERT(q.find(5, offset) == Status::Ok && offset == 2, "T27: Find 5 at wrapped frontOffset 2");
    TEST_ASSERT(q.find(999, offset) == Status::NotFound, "T27: Missing ID returns NotFound");

    // T28: Update wrapped logical offset
    TEST_ASSERT(q.updateAt(1, 44) == Status::Ok, "T28: Update wrapped frontOffset 1 to 44");
    TEST_ASSERT(q.updateAt(3, 55) == Status::Invalid, "T28: Out-of-range frontOffset 3 rejected");
    idx = 0;
    q.traverse([&](int id) { fifo[idx++] = id; });
    TEST_ASSERT(fifo[0] == 3 && fifo[1] == 44 && fifo[2] == 5, "T28: FIFO order 3, 44, 5 preserved");

    // T32: Cancel head after wraparound
    int headCancelled = 0;
    TEST_ASSERT(q.dequeue(&headCancelled) == Status::Ok && headCancelled == 3, "T32: Cancel head removed only 3");
    int peekedHead = 0;
    TEST_ASSERT(q.peek(peekedHead) == Status::Ok && peekedHead == 44 && q.getCount() == 2, "T32: Remaining front is 44");
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "  SmartLibrary SL-002 Data Structures Test Suite    \n";
    std::cout << "====================================================\n";

    test_history_list_operations();
    test_return_stack_operations();
    test_request_queue_operations();

    std::cout << "\n====================================================\n";
    std::cout << "Test Summary: " << g_tests_passed << " / " << g_tests_run << " passed.\n";
    std::cout << "====================================================\n";

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}
