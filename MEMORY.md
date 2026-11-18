# Project memory
Group C, CSE 2103. Smart Library Management System. Original teacher PDF and Group C DOCX in references/. Core console C++17, handmade catalogue array, history singly linked list, return stack and circular request queue. No ready-made core containers. Proposed capacities 100 each, configurable for tests. Full CRUD-style inspection/maintenance operations required, without changing LIFO/FIFO. Console first; GUI optional after approval. Deadline and compiler not confirmed. Current progress: documentation only; SL-001 pending. Append verified decisions and milestone results; do not erase earlier decisions without recording rationale.

## Milestone SL-001 Verified Decisions (2026-11-18)
1. **Toolchain Configured**:
   - Compiler: MinGW-w64 G++ 16.2.0 (POSIX threads, UCRT runtime) targeting C++17 (`-Wall -Wextra -Wpedantic`).
   - Build system: CMake 4.4.4 with Ninja 1.13.2 generator.
   - Dual-mode verification: Debug and Release builds validated via CTest.
2. **Book & Status Architecture**:
   - `Status` enum (`Ok`, `Empty`, `Full`, `NotFound`, `Duplicate`, `Invalid`, `Blocked`) decouples core structures from console I/O.
   - `isBlankString` rejects empty and whitespace-only strings.
3. **BookCatalogue Implementation**:
   - Raw dynamic array `Book* m_books`, zero STL containers, Rule of Five implemented, left-shift deletion preserves contiguous `[0, m_count)`.

## Milestones SL-002 to SL-006 Verified Decisions (2026-11-18)
1. **HistoryList (`include/HistoryList.hpp`, `src/HistoryList.cpp`)**:
   - Singly linked list with `m_head` & `m_tail`, monotonic `uint64_t m_nextTxId` starting at 1.
   - `PendingNode` RAII pre-allocation (`prepareAppend` + `noexcept commitAppend`) ensures failure atomicity before any catalogue/stack/queue mutation.
2. **ReturnStack & RequestQueue**:
   - `ReturnStack`: raw `int*` array; top-relative search/update (`(m_count - 1) - topOffset`).
   - `RequestQueue`: raw `int*` circular array tracking `m_front` and `m_count` (`(m_front + offset) % m_capacity`), cleanly supporting capacity 1 without wasting a sentinel slot.
3. **LibraryManager & ConsoleUI**:
   - Separated `smartlibrary_core` (UI-agnostic engine) from `smartlibrary_console_ui` (`ConsoleUI`), enabling optional SL-007 GUI reuse without code duplication.
   - Maintenance submenu (`14`) uses options `1–7` + `0 Back` and strictly prohibits altering book availability status.
   - Whole-line parsing via `std::getline` + `std::strtol`/`std::strtoull` handles malformed strings, integer overflows, long inputs, and EOF cleanly.
4. **Verification & Deliverables**:
   - Debug & Release CTest: 5/5 test executables passed (235/235 total assertions passed).
   - `cppcheck`: 0 warnings/errors across all 10 source files. MinGW lacks `-lubsan` (documented in `docs/TEST_RESULTS.md`).
   - `docs/FINAL_REPORT.md`, `docs/VIVA_GUIDE.md`, `docs/PRESENTATION_OUTLINE.md`, `docs/FLOWCHART.md`, and `RUBRIC.md` updated.
5. **SL-007 Optional GUI & System Hardening**:
   - Built standalone static Windows binaries (`-static` on MinGW) for both `smartlibrary.exe` and `smartlibrary_gui.exe`.
   - Loopback-only (`127.0.0.1`) WinSock2/POSIX HTTP/1.1 adapter (`HttpServer` + mutex-serialized `JsonApiAdapter`) with 3s `SO_RCVTIMEO`, per-connection `std::thread` handling (prevents Chrome preconnect hangs), depth-1 JSON tokenizer, and full maintenance search parity.
   - `HistoryList` copy constructor wrapped in `try/catch` for leak-free exception safety; `trimString` normalizes titles/authors consistently.
   - Verified 20/20 T38 parity assertions (`gui_parity_tests.exe`) and upgraded `web/index.html` with KPI strip, circular queue SVG ring, linked-list pointer cards, inline edit modal, and live audit feed.
