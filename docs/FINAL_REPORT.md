# Smart Library Management System — Final Project Report

**Course**: CSE 2103 (Data Structures Lab)  
**Group**: Group C  
**Student IDs**: `252311282`, `252311284`, `252311285`  
**Member Names**: `[Insert Member Names]`  
**Submission Date**: `2026-11-18`  

---

## 1. Title & Objective

**Project Title**: Smart Library Management System Using Array, Singly Linked List, Stack, and Circular Queue.

**Objectives**:
- Build a menu-driven C++17 library management engine centered on four manually implemented core data structures (`BookCatalogue`, `HistoryList`, `ReturnStack`, `RequestQueue`).
- Strictly avoid ready-made STL container substitutes (`std::vector`, `std::list`, `std::stack`, `std::queue`, `std::map`, `std::set`, `std::priority_queue`) and `<algorithm>` helpers for core logic.
- Support complete CRUD and traversal operations across all four structures (*insert, delete, search, update, traverse*) alongside robust handling of empty, full, overflow, underflow, duplicate, blocked, and not-found edge cases.
- Document and verify time/space complexity and failure atomicity guarantees with automated Debug/Release test suites.

---

## 2. Problem Statement

A library system handles four fundamentally distinct access patterns:
1. **Book Catalogue**: Fast contiguous storage and iteration over a bounded inventory, with unique book IDs and metadata updates.
2. **Transaction History**: Append-only chronological audit trail of borrow and return snapshots that grows dynamically over a session.
3. **Recently Returned Books**: Last-In, First-Out (LIFO) shelf/desk inspection stack where the most recently returned book is processed first.
4. **Waiting Borrow Requests**: First-In, First-Out (FIFO) fair waiting line when requested books are currently borrowed.

No single linear data structure optimizes all four workloads equally well. Combining a bounded array, singly linked list, bounded stack, and circular array queue models each operational concern cleanly while fulfilling academic CSE 2103 requirements.

---

## 3. Data Structures Selection & Alternatives

| Component | Selected Structure | Alternatives Considered | Why Selected |
|---|---|---|---|
| `BookCatalogue` | Bounded Raw Array (`Book*`) | Linked List, BST, Hash Table | Direct index access and contiguous cache locality for a bounded 100-book inventory; cleanly demonstrates dense left-shifting deletion. |
| `HistoryList` | Singly Linked List (`Node*` with `head` & `tail`) | Dynamic Array, Doubly Linked List | $O(1)$ tail append without reallocating or shifting existing historical snapshots; tail pointer avoids $O(h)$ append scans. |
| `ReturnStack` | Bounded Array Stack (`int*`) | Linked Stack | Zero per-push heap allocation; $O(1)$ push/pop with explicit overflow/underflow demonstration. |
| `RequestQueue` | Circular Array Queue (`int*` with `front` & `count`) | Linear Array Queue, Linked Queue | Reuses vacated front slots in $O(1)$ via `(front + count) % capacity` modulo arithmetic without shifting elements on dequeue. Tracking `count` distinguishes full vs. empty cleanly without sacrificing a sentinel slot. |

---

## 4. Operations & Complexity Analysis

Let $n$ = active books, $h$ = history nodes, $s$ = stack items, $q$ = queue items, and $B, S, Q$ = fixed capacities (default 100).

### Primitive Structure Operations
| Structure | Operation | Time Complexity | Extra Space | Notes |
|---|---|---|---|---|
| `BookCatalogue` | `add` | $O(n)$ | $O(1)$ | $O(n)$ uniqueness scan + $O(1)$ append |
| `BookCatalogue` | `remove` | $O(n)$ | $O(1)$ | $O(n)$ search + left-shift $[idx+1, n)$ |
| `BookCatalogue` | `find` / `editMetadata` / `updateStatus` | $O(n)$ | $O(1)$ | Linear scan over unsorted IDs |
| `BookCatalogue` | `traverse` | $O(n)$ | $O(1)$ | Streamed callback, zero copied collections |
| `HistoryList` | `append` / `commitAppend` | $O(1)$ | $O(1)$ per node | Direct $O(1)$ tail pointer insertion |
| `HistoryList` | `find` / `update` / `remove` / `traverse` | $O(h)$ | $O(1)$ | Single pass from `m_head`; updates `m_tail` on tail deletion |
| `ReturnStack` | `push` / `pop` / `peek` / `updateAt` | $O(1)$ | $O(1)$ | Direct index `(m_count - 1 - topOffset)` |
| `ReturnStack` | `find` / `traverse` | $O(s)$ | $O(1)$ | Top-to-bottom scan |
| `RequestQueue` | `enqueue` / `dequeue` / `peek` / `updateAt` | $O(1)$ | $O(1)$ | Modulo wraparound `(m_front + offset) % m_capacity` |
| `RequestQueue` | `contains` / `find` / `traverse` | $O(q)$ | $O(1)$ | Front-to-rear modulo scan |

