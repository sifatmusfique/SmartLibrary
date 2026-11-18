---
name: smartlibrary-test-verifier
description: Test execution and verification framework for Smart Library matching T01–T37 test catalog with automated test drivers and boundary edge case checks.
---

# Smart Library Test Verifier

Use this skill to execute, validate, and document tests against `TEST_PLAN.md` (T01 through T37).

## Verification Requirements

1. **Test Mapping**:
   - Every unit and integration test must explicitly map to a test case ID from `TEST_PLAN.md` (e.g. `T01: Add valid book`, `T02: Add book when catalogue is full`, `T14: Stack overflow`, `T21: Queue circular wrap-around`).

2. **Boundary & Edge-Case Coverage**:
   - **Empty state**: Querying, popping, dequeueing, deleting, traversing an empty structure.
   - **Full capacity**: Inserting/pushing/enqueueing past maximum capacity without memory corruption.
   - **Circular wrap-around**: Confirming proper FIFO behavior after multiple enqueue/dequeue cycles in the circular buffer.
   - **LIFO preservation**: Verifying order in the undo/return stack.
   - **Duplicate handling**: Handling duplicate IDs or ISBNs appropriately per specification.
   - **Single-node & multi-node cleanup**: Verifying all dynamically allocated linked list nodes are cleanly freed.

3. **Evidence Recording**:
   - Record actual command-line output into `docs/TEST_RESULTS.md`.
   - Never report fabricated test passes. If a test fails, state the exact failure and diagnostic trace.
