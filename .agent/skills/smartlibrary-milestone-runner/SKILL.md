---
name: smartlibrary-milestone-runner
description: Guides and enforces the milestone execution lifecycle (SL-001 through SL-007) for Smart Library with gate approvals, TRACKER.md/MEMORY.md synchronization, and viva defense generation.
---

# Smart Library Milestone Runner

Use this skill when initiating, planning, executing, or gating any milestone in `TRACKER.md`.

## Milestone Workflow Protocol

1. **Strict Gate Isolation**:
   - Only execute ONE milestone at a time (e.g. SL-001).
   - Never mark future milestones as complete.
   - Never write code for subsequent milestones until the current milestone passes verification and receives explicit user approval.

2. **Pre-Execution Step**:
   - Inspect `TRACKER.md`, `MEMORY.md`, `API_CONTRACT.md`, and `TEST_PLAN.md`.
   - List the planned header and source files to create or modify.
   - List the planned acceptance tests and edge cases.

3. **Execution & Build Verification**:
   - Build using CMake in both Debug and Release configurations when supported.
   - Run tests with CTest or the dedicated test runner executable.
   - Record exact CLI commands, exit codes, and test pass/fail tallies.

4. **Post-Execution Documentation**:
   - Update `TRACKER.md` execution log with:
     - Milestone ID
     - Timestamp
     - Files modified
     - Verification command & result
     - Next milestone gate status
   - Update `MEMORY.md` with verified architectural decisions.
   - Generate student viva prep: key questions, answers, and time complexities for lab defense.
