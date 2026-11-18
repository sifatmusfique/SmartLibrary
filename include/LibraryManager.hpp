#pragma once

#include "BookCatalogue.hpp"
#include "HistoryList.hpp"
#include "RequestQueue.hpp"
#include "ReturnStack.hpp"
#include <cstddef>
#include <cstdint>
#include <string>

/**
 * @brief Outcome distinction for borrowBook(): either borrowed immediately or queued.
 */
enum class BorrowAction {
    None,
    Borrowed,
    Queued
};

/**
 * @brief Coordinates all four handmade data structures with failure-atomic circulation rules
 * and isolated educational maintenance operations.
 */
class LibraryManager {
public:
    explicit LibraryManager(
        size_t catalogueCapacity = BookCatalogue::DEFAULT_CAPACITY,
        size_t stackCapacity = ReturnStack::DEFAULT_CAPACITY,
        size_t queueCapacity = RequestQueue::DEFAULT_CAPACITY
    );

    // --- Catalogue operations ---
    Status addBook(int id, const std::string& title, const std::string& author);
    Status deleteBook(int id);
    Status searchBook(int id, Book& outBook) const;
    Status editBookMetadata(int id, const std::string& newTitle, const std::string& newAuthor);

    // --- Circulation operations ---
    Status borrowBook(int id, BorrowAction& outAction, std::uint64_t* outTxId = nullptr);
    Status returnBook(int id, std::uint64_t* outTxId = nullptr);
    Status processNextRequest(int& outBookId, std::uint64_t* outTxId = nullptr);
    Status cancelHeadRequest(int& outBookId);
    Status popReturnStack(int& outBookId);

    // --- Maintenance operations (separate from circulation) ---
    Status findHistory(std::uint64_t txId, HistoryRecord& outRecord) const;
    Status editHistory(std::uint64_t txId, const std::string& newTitle, const std::string& newAction);
    Status deleteHistory(std::uint64_t txId);

    Status findInReturnStack(int bookId, size_t& outTopOffset) const;
    Status updateReturnStackSlot(size_t topOffset, int newBookId);

    Status findInRequestQueue(int bookId, size_t& outFrontOffset) const;
    Status updateRequestQueueSlot(size_t frontOffset, int newBookId);

    // --- Const observers ---
    const BookCatalogue& getCatalogue() const noexcept { return m_catalogue; }
    const HistoryList& getHistory() const noexcept { return m_history; }
    const ReturnStack& getReturnStack() const noexcept { return m_returnStack; }
    const RequestQueue& getRequestQueue() const noexcept { return m_requestQueue; }

    // --- Verification & test helpers ---
    bool verifyAllInvariants() const noexcept;
    void injectNextHistoryAllocationFailure(bool failNext) noexcept;

private:
    BookCatalogue m_catalogue;
    HistoryList m_history;
    ReturnStack m_returnStack;
    RequestQueue m_requestQueue;
};
