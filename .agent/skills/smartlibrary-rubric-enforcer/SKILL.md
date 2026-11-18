---
name: smartlibrary-rubric-enforcer
description: Validates and enforces academic rubric constraints for CSE 2103 Smart Library Management System (pure manual data structures, zero STL container substitutes, C++17 conformance, and clean invariants).
---

# Smart Library Rubric & Constraint Enforcer

Use this skill whenever developing, refactoring, or reviewing any data structure or logic in the Smart Library project.

## Core Directives

1. **Strict Manual Data Structures (Academic Lab Mandate)**:
   - **Catalogue**: Raw fixed-capacity array `Book* books` or `Book books[CAPACITY]` with manual integer index tracking (`count`, `capacity`).
   - **Return Stack**: Raw fixed-capacity array with integer index `top` tracking. Strict LIFO ordering.
   - **Request Queue**: Raw fixed-capacity circular array with `front`, `rear`, `count` pointers and modulo arithmetic `(index + 1) % capacity`. Strict FIFO ordering.
   - **Borrow History**: Singly linked list with manual `Node* head` (and optional `tail`). Each node dynamically allocated with `new` and cleaned up with `delete`.

2. **Absolute STL Container Ban for Core Operations**:
   - **PROHIBITED**: `std::vector`, `std::deque`, `std::list`, `std::stack`, `std::queue`, `std::set`, `std::map`, `std::unordered_map`.
   - **PROHIBITED ALGORITHMS**: `std::sort`, `std::find`, `std::binary_search`, `std::reverse` or any other `<algorithm>` function that abstracts the core algorithm.
   - **ALLOWED**: `std::string`, `std::string_view`, `std::iostream`, `std::stringstream`, `<iomanip>`, `<cstddef>`, `<utility>`, `<optional>`.

3. **Memory Safety & Invariants**:
   - Every class managing heap memory (`new` / `new[]`) MUST implement the Rule of Three/Five (Destructor, Copy Constructor, Copy Assignment Operator) or explicitly delete copy operations.
   - Proper bounds checking on all index operations with descriptive boolean returns or `Result`/`Status` codes (no unhandled array overflows or out-of-bounds reads).
   - Clear explanatory comments on time/space complexity (e.g., insertion $O(1)$, linear search $O(n)$, binary search $O(\log n)$) for examination viva readiness.

4. **Preserve Student Understanding**:
   - Clean, idiomatic modern C++17 without overly esoteric template metaprogramming or obscure macro magic.
   - Code must be straightforward for undergraduate students to read, explain, and defend during viva.
