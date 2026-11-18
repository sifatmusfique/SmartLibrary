# Requirements
Source: Group C project paper (PROJECT_PAPER.txt). IDs: 252311282, 252311284, 252311285.
## Mandatory
- Bounded array catalogue: add, delete, search by ID, update status, display.
- Singly linked list: append borrow/return history (ID, title, action), traverse, handle empty.
- Bounded stack: push returned ID, pop, display top-to-bottom, overflow/underflow.
- Circular array queue: enqueue unavailable-book requests, dequeue/process, display, overflow/underflow.
- Borrow and return update catalogue and history; return also pushes stack. Handle not found, full, empty and invalid input.
- No persistence; menu-driven console. GUI optional only after core acceptance.
## Proposed decisions (not specified by paper)
Positive unique book IDs, one copy per ID, title/author nonempty, all capacities 100 by default and configurable for tests. Reject duplicate IDs. A request stores book ID only; repeated requests are allowed because member accounts are out of scope. Return does not automatically fulfill requests; processRequest handles FIFO head only. If head book unavailable, report blocked and leave queue unchanged; user may explicitly dequeue/cancel head. Delete only available books with no pending requests; retain historical snapshots and recent-return IDs even after deletion. Reject reuse of deleted IDs only if explicitly approved; default allows reuse, history is historical. Stack overflow rejects return without mutation (user can pop first); queue overflow rejects request. Invalid operations leave state unchanged. Explain these tradeoffs to teacher and obtain approval if needed.
## Menu contract
1 Add, 2 Delete, 3 Search, 4 Display, 5 Borrow, 6 Return, 7 Show history, 8 Process request, 9 Show recent stack, 10 Pop stack, 11 Show queue, 12 Cancel head request, 0 Exit. Numbers 1,3,5,6,9,10 follow paper examples; remaining numbers proposed. Status updates occur via borrow/return only to preserve consistency.

## v2 superseding requirements — full operation coverage
The following extends the original menu contract; keep existing numbers. All structures expose search/update/traversal and normal insert/delete. History uses unique monotonically increasing transaction IDs; records retain book ID/title/action snapshots. History maintenance (update/delete) is exposed in a clearly labeled educational maintenance menu, not silently performed by circulation. Stack search/update uses logical position from top, queue search/update uses logical position from front; preserve order. Search by book ID returns first match in logical traversal order, or not-found. Stack/queue delete means pop/dequeue, not arbitrary removal. Maintenance update of stack/queue ID must refer to a valid catalogue book for new entries; history annotations may be corrected without changing circulation status. Existing historical IDs may outlive catalogue deletion. History update/delete explicitly labeled as editing educational log, not reversing a transaction. No direct status update bypass: catalogue primitive supports it; manager controls it.
13 Edit book title/author (ID immutable); 14 Maintenance submenu (history search/edit/delete, stack search/update, queue search/update); 15 Help/operation explanation. Exact submenu numbering to be documented by agent before implementation. History correction may edit title/action but action must be BORROW or RETURN; transaction ID immutable. All invalid operations produce explicit feedback. All data is volatile. No actual code or execution is claimed in this kit.
## Acceptance
All teacher requirements mapped in RUBRIC.md; all test cases executed with recorded outputs. GUI is optional after console milestone, not a prerequisite. Use same C++ engine for both interfaces.
