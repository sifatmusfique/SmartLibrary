#pragma once

#include "Book.hpp"
#include <cstddef>

/**
 * @brief Manually managed bounded circular array queue for waiting borrow requests (FIFO).
 *
 * Strict Academic Rubric Compliance:
 * - Raw heap-allocated circular array (int*), NO std::queue or std::deque.
 * - Uses m_front and m_count to distinguish full (m_count == m_capacity) from empty (m_count == 0)
 *   without wasting a sentinel slot (so capacity=1 supports 1 item cleanly).
 * - Insertion index (rear) = (m_front + m_count) % m_capacity.
 */
class RequestQueue {
public:
    static constexpr size_t DEFAULT_CAPACITY = 100;

    explicit RequestQueue(size_t capacity = DEFAULT_CAPACITY);
    ~RequestQueue();

    RequestQueue(const RequestQueue& other);
    RequestQueue& operator=(const RequestQueue& other);
    RequestQueue(RequestQueue&& other) noexcept;
    RequestQueue& operator=(RequestQueue&& other) noexcept;

    /**
     * @brief Add a requested book ID to the rear of the circular queue.
     * Time Complexity: O(1).
     */
    Status enqueue(int bookId) noexcept;

    /**
     * @brief Remove the front book ID from the queue.
     * @param outBookId Optional pointer to receive the dequeued ID.
     * Time Complexity: O(1).
     */
    Status dequeue(int* outBookId = nullptr) noexcept;

    /**
     * @brief Inspect the front book ID without removing it.
     * Time Complexity: O(1).
     */
    Status peek(int& outBookId) const noexcept;

    /**
     * @brief Check whether any pending request exists for the given book ID.
     * Time Complexity: O(q), where q is current queue count.
     */
    bool contains(int bookId) const noexcept;

    /**
     * @brief Find the first occurrence of bookId starting from the front (offset 0 = front).
     * Time Complexity: O(q).
     */
    Status find(int bookId, size_t& outFrontOffset) const noexcept;

    /**
     * @brief Update the book ID at a logical offset from the front (0 = front element).
     * Physical slot = (m_front + frontOffset) % m_capacity. Preserves FIFO order.
     * Time Complexity: O(1).
     */
    Status updateAt(size_t frontOffset, int newBookId) noexcept;

    /**
     * @brief Traverse queued elements from front to rear (FIFO order).
     * Time Complexity: O(q).
     */
    template <typename Visitor>
    void traverse(Visitor visitor) const {
        for (size_t offset = 0; offset < m_count; ++offset) {
            size_t physicalIdx = (m_front + offset) % m_capacity;
            visitor(m_items[physicalIdx]);
        }
    }

    /**
     * @brief Direct logical access by front-relative offset (0 = front). Returns nullptr if out of range.
     */
    const int* getAtFrontOffset(size_t frontOffset) const noexcept;

    void clear() noexcept;

    size_t getCount() const noexcept { return m_count; }
    size_t getCapacity() const noexcept { return m_capacity; }
    size_t getFrontIndex() const noexcept { return m_front; }
    bool isEmpty() const noexcept { return m_count == 0; }
    bool isFull() const noexcept { return m_count >= m_capacity; }

    bool verifyInvariants() const noexcept {
        return m_capacity > 0 && m_items != nullptr && m_front < m_capacity && m_count <= m_capacity;
    }

private:
    int* m_items{nullptr};
    size_t m_capacity{0};
    size_t m_front{0};
    size_t m_count{0};
};
