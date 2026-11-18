#include "RequestQueue.hpp"
#include <stdexcept>

RequestQueue::RequestQueue(size_t capacity)
    : m_capacity(capacity), m_front(0), m_count(0) {
    if (m_capacity == 0) {
        throw std::invalid_argument("RequestQueue capacity must be positive (got 0)");
    }
    m_items = new int[m_capacity]{};
}

RequestQueue::~RequestQueue() {
    delete[] m_items;
    m_items = nullptr;
    m_capacity = 0;
    m_front = 0;
    m_count = 0;
}

RequestQueue::RequestQueue(const RequestQueue& other)
    : m_capacity(other.m_capacity), m_front(0), m_count(other.m_count) {
    m_items = new int[m_capacity]{};
    // Normalize starting at index 0 during deep copy while preserving FIFO order
    for (size_t offset = 0; offset < m_count; ++offset) {
        size_t srcIdx = (other.m_front + offset) % other.m_capacity;
        m_items[offset] = other.m_items[srcIdx];
    }
}

RequestQueue& RequestQueue::operator=(const RequestQueue& other) {
    if (this != &other) {
        int* newItems = new int[other.m_capacity]{};
        for (size_t offset = 0; offset < other.m_count; ++offset) {
            size_t srcIdx = (other.m_front + offset) % other.m_capacity;
            newItems[offset] = other.m_items[srcIdx];
        }
        delete[] m_items;
        m_items = newItems;
        m_capacity = other.m_capacity;
        m_front = 0;
        m_count = other.m_count;
    }
    return *this;
}

RequestQueue::RequestQueue(RequestQueue&& other) noexcept
    : m_items(other.m_items),
      m_capacity(other.m_capacity),
      m_front(other.m_front),
      m_count(other.m_count) {
    other.m_items = nullptr;
    other.m_capacity = 0;
    other.m_front = 0;
    other.m_count = 0;
}

RequestQueue& RequestQueue::operator=(RequestQueue&& other) noexcept {
    if (this != &other) {
        delete[] m_items;
        m_items = other.m_items;
        m_capacity = other.m_capacity;
        m_front = other.m_front;
        m_count = other.m_count;

        other.m_items = nullptr;
        other.m_capacity = 0;
        other.m_front = 0;
        other.m_count = 0;
    }
    return *this;
}

Status RequestQueue::enqueue(int bookId) noexcept {
    if (bookId <= 0) {
        return Status::Invalid;
    }
    if (m_count >= m_capacity) {
        return Status::Full;
    }
    size_t rearIdx = (m_front + m_count) % m_capacity;
    m_items[rearIdx] = bookId;
    ++m_count;
    return Status::Ok;
}

Status RequestQueue::dequeue(int* outBookId) noexcept {
    if (m_count == 0) {
        return Status::Empty;
    }
    if (outBookId != nullptr) {
        *outBookId = m_items[m_front];
    }
    m_items[m_front] = 0;
    m_front = (m_front + 1) % m_capacity;
    --m_count;
    return Status::Ok;
}

Status RequestQueue::peek(int& outBookId) const noexcept {
    if (m_count == 0) {
        return Status::Empty;
    }
    outBookId = m_items[m_front];
    return Status::Ok;
}

bool RequestQueue::contains(int bookId) const noexcept {
    if (bookId <= 0 || m_count == 0) {
        return false;
    }
    for (size_t offset = 0; offset < m_count; ++offset) {
        size_t physicalIdx = (m_front + offset) % m_capacity;
        if (m_items[physicalIdx] == bookId) {
            return true;
        }
    }
    return false;
}

Status RequestQueue::find(int bookId, size_t& outFrontOffset) const noexcept {
    if (bookId <= 0) {
        return Status::Invalid;
    }
    if (m_count == 0) {
        return Status::Empty;
    }
    for (size_t offset = 0; offset < m_count; ++offset) {
        size_t physicalIdx = (m_front + offset) % m_capacity;
        if (m_items[physicalIdx] == bookId) {
            outFrontOffset = offset;
            return Status::Ok;
        }
    }
    return Status::NotFound;
}

Status RequestQueue::updateAt(size_t frontOffset, int newBookId) noexcept {
    if (newBookId <= 0) {
        return Status::Invalid;
    }
    if (m_count == 0) {
        return Status::Empty;
    }
    if (frontOffset >= m_count) {
        return Status::Invalid;
    }
    size_t physicalIdx = (m_front + frontOffset) % m_capacity;
    m_items[physicalIdx] = newBookId;
    return Status::Ok;
}

const int* RequestQueue::getAtFrontOffset(size_t frontOffset) const noexcept {
    if (frontOffset >= m_count) {
        return nullptr;
    }
    size_t physicalIdx = (m_front + frontOffset) % m_capacity;
    return &m_items[physicalIdx];
}

void RequestQueue::clear() noexcept {
    for (size_t offset = 0; offset < m_count; ++offset) {
        size_t physicalIdx = (m_front + offset) % m_capacity;
        m_items[physicalIdx] = 0;
    }
    m_front = 0;
    m_count = 0;
}
