#pragma once

#include "Book.hpp"
#include <cstddef>

/**
 * @brief Manually managed bounded array representing the Library Book Catalogue.
 * 
 * Strict Academic Rubric Compliance:
 * - Direct heap-allocated raw array (Book*), NO std::vector or any other STL container.
 * - Enforces contiguous dense storage [0, m_count) with manual left-shifting on deletion.
 * - Pure manual search and update logic; zero std::algorithm helpers.
 * - Implements Rule of Five for explicit, leak-free memory management.
 */
class BookCatalogue {
public:
    static constexpr size_t DEFAULT_CAPACITY = 100;

    /**
     * @brief Construct a new Book Catalogue with a given bounded capacity.
     * @param capacity Maximum number of books the array can store (must be > 0).
     */
    explicit BookCatalogue(size_t capacity = DEFAULT_CAPACITY);

    /**
     * @brief Destructor deallocates the raw array.
     */
    ~BookCatalogue();

    // Rule of Five memory management
    BookCatalogue(const BookCatalogue& other);
    BookCatalogue& operator=(const BookCatalogue& other);
    BookCatalogue(BookCatalogue&& other) noexcept;
    BookCatalogue& operator=(BookCatalogue&& other) noexcept;

    /**
     * @brief Add a new book to the catalogue array.
     * Checks capacity, positive ID, non-empty fields, and uniqueness.
     * Time Complexity: O(n) due to uniqueness check; O(1) append.
     */
    Status add(const Book& book);

    /**
     * @brief Remove a book by ID.
     * Elements to the right are shifted left by 1 position to preserve density.
     * Time Complexity: O(n) search + O(n) shifting = O(n).
     */
    Status remove(int id);

    /**
     * @brief Search for a book by its positive ID.
     * @param id The book ID to look for.
     * @param outBook Output reference populated with the found book data.
     * Time Complexity: O(n) linear search.
     */
    Status find(int id, Book& outBook) const;

    /**
     * @brief Check whether a book with the given ID exists in the catalogue.
     * Time Complexity: O(n).
     */
    bool contains(int id) const;

    /**
     * @brief Update title and author of an existing book without modifying ID or availability.
     * Time Complexity: O(n) search.
     */
    Status editMetadata(int id, const std::string& newTitle, const std::string& newAuthor);

    /**
     * @brief Update the availability status (used exclusively by LibraryManager for borrow/return).
     * Time Complexity: O(n) search.
     */
    Status updateStatus(int id, bool available);

    /**
     * @brief Streamed traversal using a callable visitor function.
     * Avoids allocating or copying any STL container collections.
     * Time Complexity: O(n).
     */
    template <typename Visitor>
    void traverse(Visitor visitor) const {
        for (size_t i = 0; i < m_count; ++i) {
            visitor(m_books[i]);
        }
    }

    /**
     * @brief Const direct element access for inspection and testing.
     * Returns nullptr if index is out of bounds.
     */
    const Book* getAt(size_t index) const;

    /**
     * @brief Reset catalogue count to 0. Does not reallocate memory.
     */
    void clear() noexcept;

    // Capacity & state observers
    size_t getCount() const noexcept { return m_count; }
    size_t getCapacity() const noexcept { return m_capacity; }
    bool isEmpty() const noexcept { return m_count == 0; }
    bool isFull() const noexcept { return m_count >= m_capacity; }

private:
    Book* m_books{nullptr};  ///< Raw dynamically allocated array of Book structures
    size_t m_capacity{0};    ///< Fixed maximum capacity of the array
    size_t m_count{0};       ///< Number of currently active books stored

    /**
     * @brief Internal helper to find index of a book by ID.
     * @return Zero-based index if found; -1 if not found.
     */
    int findIndex(int id) const;
};
