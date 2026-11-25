# Smart Library Management System — CSE 2103 (Group C Section G)

**Student IDs:** `252311282`, `252311284`, `252311285`

A C++17 library management system built on **four manually implemented core data structures** (zero STL containers or `<algorithm>` substitutes for core storage):

1. **`BookCatalogue`** (`include/BookCatalogue.hpp`, `src/BookCatalogue.cpp`) — Bounded raw array (`Book*`) with dense left-shift deletion.
2. **`HistoryList`** (`include/HistoryList.hpp`, `src/HistoryList.cpp`) — Singly linked list with `head`/`tail` pointers, monotonic 64-bit transaction IDs, and failure-atomic node pre-allocation.
3. **`ReturnStack`** (`include/ReturnStack.hpp`, `src/ReturnStack.cpp`) — Bounded LIFO array stack (`int*`) with top-relative offset search/update.
4. **`RequestQueue`** (`include/RequestQueue.hpp`, `src/RequestQueue.cpp`) — Bounded circular FIFO array queue (`int*`) using `(front + count) % capacity`.

---

## Quick Start for Teammates

### Prerequisites
- C++17 compiler (GCC/MinGW-w64, Clang, or MSVC)
- CMake 3.16+ (and Ninja or Make/MSBuild)

### Build & Run Tests
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

### Run Console App
```bash
./build/smartlibrary
```

### Run Optional Live Web Dashboard (Loopback GUI)
```bash
./build/smartlibrary_gui --port 8080
```
Then open **http://127.0.0.1:8080** in your browser.

---

## Project Layout
- `include/` & `src/` — Core data structures, `LibraryManager`, `ConsoleUI`, and loopback HTTP/JSON GUI adapter.
- `web/` — Static HTML/CSS/JS live visualizer (`index.html`).
- `tests/` — 5 automated test suites covering `T01–T38` (`235 / 235` assertions passing in Debug & Release).
- `docs/` — Architecture flowchart, test evidence (`TEST_RESULTS.md`), viva guide (`VIVA_GUIDE.md`), final report (`SmartLibrary_Final_Report.pdf`), presentation slides (`SmartLibrary_Presentation.pdf`), and 3-student walkthrough (`SmartLibrary_Website_Walkthrough_3_Students.pdf`).
- `references/` — Original CSE 2103 project guidelines PDF and Group C proposal DOCX.
