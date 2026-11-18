#include "BookCatalogue.hpp"
#include <stdexcept>
#include <utility>

BookCatalogue::BookCatalogue(size_t capacity)
    : m_capacity(capacity), m_count(0) {
    if (m_capacity == 0) {
        throw std::invalid_argument("BookCatalogue capacity must be positive (got 0)");
    }
    m_books = new Book[m_capacity];
}

BookCatalogue::~BookCatalogue() {
    delete[] m_books;
    m_books = nullptr;
    m_count = 0;
    m_capacity = 0;
}

BookCatalogue::BookCatalogue(const BookCatalogue& other)
    : m_capacity(other.m_capacity), m_count(other.m_count) {
    m_books = new Book[m_capacity];
    for (size_t i = 0; i < m_count; ++i) {
        m_books[i] = other.m_books[i];
    }
}

BookCatalogue& BookCatalogue::operator=(const BookCatalogue& other) {
    if (this != &other) {
        Book* newBooks = new Book[other.m_capacity];
        for (size_t i = 0; i < other.m_count; ++i) {
            newBooks[i] = other.m_books[i];
        }
        delete[] m_books;
        m_books = newBooks;
        m_capacity = other.m_capacity;
        m_count = other.m_count;
    }
    return *this;
}

BookCatalogue::BookCatalogue(BookCatalogue&& other) noexcept
    : m_books(other.m_books), m_capacity(other.m_capacity), m_count(other.m_count) {
    other.m_books = nullptr;
    other.m_capacity = 0;
    other.m_count = 0;
}

BookCatalogue& BookCatalogue::operator=(BookCatalogue&& other) noexcept {
    if (this != &other) {
        delete[] m_books;
        m_books = other.m_books;
        m_capacity = other.m_capacity;
        m_count = other.m_count;

        other.m_books = nullptr;
        other.m_capacity = 0;
        other.m_count = 0;
    }
    return *this;
}

int BookCatalogue::findIndex(int id) const {
    if (id <= 0) {
        return -1;
    }
    for (size_t i = 0; i < m_count; ++i) {
        if (m_books[i].id == id) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

Status BookCatalogue::add(const Book& book) {
    // 1. Validate ID: must be strictly positive
    if (book.id <= 0) {
        return Status::Invalid;
    }

    // 2. Validate fields: title and author must not be empty or whitespace-only
    if (isBlankString(book.title) || isBlankString(book.author)) {
        return Status::Invalid;
    }

    // 3. Validate capacity bound
    if (m_count >= m_capacity) {
        return Status::Full;
    }

    // 4. Validate uniqueness: no duplicate active IDs
    if (findIndex(book.id) != -1) {
        return Status::Duplicate;
    }

    // 5. Append normalized copy at end of occupied prefix
    m_books[m_count] = Book(book.id, trimString(book.title), trimString(book.author), book.available);
    ++m_count;
    return Status::Ok;
}

Status BookCatalogue::remove(int id) {
    int idx = findIndex(id);
    if (idx == -1) {
        return Status::NotFound;
    }

    // Shift all subsequent elements left by 1 to maintain dense array invariant [0, m_count)
    size_t target = static_cast<size_t>(idx);
    for (size_t i = target; i + 1 < m_count; ++i) {
        m_books[i] = std::move(m_books[i + 1]);
    }

    // Reset now-empty tail slot
    m_books[m_count - 1] = Book{};
    --m_count;
    return Status::Ok;
}

Status BookCatalogue::find(int id, Book& outBook) const {
    int idx = findIndex(id);
    if (idx == -1) {
        return Status::NotFound;
    }
    outBook = m_books[idx];
    return Status::Ok;
}

bool BookCatalogue::contains(int id) const {
    return findIndex(id) != -1;
}

Status BookCatalogue::editMetadata(int id, const std::string& newTitle, const std::string& newAuthor) {
    if (isBlankString(newTitle) || isBlankString(newAuthor)) {
        return Status::Invalid;
    }

    int idx = findIndex(id);
    if (idx == -1) {
        return Status::NotFound;
    }

    // Modify only metadata; ID and availability invariant remain unchanged
    m_books[idx].title = trimString(newTitle);
    m_books[idx].author = trimString(newAuthor);
    return Status::Ok;
}

Status BookCatalogue::updateStatus(int id, bool available) {
    int idx = findIndex(id);
    if (idx == -1) {
        return Status::NotFound;
    }

    m_books[idx].available = available;
    return Status::Ok;
}

const Book* BookCatalogue::getAt(size_t index) const {
    if (index >= m_count) {
        return nullptr;
    }
    return &m_books[index];
}

void BookCatalogue::clear() noexcept {
    for (size_t i = 0; i < m_count; ++i) {
        m_books[i] = Book{};
    }
    m_count = 0;
}
