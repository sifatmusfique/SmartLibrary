#include "LibraryManager.hpp"

LibraryManager::LibraryManager(
    size_t catalogueCapacity,
    size_t stackCapacity,
    size_t queueCapacity)
    : m_catalogue(catalogueCapacity),
      m_returnStack(stackCapacity),
      m_requestQueue(queueCapacity) {}

Status LibraryManager::addBook(int id, const std::string& title, const std::string& author) {
    return m_catalogue.add(Book(id, title, author, true));
}

Status LibraryManager::deleteBook(int id) {
    if (id <= 0) {
        return Status::Invalid;
    }

    Book book;
    Status findStatus = m_catalogue.find(id, book);
    if (findStatus != Status::Ok) {
        return findStatus;
    }

    // Cannot delete a currently borrowed book or one with waiting requests
    if (!book.available || m_requestQueue.contains(id)) {
        return Status::Blocked;
    }

    return m_catalogue.remove(id);
}

Status LibraryManager::searchBook(int id, Book& outBook) const {
    if (id <= 0) {
        return Status::Invalid;
    }
    return m_catalogue.find(id, outBook);
}

Status LibraryManager::editBookMetadata(int id, const std::string& newTitle, const std::string& newAuthor) {
    if (id <= 0) {
        return Status::Invalid;
    }
    return m_catalogue.editMetadata(id, newTitle, newAuthor);
}

Status LibraryManager::borrowBook(int id, BorrowAction& outAction, std::uint64_t* outTxId) {
    outAction = BorrowAction::None;
    if (id <= 0) {
        return Status::Invalid;
    }

    Book book;
    Status findStatus = m_catalogue.find(id, book);
    if (findStatus != Status::Ok) {
        return findStatus;
    }

    if (book.available) {
        // Preflight & allocate history node before mutating catalogue state (failure atomicity)
        HistoryList::PendingNode pending;
        Status prepStatus = m_history.prepareAppend(id, book.title, "BORROW", pending);
        if (prepStatus != Status::Ok) {
            return prepStatus;
        }

        // Non-throwing state commit
        m_catalogue.updateStatus(id, false);
        std::uint64_t txId = m_history.commitAppend(pending);
        if (outTxId != nullptr) {
            *outTxId = txId;
        }
        outAction = BorrowAction::Borrowed;
        return Status::Ok;
    }

    // Book is already borrowed -> enqueue waiting request without changing history or status
    Status queueStatus = m_requestQueue.enqueue(id);
    if (queueStatus != Status::Ok) {
        return queueStatus;
    }
    outAction = BorrowAction::Queued;
    return Status::Ok;
}

Status LibraryManager::returnBook(int id, std::uint64_t* outTxId) {
    if (id <= 0) {
        return Status::Invalid;
    }

    Book book;
    Status findStatus = m_catalogue.find(id, book);
    if (findStatus != Status::Ok) {
        return findStatus;
    }

    // Reject returning an already available book
    if (book.available) {
        return Status::Blocked;
    }

    // Reject if return stack is full (prevents inconsistent partial state)
    if (m_returnStack.isFull()) {
        return Status::Full;
    }

    // Preflight & allocate history node before mutating state
    HistoryList::PendingNode pending;
    Status prepStatus = m_history.prepareAppend(id, book.title, "RETURN", pending);
    if (prepStatus != Status::Ok) {
        return prepStatus;
    }

    // Non-throwing state commit
    m_catalogue.updateStatus(id, true);
    m_returnStack.push(id);
    std::uint64_t txId = m_history.commitAppend(pending);
    if (outTxId != nullptr) {
        *outTxId = txId;
    }
    return Status::Ok;
}

