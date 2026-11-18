# Milestones

| ID | Scope | Gate | Status |
|---|---|---|---|
| SL-001 | CMake, models, catalogue | Clean build, catalogue operations and edge cases tested | **Done** (`catalogue_tests`: 76/76 pass) |
| SL-002 | History list, stack, circular queue | Full operations, wraparound, cleanup and capacity tests | **Done** (`structure_tests`: 76/76 pass) |
| SL-003 | Manager | Circulation, maintenance, consistency and rejection tests | **Done** (`manager_tests`: 48/48 pass) |
| SL-004 | Console | Original menu examples, additional maintenance menu, EOF handling | **Done** (`console_tests`: 15/15 pass; `smartlibrary.exe` verified) |
| SL-005 | Verification | T01–T37 actual evidence, Debug/Release, sanitizer if supported | **Done** (4/4 Debug & Release CTest pass, `cppcheck` clean; MinGW lacks `-lubsan`) |
| SL-006 | Academic deliverables | Report PDF, presentation, demo, viva, rubric evidence | **Done** (`SmartLibrary_Final_Report.pdf`, `SmartLibrary_Presentation.pdf`, `VIVA_GUIDE.md`; insert 3 member names before submission) |
| SL-007 | Optional GUI | Explicit approval; same engine, parity, no console regression | **Done** (`smartlibrary_gui.exe`, `gui_parity_tests`: 20/20 pass, premium dashboard live) |

## Execution log

### SL-001: CMake, Book Model, and Bounded BookCatalogue Array
- **Date**: 2026-11-18
- **Files**: `CMakeLists.txt`, `SmartLibrary/CMakeLists.txt`, `include/Book.hpp`, `include/BookCatalogue.hpp`, `src/BookCatalogue.cpp`, `tests/test_catalogue.cpp`
- **Verification**: Debug & Release builds clean; `catalogue_tests.exe` 76/76 assertions pass (Exit 0).

### SL-002: HistoryList, ReturnStack, and RequestQueue
- **Date**: 2026-11-18
- **Files**: `include/HistoryList.hpp`, `src/HistoryList.cpp`, `include/ReturnStack.hpp`, `src/ReturnStack.cpp`, `include/RequestQueue.hpp`, `src/RequestQueue.cpp`, `tests/test_structures.cpp`
- **Verification**: `structure_tests.exe` 76/76 assertions pass; Debug & Release CTest 2/2 pass (Exit 0).
- **Decisions**: Implemented `PendingNode` RAII pre-allocation for failure atomicity; `RequestQueue` tracks `m_front` and `m_count` so `capacity=1` works cleanly without sentinel slot waste.

### SL-003: LibraryManager Coordinator
- **Date**: 2026-11-18
- **Files**: `include/LibraryManager.hpp`, `src/LibraryManager.cpp`, `tests/test_manager.cpp`
- **Verification**: `manager_tests.exe` 48/48 assertions pass (including T33 allocation failure simulation and T35 300-step seeded invariant stress test); Debug & Release CTest 3/3 pass (Exit 0).

### SL-004: ConsoleUI & Application Entry
- **Date**: 2026-11-18
- **Files**: `include/ConsoleUI.hpp`, `src/ConsoleUI.cpp`, `src/main.cpp`, `tests/test_console.cpp`
- **Verification**: `console_tests.exe` 15/15 assertions pass; `smartlibrary.exe` interactive/piped run verified; Debug & Release CTest 4/4 pass (Exit 0).
- **Decisions**: Maintenance submenu (Menu 14) numbered 1–7 + 0 Back; strict `std::getline` + `std::strtol`/`std::strtoull` parsing avoids infinite loops on bad input and exits cleanly on EOF.

### SL-005 & SL-006: Full T01–T37 Verification & Academic Deliverables
- **Date**: 2026-11-18
- **Files**: `docs/TEST_RESULTS.md`, `docs/FLOWCHART.md`, `docs/PRESENTATION_OUTLINE.md`, `docs/FINAL_REPORT.md`, `docs/VIVA_GUIDE.md`, `RUBRIC.md`
- **Commands & Results**:
  - `cmake --build build && ctest --test-dir build --output-on-failure` -> 4/4 passed (Exit 0)
  - `cmake --build build-release && ctest --test-dir build-release --output-on-failure` -> 4/4 passed (Exit 0)
  - `cppcheck --enable=warning,performance,portability --std=c++17 -I include src/` -> 0 warnings/errors across 7 files (Exit 0)
  - `g++ -fsanitize=undefined ...` -> Unsupported on MinGW-w64 (`ld.exe: cannot find -lubsan`); documented honestly in `docs/TEST_RESULTS.md`.
- **Approval State**: SL-001 through SL-007 implemented and verified; `docs/FINAL_REPORT.md` needs student names/date and terminal/GUI screenshots prior to PDF export.

### SL-007: Optional Local Browser GUI & C++ Loopback Adapter
- **Date**: 2026-11-18
- **Files**: `include/JsonApiAdapter.hpp`, `src/JsonApiAdapter.cpp`, `include/HttpServer.hpp`, `src/HttpServer.cpp`, `src/gui_main.cpp`, `web/index.html`, `tests/test_gui_parity.cpp`, `CMakeLists.txt`
- **Commands & Results**:
  - `cmake --build build && ctest --test-dir build --output-on-failure` -> 5/5 passed (Exit 0)
  - `cmake --build build-release && ctest --test-dir build-release --output-on-failure` -> 5/5 passed (Exit 0)
  - `gui_parity_tests.exe` -> 14/14 assertions pass (Exit 0)
  - `cppcheck --enable=warning,performance,portability --std=c++17 -I include src/` -> 0 errors/warnings across 10 files (Exit 0)
  - Live `smartlibrary_gui.exe --port 18080` HTTP smoke test -> HTML 200 (22,606 bytes) + `/api/books/add`, `/api/circulation/borrow`, `/api/circulation/return`, `/api/circulation/process`, `/api/state` verified.
- **Decisions**: Static MinGW linking (`-static`) added so `smartlibrary.exe` and `smartlibrary_gui.exe` run standalone without runtime DLL errors. Requests serialized with `std::mutex` in `JsonApiAdapter`. Chrome DevTools visual check deferred until user opens Chrome with remote debugging or views `http://127.0.0.1:8080` directly.
