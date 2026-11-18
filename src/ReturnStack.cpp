#include "ReturnStack.hpp"
#include <stdexcept>

ReturnStack::ReturnStack(size_t capacity)
    : m_capacity(capacity), m_count(0) {
    if (m_capacity == 0) {
        throw std::invalid_argument("ReturnStack capacity must be positive (got 0)");
    }
    m_items = new int[m_capacity]{};
}

ReturnStack::~ReturnStack() {
    delete[] m_items;
    m_items = nullptr;
    m_capacity = 0;
    m_count = 0;
}

ReturnStack::ReturnStack(const ReturnStack& other)
    : m_capacity(other.m_capacity), m_count(other.m_count) {
    m_items = new int[m_capacity]{};
    for (size_t i = 0; i < m_count; ++i) {
        m_items[i] = other.m_items[i];
    }
}

ReturnStack& ReturnStack::operator=(const ReturnStack& other) {
    if (this != &other) {
        int* newItems = new int[other.m_capacity]{};
        for (size_t i = 0; i < other.m_count; ++i) {
            newItems[i] = other.m_items[i];
        }
        delete[] m_items;
        m_items = newItems;
        m_capacity = other.m_capacity;
        m_count = other.m_count;
    }
    return *this;
}

ReturnStack::ReturnStack(ReturnStack&& other) noexcept
    : m_items(other.m_items), m_capacity(other.m_capacity), m_count(other.m_count) {
    other.m_items = nullptr;
    other.m_capacity = 0;
    other.m_count = 0;
}

ReturnStack& ReturnStack::operator=(ReturnStack&& other) noexcept {
    if (this != &other) {
        delete[] m_items;
        m_items = other.m_items;
        m_capacity = other.m_capacity;
        m_count = other.m_count;

        other.m_items = nullptr;
        other.m_capacity = 0;
        other.m_count = 0;
    }
    return *this;
}

Status ReturnStack::push(int bookId) noexcept {
    if (bookId <= 0) {
        return Status::Invalid;
    }
    if (m_count >= m_capacity) {
        return Status::Full;
    }
    m_items[m_count] = bookId;
    ++m_count;
    return Status::Ok;
}

Status ReturnStack::pop(int* outBookId) noexcept {
    if (m_count == 0) {
        return Status::Empty;
    }
    --m_count;
    if (outBookId != nullptr) {
        *outBookId = m_items[m_count];
    }
    m_items[m_count] = 0;
    return Status::Ok;
}

Status ReturnStack::peek(int& outBookId) const noexcept {
    if (m_count == 0) {
        return Status::Empty;
    }
    outBookId = m_items[m_count - 1];
    return Status::Ok;
}

Status ReturnStack::find(int bookId, size_t& outTopOffset) const noexcept {
    if (bookId <= 0) {
        return Status::Invalid;
    }
    if (m_count == 0) {
        return Status::Empty;
    }
    for (size_t offset = 0; offset < m_count; ++offset) {
        if (m_items[m_count - 1 - offset] == bookId) {
            outTopOffset = offset;
            return Status::Ok;
        }
    }
    return Status::NotFound;
}

Status ReturnStack::updateAt(size_t topOffset, int newBookId) noexcept {
    if (newBookId <= 0) {
        return Status::Invalid;
    }
    if (m_count == 0) {
        return Status::Empty;
    }
    if (topOffset >= m_count) {
        return Status::Invalid;
    }
    m_items[m_count - 1 - topOffset] = newBookId;
    return Status::Ok;
}

const int* ReturnStack::getAtTopOffset(size_t topOffset) const noexcept {
    if (topOffset >= m_count) {
        return nullptr;
    }
    return &m_items[m_count - 1 - topOffset];
}

void ReturnStack::clear() noexcept {
    for (size_t i = 0; i < m_count; ++i) {
        m_items[i] = 0;
    }
    m_count = 0;
}
