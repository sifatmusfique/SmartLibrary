# Test Evidence — SmartLibrary (CSE 2103 Group C)

## Test Environment
- **Compiler**: MinGW-w64 G++ 16.2.0 (`-std=c++17 -Wall -Wextra -Wpedantic`)
- **Build System**: CMake 4.4.4, Ninja 1.13.2
- **Static Analysis**: `cppcheck --enable=warning,performance,portability --std=c++17` (0 errors, 0 warnings across 7 source files)
- **Sanitizers**: Not supported on installed MinGW-w64 (`ld.exe: cannot find -lubsan`); mitigated via explicit Rule-of-Five unit tests, cycle checks, invariant assertions, and `cppcheck`.

## Execution Summary
- **Debug (`ctest --test-dir build --output-on-failure`)**: 5 / 5 test suites passed (Exit code 0)
- **Release (`ctest --test-dir build-release --output-on-failure`)**: 5 / 5 test suites passed (Exit code 0)
- **Total Verified Assertions**: **235 / 235 passed** (`catalogue_tests`: 76, `structure_tests`: 76, `manager_tests`: 48, `console_tests`: 15, `gui_parity_tests`: 20)
- **Live HTTP Loopback Test (`smartlibrary_gui.exe --port 18080`)**: Served `web/index.html` (HTTP 200, 22,606 bytes) and executed `/api/books/add`, `/api/circulation/borrow`, `/api/circulation/return`, `/api/circulation/process`, and `/api/state` cleanly against `LibraryManager`.
- **Browser Visual Verification**: Deferred because Chrome remote debugging (`DevToolsActivePort`) was not active in the host Chrome profile.

## Test Matrix (T01–T38)

