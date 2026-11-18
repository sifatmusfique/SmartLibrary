#pragma once

#include <cctype>
#include <cstdint>
#include <string>
#include <utility>

/**
 * @brief Standard typed result status codes across all data structure operations.
 * Keeps data structures decoupled from console I/O.
 */
enum class Status {
    Ok,
    Empty,
    Full,
    NotFound,
    Duplicate,
    Invalid,
    Blocked
};

/**
 * @brief Convert status code to readable string representation for tests and UI.
 */
inline const char* statusToString(Status status) noexcept {
    switch (status) {
        case Status::Ok:        return "Ok";
        case Status::Empty:     return "Empty";
        case Status::Full:      return "Full";
        case Status::NotFound:  return "NotFound";
        case Status::Duplicate: return "Duplicate";
        case Status::Invalid:   return "Invalid";
        case Status::Blocked:   return "Blocked";
    }
    return "Unknown";
}

/**
 * @brief Return a copy of text with leading and trailing whitespace removed.
 */
inline std::string trimString(const std::string& text) {
    size_t start = 0;
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) {
        ++start;
    }
    size_t end = text.size();
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }
    return text.substr(start, end - start);
}

/**
 * @brief Check whether a string is empty or contains only whitespace characters.
 */
inline bool isBlankString(const std::string& text) noexcept {
    for (unsigned char ch : text) {
        if (!std::isspace(ch)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Core entity representing a book item in the Smart Library.
 */
struct Book {
    int id{0};
    std::string title;
    std::string author;
    bool available{true};

    Book() = default;

    Book(int id, std::string title, std::string author, bool available = true)
        : id(id), title(std::move(title)), author(std::move(author)), available(available) {}

    bool operator==(const Book& other) const noexcept {
        return id == other.id &&
               available == other.available &&
               title == other.title &&
               author == other.author;
    }

    bool operator!=(const Book& other) const noexcept {
        return !(*this == other);
    }
};

/**
 * @brief Historical snapshot record of a borrow or return transaction.
 */
struct HistoryRecord {
    std::uint64_t transactionId{0};
    int bookId{0};
    std::string title;
    std::string action; // "BORROW" or "RETURN"

    HistoryRecord() = default;

    HistoryRecord(std::uint64_t txId, int bookId, std::string title, std::string action)
        : transactionId(txId), bookId(bookId), title(std::move(title)), action(std::move(action)) {}

    bool operator==(const HistoryRecord& other) const noexcept {
        return transactionId == other.transactionId &&
               bookId == other.bookId &&
               title == other.title &&
               action == other.action;
    }
};
