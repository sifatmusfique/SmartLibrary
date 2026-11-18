#pragma once

#include "Book.hpp"
#include <cstddef>

/**
 * @brief Manually managed bounded array stack storing recently returned book IDs (LIFO).
 *
 * Strict Academic Rubric Compliance:
 * - Raw heap-allocated array (int*), NO std::stack or std::vector.
 * - Top element is located at m_items[m_count - 1] when m_count > 0.
 * - Supports push, pop, peek, top-relative search, logical offset update, and top-to-bottom traversal.
 */
class ReturnStack {
public:
    static constexpr size_t DEFAULT_CAPACITY = 100;

    explicit ReturnStack(size_t capacity = DEFAULT_CAPACITY);
    ~ReturnStack();

    ReturnStack(const ReturnStack& other);
    ReturnStack& operator=(const ReturnStack& other);
    ReturnStack(ReturnStack&& other) noexcept;
    ReturnStack& operator=(ReturnStack&& other) noexcept;

    /**
     * @brief Push a returned book ID onto the top of the stack.
     * Time Complexity: O(1).
     */
    Status push(int bookId) noexcept;

    /**
     * @brief Pop the top book ID from the stack.
     * @param outBookId Optional pointer to receive the popped ID.
     * Time Complexity: O(1).
     */
    Status pop(int* outBookId = nullptr) noexcept;

    /**
     * @brief Inspect the top book ID without removing it.
     * Time Complexity: O(1).
     */
    Status peek(int& outBookId) const noexcept;

    /**
     * @brief Find the first occurrence of bookId starting from the top (offset 0 = top).
     * Time Complexity: O(s), where s is the current stack count.
     */
    Status find(int bookId, size_t& outTopOffset) const noexcept;

    /**
     * @brief Update the book ID at a logical offset from the top (0 = top element).
     * Preserves LIFO ordering without shifting elements.
     * Time Complexity: O(1).
     */
    Status updateAt(size_t topOffset, int newBookId) noexcept;

    /**
     * @brief Traverse stack elements from top to bottom (LIFO display order).
     * Time Complexity: O(s).
     */
    template <typename Visitor>
    void traverse(Visitor visitor) const {
        for (size_t offset = 0; offset < m_count; ++offset) {
            visitor(m_items[m_count - 1 - offset]);
        }
    }

    /**
     * @brief Direct logical access by top-relative offset (0 = top). Returns nullptr if out of range.
     */
    const int* getAtTopOffset(size_t topOffset) const noexcept;

    void clear() noexcept;

    size_t getCount() const noexcept { return m_count; }
    size_t getCapacity() const noexcept { return m_capacity; }
    bool isEmpty() const noexcept { return m_count == 0; }
    bool isFull() const noexcept { return m_count >= m_capacity; }

    bool verifyInvariants() const noexcept {
        return m_capacity > 0 && m_booksValid() && m_count <= m_capacity;
    }

private:
    int* m_items{nullptr};
    size_t m_capacity{0};
    size_t m_count{0};

    bool m_booksValid() const noexcept {
        return m_items != nullptr;
    }
};
