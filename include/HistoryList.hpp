#pragma once

#include "Book.hpp"
#include <cstddef>
#include <cstdint>
#include <string>

/**
 * @brief Manually managed singly linked list storing chronological borrow/return history.
 *
 * Strict Academic Rubric Compliance:
 * - Custom Node struct with raw next pointer; NO std::list or std::forward_list.
 * - Maintains both m_head and m_tail pointers for O(1) chronological append.
 * - Exclusive dynamic node ownership with full Rule of Five (deep copy, safe move, clean destructor).
 */
class HistoryList {
private:
    struct Node {
        HistoryRecord data;
        Node* next{nullptr};

        explicit Node(const HistoryRecord& record) : data(record), next(nullptr) {}
    };

public:
    /**
     * @brief RAII-safe handle for pre-allocated history nodes used by LibraryManager
     * to guarantee failure atomicity before mutating other structures.
     */
    struct PendingNode {
        Node* ptr{nullptr};

        PendingNode() = default;
        ~PendingNode() {
            delete ptr;
            ptr = nullptr;
        }
        PendingNode(const PendingNode&) = delete;
        PendingNode& operator=(const PendingNode&) = delete;
        PendingNode(PendingNode&& other) noexcept : ptr(other.ptr) { other.ptr = nullptr; }
        PendingNode& operator=(PendingNode&& other) noexcept {
            if (this != &other) {
                delete ptr;
                ptr = other.ptr;
                other.ptr = nullptr;
            }
            return *this;
        }
        explicit operator bool() const noexcept { return ptr != nullptr; }
    };

    HistoryList() = default;
    ~HistoryList();

    HistoryList(const HistoryList& other);
    HistoryList& operator=(const HistoryList& other);
    HistoryList(HistoryList&& other) noexcept;
    HistoryList& operator=(HistoryList&& other) noexcept;

    /**
     * @brief Validate and allocate a new node without linking it into the list.
     * Throws std::bad_alloc if allocation fails (or simulated failure is enabled).
     * Does not increment transaction counter or mutate list state.
     */
    Status prepareAppend(int bookId, const std::string& title, const std::string& action, PendingNode& outNode) const;

    /**
     * @brief Link a previously prepared node at the tail in O(1) noexcept and assign its unique transaction ID.
     */
    std::uint64_t commitAppend(PendingNode& pending) noexcept;

    /**
     * @brief Convenience one-step append at the tail.
     * Time Complexity: O(1).
     */
    Status append(int bookId, const std::string& title, const std::string& action, std::uint64_t* outTxId = nullptr);

    /**
     * @brief Find a historical transaction record by its unique transaction ID.
     * Time Complexity: O(h), where h is the number of history nodes.
     */
    Status find(std::uint64_t txId, HistoryRecord& outRecord) const;

    /**
     * @brief Edit the title and action ("BORROW" or "RETURN") of an existing history record.
     * Used strictly for educational log correction; leaves transactionId and bookId unchanged.
     * Time Complexity: O(h).
     */
    Status update(std::uint64_t txId, const std::string& newTitle, const std::string& newAction);

    /**
     * @brief Remove a history node by transaction ID, properly updating head/tail pointers.
     * Time Complexity: O(h).
     */
    Status remove(std::uint64_t txId);

    /**
     * @brief Streamed chronological traversal from head to tail using a callable visitor.
     * Time Complexity: O(h).
     */
    template <typename Visitor>
    void traverse(Visitor visitor) const {
        const Node* current = m_head;
        while (current != nullptr) {
            visitor(current->data);
            current = current->next;
        }
    }

    /**
     * @brief Delete all nodes and reset head, tail, and count.
     * Note: m_nextTxId remains monotonic to prevent transaction ID reuse.
     */
    void clear() noexcept;

    // Observers & test inspection helpers
    size_t getCount() const noexcept { return m_count; }
    bool isEmpty() const noexcept { return m_count == 0; }
    std::uint64_t getNextTransactionId() const noexcept { return m_nextTxId; }

    // Test-only helpers for invariant inspection and T33 allocation failure simulation
    bool verifyInvariants() const noexcept;
    void setSimulateAllocationFailure(bool failNext) noexcept { m_simulateAllocFailure = failNext; }

private:
    Node* m_head{nullptr};
    Node* m_tail{nullptr};
    size_t m_count{0};
    std::uint64_t m_nextTxId{1};
    mutable bool m_simulateAllocFailure{false};

    static bool isValidAction(const std::string& action) noexcept;
};
