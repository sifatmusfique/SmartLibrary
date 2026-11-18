# Acceptance tests
Run isolated structure tests and manager integration tests. Use small capacities (e.g. 3) for overflow tests. Record actual commands/output, not fabricated screenshots.
| ID | Scenario | Expected |
|---|---|---|
| T01 | Add 101 Data Structures / Seymour Lipschutz; search | Available, correct fields |
| T02 | Borrow 101 then return | Status changes; two chronological history nodes; stack top 101 |
| T03 | Borrow 101 twice | First borrows, second queues 101 |
| T04 | Return 101 then display stack | 101 at top |
| T05 | Pop empty stack | Stack Underflow! |
| T06 | Search 999 | Book not found |
| T07 | Duplicate ID, invalid ID, blank title | Reject without mutation |
| T08 | Full catalogue then add | Reject; existing books unchanged |
| T09 | Delete first/middle/last | Shift correctly, no gaps |
| T10 | Delete borrowed or pending book | Reject |
| T11 | Queue capacity 3: enqueue 1,2,3; dequeue 1,2; enqueue 4,5 | FIFO 3,4,5; wraparound |
| T12 | Full queue; empty dequeue | Safe overflow/underflow |
| T13 | Two returns then pop twice | Reverse return order |
| T14 | Full return stack then return | Reject without changing status/history |
| T15 | Process available head | Dequeue, borrow, add history |
| T16 | Process unavailable head | No mutation; explicit cancel works |
| T17 | Empty history; repeated list append/destruction | Safe traversal and cleanup |
| T18 | Invalid menu, malformed numeric, EOF | No infinite loop or crash |
| T19 | Return already available or missing book | Reject without history/stack change |
| T20 | Delete available book after history | History remains readable |
Run debug and release builds; sanitizer checks where compiler supports them. Ensure tests still execute in release (avoid assert-only side effects).

## v2 additional mandatory tests
| ID | Scenario | Expected |
|---|---|---|
| T21 | Edit title/author; invalid blank; nonexistent ID | Valid edit only, ID and status unchanged |
| T22 | History search first/middle/last/missing transaction | Correct record or not found |
| T23 | History update valid/invalid action | Only valid correction; catalogue unchanged |
| T24 | History delete head/middle/tail/only node; append again | Correct head/tail and order |
| T25 | Stack find duplicates and missing ID | First top-relative match or not found |
| T26 | Stack update valid/out-of-range offset | Correct logical slot, LIFO order maintained |
| T27 | Queue find after wraparound and missing ID | First front-relative match or not found |
| T28 | Queue update wrapped logical offset | FIFO order preserved |
| T29 | Maintenance new ID missing or invalid | Reject, state unchanged |
| T30 | Empty history/stack/queue search/update/delete | Safe explicit empty/not-found response |
| T31 | Invalid capacity; capacity one | Reject invalid; single slot works |
| T32 | Queue cancel after wraparound | Only head removed |
| T33 | History allocation failure (inject if feasible) | Circulation unchanged; otherwise document untested |
| T34 | Long text, malformed integer, EOF, overflow input | Safe parsing and shutdown |
| T35 | Repeated seeded mixed operations | Invariants checked after each operation |
| T36 | Console scripted original sample inputs | Expected messages and state transitions |
| T37 | Debug and Release test runs | All tests run in both; no assert side-effect dependence |
| T38 | Optional GUI parity (only if approved) | Same engine, same outcomes as console |
Record expected vs actual, command, status, evidence path in docs/TEST_RESULTS.md. Tests are planned, not executed. Use test-only helpers for invariant inspection; never change production semantics for tests.