Status LibraryManager::processNextRequest(int& outBookId, std::uint64_t* outTxId) {
    outBookId = 0;

    int headId = 0;
    Status peekStatus = m_requestQueue.peek(headId);
    if (peekStatus != Status::Ok) {
        return peekStatus; // Empty
    }

    Book book;
    Status findStatus = m_catalogue.find(headId, book);
    if (findStatus != Status::Ok || !book.available) {
        // Head book unavailable -> blocked, leave queue unchanged
        outBookId = headId;
        return Status::Blocked;
    }

    // Preflight & allocate history node before mutating state
    HistoryList::PendingNode pending;
    Status prepStatus = m_history.prepareAppend(headId, book.title, "BORROW", pending);
    if (prepStatus != Status::Ok) {
        return prepStatus;
    }

    // Non-throwing state commit
    m_requestQueue.dequeue();
    m_catalogue.updateStatus(headId, false);
    std::uint64_t txId = m_history.commitAppend(pending);
    if (outTxId != nullptr) {
        *outTxId = txId;
    }
    outBookId = headId;
    return Status::Ok;
}

Status LibraryManager::cancelHeadRequest(int& outBookId) {
    outBookId = 0;
    return m_requestQueue.dequeue(&outBookId);
}

Status LibraryManager::popReturnStack(int& outBookId) {
    outBookId = 0;
    return m_returnStack.pop(&outBookId);
}

Status LibraryManager::findHistory(std::uint64_t txId, HistoryRecord& outRecord) const {
    return m_history.find(txId, outRecord);
}

Status LibraryManager::editHistory(std::uint64_t txId, const std::string& newTitle, const std::string& newAction) {
    return m_history.update(txId, newTitle, newAction);
}

Status LibraryManager::deleteHistory(std::uint64_t txId) {
    return m_history.remove(txId);
}

Status LibraryManager::findInReturnStack(int bookId, size_t& outTopOffset) const {
    return m_returnStack.find(bookId, outTopOffset);
}

Status LibraryManager::updateReturnStackSlot(size_t topOffset, int newBookId) {
    if (m_returnStack.isEmpty()) {
        return Status::Empty;
    }
    if (topOffset >= m_returnStack.getCount() || newBookId <= 0) {
        return Status::Invalid;
    }
    if (!m_catalogue.contains(newBookId)) {
        return Status::NotFound;
    }
    return m_returnStack.updateAt(topOffset, newBookId);
}

Status LibraryManager::findInRequestQueue(int bookId, size_t& outFrontOffset) const {
    return m_requestQueue.find(bookId, outFrontOffset);
}

Status LibraryManager::updateRequestQueueSlot(size_t frontOffset, int newBookId) {
    if (m_requestQueue.isEmpty()) {
        return Status::Empty;
    }
    if (frontOffset >= m_requestQueue.getCount() || newBookId <= 0) {
        return Status::Invalid;
    }
    if (!m_catalogue.contains(newBookId)) {
        return Status::NotFound;
    }
    return m_requestQueue.updateAt(frontOffset, newBookId);
}

bool LibraryManager::verifyAllInvariants() const noexcept {
    // 1. Structural invariants
    if (!m_history.verifyInvariants() ||
        !m_returnStack.verifyInvariants() ||
        !m_requestQueue.verifyInvariants()) {
        return false;
    }

    // 2. Catalogue invariants: unique positive IDs in dense prefix
    for (size_t i = 0; i < m_catalogue.getCount(); ++i) {
        const Book* b = m_catalogue.getAt(i);
        if (b == nullptr || b->id <= 0 || isBlankString(b->title) || isBlankString(b->author)) {
            return false;
        }
        for (size_t j = i + 1; j < m_catalogue.getCount(); ++j) {
            if (m_catalogue.getAt(j)->id == b->id) {
                return false;
            }
        }
    }

    // 3. Every pending request in queue must correspond to a valid book in catalogue
    bool queueValid = true;
    m_requestQueue.traverse([&](int reqBookId) {
        if (!m_catalogue.contains(reqBookId)) {
            queueValid = false;
        }
    });
    return queueValid;
}

void LibraryManager::injectNextHistoryAllocationFailure(bool failNext) noexcept {
    m_history.setSimulateAllocationFailure(failNext);
}
