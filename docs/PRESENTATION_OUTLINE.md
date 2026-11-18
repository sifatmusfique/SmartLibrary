# Short Presentation Plan (CSE 2103 — Group C)

**Duration**: 5–7 minutes (confirm exact instructor limit before presentation)

1. **Title, Team & Problem Statement (30s)**
   - Smart Library Management System using Array, Singly Linked List, Stack, and Circular Queue.
   - Group C IDs: `252311282`, `252311284`, `252311285`.
   - Problem: Managing catalogue inventory, chronological history, recent returns, and waiting queues using custom manual data structures without STL containers.

2. **Four Handmade Data Structures & Justification (60s)**
   - `BookCatalogue` (Bounded Array): Contiguous storage, $O(1)$ append, $O(n)$ search/left-shift deletion.
   - `HistoryList` (Singly Linked List with `head` & `tail`): $O(1)$ tail append without shifting; dynamic growth.
   - `ReturnStack` (Bounded Array Stack): $O(1)$ LIFO push/pop for most recent returns.
   - `RequestQueue` (Circular Array Queue): $O(1)$ FIFO enqueue/dequeue with `(front + count) % capacity` wraparound.

3. **Architecture & Failure Atomicity (45s)**
   - Layered design: `ConsoleUI` -> `LibraryManager` -> 4 handmade structures.
   - Pre-allocates history nodes before mutating catalogue/stack/queue so failed allocations leave state intact.

4. **Live Console Demo Script (2–3 min)**
   - Run `smartlibrary.exe`:
     1. Add book `101` (`Data Structures` / `Seymour Lipschutz`) -> Search `101`.
     2. Borrow `101` -> Borrow `101` again (shows queued in circular queue).
     3. Return `101` -> Show stack (`101` at top) -> Process waiting request (serves `101` FIFO).
     4. Show history (`BORROW`, `RETURN`, `BORROW` with monotonic TxIDs).
     5. Demonstrate edge cases: Pop empty stack (`Stack Underflow!`), Search `999` (`Book not found.`), Maintenance submenu (`14`).

5. **Complexity, Test Verification & Limitations (60s)**
   - Show Debug & Release CTest results (**4/4 suites, 215/215 assertions passed**) and `cppcheck` clean pass.
   - State current limitations: volatile in-memory storage, bounded capacity (100 default), no user accounts.

6. **Conclusion & Viva Q&A**
   - All three members handle questions on circular modulo wraparound, array left-shifting, linked list tail deletion, and memory ownership.