| Test ID | Scenario | Target / Command | Actual Result | Status | Evidence |
|---|---|---|---|---|---|
| T01 | Add 101 Data Structures / Seymour Lipschutz; search | `catalogue_tests.exe` | Returns `Status::Ok`; found Available with exact fields | PASS | `tests/test_catalogue.cpp` |
| T02 | Borrow 101 then return | `manager_tests.exe` | Status toggles Borrowed -> Available; 2 history nodes; stack top 101 | PASS | `tests/test_manager.cpp` |
| T03 | Borrow 101 twice | `manager_tests.exe` | First borrows; second enqueues 101 without history mutation | PASS | `tests/test_manager.cpp` |
| T04 | Return 101 then display stack | `structure_tests.exe`, `console_tests.exe` | Top element verified as 101 | PASS | `tests/test_structures.cpp` |
| T05 | Pop empty stack | `structure_tests.exe`, `console_tests.exe` | `Status::Empty`; prints `Stack Underflow!` | PASS | `tests/test_console.cpp` |
| T06 | Search 999 | `catalogue_tests.exe`, `console_tests.exe` | `Status::NotFound`; prints `Book not found.` | PASS | `tests/test_catalogue.cpp` |
| T07 | Duplicate ID, invalid ID, blank/whitespace title | `catalogue_tests.exe` | Rejected (`Duplicate` / `Invalid`) without mutating count | PASS | `tests/test_catalogue.cpp` |
| T08 | Full catalogue then add | `catalogue_tests.exe` | Capacity-3 rejects 4th with `Status::Full`; count unchanged | PASS | `tests/test_catalogue.cpp` |
| T09 | Delete first/middle/last | `catalogue_tests.exe` | Elements shift left; dense range `[0, count)` maintained | PASS | `tests/test_catalogue.cpp` |
| T10 | Delete borrowed or pending book | `manager_tests.exe` | Rejected with `Status::Blocked` | PASS | `tests/test_manager.cpp` |
| T11 | Queue cap 3 wraparound: enq 1,2,3; deq 1,2; enq 4,5 | `structure_tests.exe` | Front wraps to 2; FIFO order verified as 3, 4, 5 | PASS | `tests/test_structures.cpp` |
| T12 | Full queue; empty dequeue | `structure_tests.exe` | Overflow returns `Status::Full`; underflow returns `Status::Empty` | PASS | `tests/test_structures.cpp` |
| T13 | Two returns then pop twice | `structure_tests.exe` | Pops in strict LIFO reverse order | PASS | `tests/test_structures.cpp` |
| T14 | Full return stack then return | `manager_tests.exe` | Rejected with `Status::Full`; book stays Borrowed; history unchanged | PASS | `tests/test_manager.cpp` |
| T15 | Process available head | `manager_tests.exe` | Dequeues head, borrows book, appends history node | PASS | `tests/test_manager.cpp` |
| T16 | Process unavailable head | `manager_tests.exe` | Returns `Status::Blocked` unchanged; explicit cancel removes head | PASS | `tests/test_manager.cpp` |
| T17 | Empty history; repeated append/destruction | `structure_tests.exe` | Safe 0-node traversal, `clear()`, and deep-copy destructor cleanup | PASS | `tests/test_structures.cpp` |
| T18 | Invalid menu, malformed numeric, EOF | `console_tests.exe` | Rejects invalid lines; exits cleanly on EOF without loop | PASS | `tests/test_console.cpp` |
| T19 | Return already available or missing book | `manager_tests.exe` | Returns `Blocked`/`NotFound` without stack/history changes | PASS | `tests/test_manager.cpp` |
| T20 | Delete available book after history | `manager_tests.exe` | Book deleted; historical snapshot remains readable | PASS | `tests/test_manager.cpp` |
| T21 | Edit title/author; invalid blank; missing ID | `catalogue_tests.exe` | Updates metadata only; ID & status immutable; blanks rejected | PASS | `tests/test_catalogue.cpp` |
| T22 | History search first/middle/last/missing txId | `structure_tests.exe` | Returns exact records and `NotFound` for missing | PASS | `tests/test_structures.cpp` |
| T23 | History update valid/invalid action | `structure_tests.exe` | Accepts `BORROW`/`RETURN`; rejects invalid action/blank title | PASS | `tests/test_structures.cpp` |
| T24 | History delete head/middle/tail/only; re-append | `structure_tests.exe` | Updates `head`/`tail` cleanly; re-append keeps monotonic txId | PASS | `tests/test_structures.cpp` |
| T25 | Stack find duplicates and missing ID | `structure_tests.exe` | Finds top-most offset (0) first; missing returns `NotFound` | PASS | `tests/test_structures.cpp` |
| T26 | Stack update valid/out-of-range offset | `structure_tests.exe` | Updates logical slot; rejects out-of-range; preserves LIFO | PASS | `tests/test_structures.cpp` |
| T27 | Queue find after wraparound and missing ID | `structure_tests.exe` | Finds front-relative offset across wraparound | PASS | `tests/test_structures.cpp` |
| T28 | Queue update wrapped logical offset | `structure_tests.exe` | Updates `(front + offset) % capacity`; preserves FIFO | PASS | `tests/test_structures.cpp` |
| T29 | Maintenance new ID missing or invalid | `manager_tests.exe` | Rejects missing/invalid ID without mutating stack/queue | PASS | `tests/test_manager.cpp` |
| T30 | Empty history/stack/queue search/update/delete | `structure_tests.exe` | Explicitly returns `Status::Empty` without crash | PASS | `tests/test_structures.cpp` |
| T31 | Invalid capacity (0); capacity 1 | `catalogue_tests.exe`, `structure_tests.exe` | Capacity 0 throws `std::invalid_argument`; capacity 1 works | PASS | `tests/test_catalogue.cpp` |
| T32 | Queue cancel after wraparound | `structure_tests.exe` | Removes only front element; exposes next element | PASS | `tests/test_structures.cpp` |
| T33 | History allocation failure injection | `manager_tests.exe` | `std::bad_alloc` thrown before mutation; state & txId unchanged | PASS | `tests/test_manager.cpp` |
| T34 | Long text (500 chars), overflow integer, EOF | `console_tests.exe` | Parsed safely; long string stored; EOF terminates cleanly | PASS | `tests/test_console.cpp` |
| T35 | Repeated seeded mixed operations (300 steps) | `manager_tests.exe` | `verifyAllInvariants()` passed after every single operation | PASS | `tests/test_manager.cpp` |
| T36 | Console scripted original Group C sample inputs | `console_tests.exe`, `smartlibrary.exe` | Exact expected console messages and state verified | PASS | `tests/test_console.cpp` |
| T37 | Debug and Release test runs | `ctest --test-dir build` & `build-release` | 5/5 CTest targets passed in both Debug and Release | PASS | CTest logs |
| T38 | Optional GUI parity (`JsonApiAdapter` & loopback HTTP) | `gui_parity_tests.exe` & `smartlibrary_gui.exe` | 20/20 assertions pass (including maintenance search, JSON string key safety, overflow, trimming); live HTTP endpoints match C++ engine | PASS | `tests/test_gui_parity.cpp` |