### Manager & Total Space Complexity
- `borrowBook`: $O(n)$ (catalogue lookup + $O(1)$ history append or $O(1)$ queue enqueue)
- `returnBook`: $O(n)$ (catalogue lookup + $O(1)$ stack push + $O(1)$ history append)
- `processNextRequest`: $O(n)$ (queue peek + catalogue lookup + $O(1)$ dequeue/history append)
- `deleteBook`: $O(n + q)$ (catalogue lookup/shift + queue pending-request check)
- `updateReturnStackSlot` / `updateRequestQueueSlot`: $O(n)$ (validates target book ID exists in catalogue) + $O(1)$ slot write.
- **Total Space**: $O(B + S + Q + h)$ plus string payload lengths.

---

## 5. System Architecture & Design

```text
ConsoleUI (whole-line parsing, menu 0..15, submenu 1..7)
    └── LibraryManager (coordination, consistency rules, failure atomicity)
            ├── BookCatalogue   (bounded raw Book* array)
            ├── HistoryList     (singly linked Node* list with head/tail)
            ├── ReturnStack     (bounded raw int* LIFO array)
            └── RequestQueue    (bounded raw int* circular FIFO array)
```

### Failure Atomicity & Consistency Invariants
1. **Two-Phase History Allocation**: `borrowBook`, `returnBook`, and `processNextRequest` validate all preconditions and pre-allocate the `HistoryList::PendingNode` *before* mutating catalogue status, stack, or queue. If allocation throws `std::bad_alloc`, no logical state or transaction counter changes.
2. **Deletion Protection**: `deleteBook` rejects any book that is currently borrowed or has pending requests in `RequestQueue`.
3. **Maintenance Isolation**: Submenu `14` allows educational inspection and correction of log entries (`BORROW`/`RETURN`) and stack/queue slots without mutating live book availability.

*(Insert verified flowchart from `docs/FLOWCHART.md` and actual console execution screenshots here before exporting to PDF.)*

---

## 6. Modules & Implementation Files

- `include/Book.hpp`: `Status` enum, `isBlankString`, `Book`, and `HistoryRecord`.
- `include/BookCatalogue.hpp`, `src/BookCatalogue.cpp`: Bounded array catalogue with Rule of Five.
- `include/HistoryList.hpp`, `src/HistoryList.cpp`: Singly linked list with `PendingNode` atomic handle and Rule of Five.
- `include/ReturnStack.hpp`, `src/ReturnStack.cpp`: Bounded LIFO stack with top-relative search/update.
- `include/RequestQueue.hpp`, `src/RequestQueue.cpp`: Circular FIFO queue with modulo indexing.
- `include/LibraryManager.hpp`, `src/LibraryManager.cpp`: Central coordinator enforcing consistency rules.
- `include/ConsoleUI.hpp`, `src/ConsoleUI.cpp`, `src/main.cpp`: Whole-line console parser handling menus 0–15, maintenance submenu 1–7, numeric overflow, and EOF.

---

## 7. Test Results & Verification

- **Compiler**: MinGW-w64 G++ 16.2.0 (`-std=c++17 -Wall -Wextra -Wpedantic`), CMake 4.4.4, Ninja 1.13.2.
- **Debug & Release CTest**: **5 / 5 suites passed** in both Debug and Release builds (**235 / 235 total assertions passed**, exit code `0`).
- **Static Analysis (`cppcheck`)**: 0 errors and 0 warnings across all 10 source files.
- **Coverage**: All test IDs `T01` through `T38` verified, including optional GUI parity via `smartlibrary_gui.exe` (detailed evidence in `docs/TEST_RESULTS.md`).

### Captured Console & CTest Execution Evidence
```text
Enter Book ID (positive integer): 101 | Title: Data Structures | Author: Seymour Lipschutz
Book added successfully.
ID: 101 | Title: Data Structures | Author: Seymour Lipschutz | Status: Available
Book borrowed successfully. (TxID: 1)
Book is already borrowed. Request added to queue.
Waiting request queue (front to rear): 101
Book returned successfully. (TxID: 2)
Return stack (top to bottom): 101
Processed waiting request for Book ID 101. Book borrowed successfully. (TxID: 3)
--- Borrow/Return History (3 records) ---
[TxID: 1] Book ID: 101 | Title: Data Structures | Action: BORROW
[TxID: 2] Book ID: 101 | Title: Data Structures | Action: RETURN
[TxID: 3] Book ID: 101 | Title: Data Structures | Action: BORROW
Popped Book ID 101 from return stack.
Stack Underflow!
Book not found.

ctest --test-dir build-release --output-on-failure:
1/5 Test #1: CatalogueTests ...................   Passed    0.03 sec
2/5 Test #2: StructureTests ...................   Passed    0.03 sec
3/5 Test #3: ManagerTests .....................   Passed    0.03 sec
4/5 Test #4: ConsoleTests .....................   Passed    0.03 sec
5/5 Test #5: GuiParityTests ...................   Passed    0.03 sec (20 / 20)
100% tests passed out of 5 (235 / 235 assertions passed)
```

---

## 8. Limitations & Conclusion

**Current Limitations**:
- Data is stored in volatile memory and resets when the program exits.
- Default capacities are bounded at 100 entries per array structure.
- No member accounts, due dates, fine calculation, or file persistence (deferred by course scope).
- Browser UI connects to a single-process loopback server (`127.0.0.1:8080`) with no authentication.

**Conclusion**:
The Smart Library Management System successfully demonstrates how combining four custom-built data structures produces clean, predictable performance and strict invariant safety without relying on STL containers.
