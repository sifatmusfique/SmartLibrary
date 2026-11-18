# Viva Defense & Explanation Guide (CSE 2103 — Group C)

Every group member (`252311282`, `252311284`, `252311285`) must be able to trace and explain every module.

---

## Key Viva Questions & Model Answers

1. **Why did you avoid STL containers (`std::vector`, `std::list`, `std::stack`, `std::queue`)?**
   - Course rubric prohibits ready-made STL containers for core storage. Every structure allocates its own memory (`new Book[]`, `new int[]`, `new Node`) and implements manual algorithms and Rule-of-Five cleanup (`delete[]` / `delete`). Only `std::string` and I/O streams are used for text and terminal interaction.

2. **How does `BookCatalogue::remove(id)` keep the array dense without gaps?**
   - After locating index `idx` via linear search ($O(n)$), every element from `idx + 1` to `m_count - 1` is shifted one slot left (`m_books[i] = std::move(m_books[i + 1])`), the vacated tail slot is reset, and `m_count` is decremented. Occupied items always reside in `[0, m_count)`.

3. **How does `RequestQueue` handle circular wraparound and distinguish Full from Empty?**
   - We maintain `m_front` (index of oldest element) and `m_count` (current item count).
   - Rear insertion index is `(m_front + m_count) % m_capacity`.
   - Dequeue advances `m_front = (m_front + 1) % m_capacity` and decrements `m_count`.
   - Tracking `m_count` avoids the classic circular-buffer ambiguity where `front == rear` means both empty and full, allowing `capacity = 1` to hold a full item without wasting a sentinel slot.

4. **Why does `HistoryList` use both `m_head` and `m_tail` pointers?**
   - With `m_tail`, appending a new borrow/return snapshot takes $O(1)$ time instead of traversing $O(h)$ nodes from `m_head`. When removing the tail node in `remove(txId)`, we update `m_tail = prev` (or `nullptr` if the list becomes empty) to keep `m_tail` valid.

5. **How do logical offsets work in `ReturnStack` and `RequestQueue` maintenance?**
   - Stack `topOffset = 0` maps to physical index `(m_count - 1) - topOffset` (top-to-bottom).
   - Queue `frontOffset = 0` maps to physical index `(m_front + frontOffset) % m_capacity` (front-to-rear).
   - Updating a slot validates that the new book ID exists in `BookCatalogue` ($O(n)$) and overwrites only that slot ($O(1)$), preserving strict LIFO/FIFO order.

6. **How do you guarantee Failure Atomicity in `LibraryManager`?**
   - Before modifying `BookCatalogue`, `ReturnStack`, or `RequestQueue`, `LibraryManager` calls `m_history.prepareAppend(...)` which validates inputs and allocates the `Node` inside an RAII `PendingNode` handle. If `new Node` fails (`std::bad_alloc`), the pending handle never links into the list and no catalogue/stack/queue/transaction-ID state changes.

7. **Why does returning a book NOT automatically lend it to the first person in `RequestQueue`?**
   - Per our explicit policy in `API_CONTRACT.md`, returning places the book on the physical `ReturnStack` and marks it `Available`. Processing the FIFO queue head is an explicit librarian step (Menu `8`). If the head request's book is unavailable, `processNextRequest` returns `Status::Blocked` and leaves the queue intact rather than skipping ahead (preserving FIFO fairness).

8. **Why do you use whole-line parsing (`std::getline`) in `ConsoleUI` instead of `std::cin >>`?**
   - Using `std::cin >> int` leaves invalid characters or trailing text in the input stream, causing infinite loops on bad input like `"abc"`. Reading lines with `std::getline` and validating via `std::strtol` cleanly rejects `"12abc"`, integer overflows, and blank lines, while detecting `EOF` cleanly.
