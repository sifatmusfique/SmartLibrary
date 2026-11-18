#include "HistoryList.hpp"
#include <new>

HistoryList::~HistoryList() {
    clear();
}

HistoryList::HistoryList(const HistoryList& other)
    : m_nextTxId(other.m_nextTxId) {
    try {
        const Node* current = other.m_head;
        while (current != nullptr) {
            Node* newNode = new Node(current->data);
            if (m_head == nullptr) {
                m_head = newNode;
                m_tail = newNode;
            } else {
                m_tail->next = newNode;
                m_tail = newNode;
            }
            ++m_count;
            current = current->next;
        }
    } catch (...) {
        clear();
        throw;
    }
}

HistoryList& HistoryList::operator=(const HistoryList& other) {
    if (this != &other) {
        HistoryList temp(other);
        clear();
        m_head = temp.m_head;
        m_tail = temp.m_tail;
        m_count = temp.m_count;
        m_nextTxId = temp.m_nextTxId;

        temp.m_head = nullptr;
        temp.m_tail = nullptr;
        temp.m_count = 0;
    }
    return *this;
}

HistoryList::HistoryList(HistoryList&& other) noexcept
    : m_head(other.m_head),
      m_tail(other.m_tail),
      m_count(other.m_count),
      m_nextTxId(other.m_nextTxId) {
    other.m_head = nullptr;
    other.m_tail = nullptr;
    other.m_count = 0;
}

HistoryList& HistoryList::operator=(HistoryList&& other) noexcept {
    if (this != &other) {
        clear();
        m_head = other.m_head;
        m_tail = other.m_tail;
        m_count = other.m_count;
        m_nextTxId = other.m_nextTxId;

        other.m_head = nullptr;
        other.m_tail = nullptr;
        other.m_count = 0;
    }
    return *this;
}

bool HistoryList::isValidAction(const std::string& action) noexcept {
    return action == "BORROW" || action == "RETURN";
}

Status HistoryList::prepareAppend(int bookId, const std::string& title, const std::string& action, PendingNode& outNode) const {
    if (bookId <= 0 || isBlankString(title) || !isValidAction(action)) {
        return Status::Invalid;
    }

    if (m_simulateAllocFailure) {
        m_simulateAllocFailure = false;
        throw std::bad_alloc();
    }

    HistoryRecord record(0, bookId, trimString(title), action);
    Node* newNode = new Node(record);
    delete outNode.ptr;
    outNode.ptr = newNode;
    return Status::Ok;
}

std::uint64_t HistoryList::commitAppend(PendingNode& pending) noexcept {
    Node* node = pending.ptr;
    if (node == nullptr) {
        return 0;
    }

    pending.ptr = nullptr;
    node->data.transactionId = m_nextTxId++;
    node->next = nullptr;

    if (m_tail == nullptr) {
        m_head = node;
        m_tail = node;
    } else {
        m_tail->next = node;
        m_tail = node;
    }
    ++m_count;
    return node->data.transactionId;
}

Status HistoryList::append(int bookId, const std::string& title, const std::string& action, std::uint64_t* outTxId) {
    PendingNode pending;
    Status prepStatus = prepareAppend(bookId, title, action, pending);
    if (prepStatus != Status::Ok) {
        return prepStatus;
    }
    std::uint64_t txId = commitAppend(pending);
    if (outTxId != nullptr) {
        *outTxId = txId;
    }
    return Status::Ok;
}

Status HistoryList::find(std::uint64_t txId, HistoryRecord& outRecord) const {
    if (txId == 0) {
        return Status::Invalid;
    }
    if (m_head == nullptr) {
        return Status::Empty;
    }

    const Node* current = m_head;
    while (current != nullptr) {
        if (current->data.transactionId == txId) {
            outRecord = current->data;
            return Status::Ok;
        }
        current = current->next;
    }
    return Status::NotFound;
}

Status HistoryList::update(std::uint64_t txId, const std::string& newTitle, const std::string& newAction) {
    if (txId == 0 || isBlankString(newTitle) || !isValidAction(newAction)) {
        return Status::Invalid;
    }
    if (m_head == nullptr) {
        return Status::Empty;
    }

    Node* current = m_head;
    while (current != nullptr) {
        if (current->data.transactionId == txId) {
            current->data.title = trimString(newTitle);
            current->data.action = newAction;
            return Status::Ok;
        }
        current = current->next;
    }
    return Status::NotFound;
}

Status HistoryList::remove(std::uint64_t txId) {
    if (txId == 0) {
        return Status::Invalid;
    }
    if (m_head == nullptr) {
        return Status::Empty;
    }

    // Deleting the head node (also covers single-node list)
    if (m_head->data.transactionId == txId) {
        Node* target = m_head;
        m_head = m_head->next;
        if (m_head == nullptr) {
            m_tail = nullptr;
        }
        delete target;
        --m_count;
        return Status::Ok;
    }

    // Deleting a middle or tail node
    Node* prev = m_head;
    while (prev->next != nullptr) {
        if (prev->next->data.transactionId == txId) {
            Node* target = prev->next;
            prev->next = target->next;
            if (target == m_tail) {
                m_tail = prev;
            }
            delete target;
            --m_count;
            return Status::Ok;
        }
        prev = prev->next;
    }

    return Status::NotFound;
}

void HistoryList::clear() noexcept {
    Node* current = m_head;
    while (current != nullptr) {
        Node* nextNode = current->next;
        delete current;
        current = nextNode;
    }
    m_head = nullptr;
    m_tail = nullptr;
    m_count = 0;
}

bool HistoryList::verifyInvariants() const noexcept {
    if (m_count == 0) {
        return m_head == nullptr && m_tail == nullptr;
    }
    if (m_head == nullptr || m_tail == nullptr || m_tail->next != nullptr) {
        return false;
    }

    size_t walked = 0;
    const Node* current = m_head;
    const Node* last = nullptr;
    while (current != nullptr) {
        ++walked;
        if (walked > m_count) {
            return false; // Cycle protection
        }
        last = current;
        current = current->next;
    }
    return walked == m_count && last == m_tail;
}
