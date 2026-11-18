#include "ConsoleUI.hpp"
#include <cctype>
#include <cstdlib>
#include <errno.h>
#include <limits>

namespace {
std::string trimCopy(const std::string& input) {
    size_t start = 0;
    while (start < input.size() && std::isspace(static_cast<unsigned char>(input[start]))) {
        ++start;
    }
    size_t end = input.size();
    while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
        --end;
    }
    return input.substr(start, end - start);
}
} // namespace

ConsoleUI::ConsoleUI(LibraryManager& manager, std::istream& in, std::ostream& out)
    : m_manager(manager), m_in(in), m_out(out) {}

void ConsoleUI::run() {
    m_out << "====================================================\n";
    m_out << "   Smart Library Management System (CSE 2103)       \n";
    m_out << "====================================================\n";

    while (true) {
        printMainMenu();
        int choice = -1;
        if (!readInt("Enter choice: ", choice)) {
            if (m_in.eof()) {
                m_out << "\nEOF reached. Exiting cleanly.\n";
                break;
            }
            m_out << "Invalid menu input. Please enter a valid integer.\n";
            continue;
        }

        switch (choice) {
            case 1:  handleAddBook(); break;
            case 2:  handleDeleteBook(); break;
            case 3:  handleSearchBook(); break;
            case 4:  handleDisplayBooks(); break;
            case 5:  handleBorrowBook(); break;
            case 6:  handleReturnBook(); break;
            case 7:  handleShowHistory(); break;
            case 8:  handleProcessRequest(); break;
            case 9:  handleShowStack(); break;
            case 10: handlePopStack(); break;
            case 11: handleShowQueue(); break;
            case 12: handleCancelHeadRequest(); break;
            case 13: handleEditBookMetadata(); break;
            case 14: handleMaintenanceMenu(); break;
            case 15: printHelp(); break;
            case 0:
                m_out << "Exiting Smart Library. Goodbye!\n";
                return;
            default:
                m_out << "Invalid menu option. Choose 0 through 15.\n";
                break;
        }

        if (m_in.eof()) {
            m_out << "\nEOF reached. Exiting cleanly.\n";
            break;
        }
    }
}

void ConsoleUI::printMainMenu() const {
    m_out << "\n--- Main Menu ---\n"
          << " 1. Add Book\n"
          << " 2. Delete Book\n"
          << " 3. Search Book\n"
          << " 4. Display All Books\n"
          << " 5. Borrow Book\n"
          << " 6. Return Book\n"
          << " 7. Show Borrow/Return History\n"
          << " 8. Process Waiting Request (FIFO Head)\n"
          << " 9. Show Recent Returns Stack (LIFO)\n"
          << "10. Pop Recent Return Stack\n"
          << "11. Show Waiting Request Queue (FIFO)\n"
          << "12. Cancel Head Request\n"
          << "13. Edit Book Title/Author\n"
          << "14. Educational Maintenance Submenu\n"
          << "15. Help / Operation Guide\n"
          << " 0. Exit\n";
}

void ConsoleUI::printMaintenanceMenu() const {
    m_out << "\n--- Educational Maintenance Submenu ---\n"
          << "(Note: Editing logs/slots is for demonstration only and does not alter circulation status.)\n"
          << " 1. Search History by Transaction ID\n"
          << " 2. Edit History Entry (Title / Action)\n"
          << " 3. Delete History Entry by Transaction ID\n"
          << " 4. Search Return Stack by Book ID (Top Offset)\n"
          << " 5. Update Return Stack Slot by Top Offset\n"
          << " 6. Search Waiting Queue by Book ID (Front Offset)\n"
          << " 7. Update Waiting Queue Slot by Front Offset\n"
          << " 0. Back to Main Menu\n";
}

void ConsoleUI::printHelp() const {
    m_out << "\n--- Smart Library Operation Guide ---\n"
          << "- Catalogue (Bounded Array): Stores unique positive-ID books; deleting shifts left.\n"
          << "- History (Singly Linked List): Appends chronological BORROW/RETURN snapshots.\n"
          << "- Recent Returns (Bounded Stack): LIFO tracking of returned book IDs.\n"
          << "- Waiting Requests (Circular Queue): FIFO queue when borrowing an unavailable book.\n"
          << "- Consistency Rules: Cannot delete a borrowed book or one with pending requests.\n"
          << "  Returning requires space on the Return Stack. Processing only serves the FIFO head.\n";
}

bool ConsoleUI::readLine(const std::string& prompt, std::string& outLine) {
    m_out << prompt;
    if (!std::getline(m_in, outLine)) {
        return false;
    }
    return true;
}

bool ConsoleUI::parseStrictInt(const std::string& raw, int& outValue) noexcept {
    std::string trimmed = trimCopy(raw);
    if (trimmed.empty()) {
        return false;
    }

    char* endPtr = nullptr;
    errno = 0;
    long parsed = std::strtol(trimmed.c_str(), &endPtr, 10);
    if (errno == ERANGE || endPtr == trimmed.c_str() || *endPtr != '\0') {
        return false;
    }
    if (parsed < std::numeric_limits<int>::min() || parsed > std::numeric_limits<int>::max()) {
        return false;
    }
    outValue = static_cast<int>(parsed);
    return true;
}

bool ConsoleUI::parseStrictUint64(const std::string& raw, std::uint64_t& outValue) noexcept {
    std::string trimmed = trimCopy(raw);
    if (trimmed.empty() || trimmed[0] == '-') {
        return false;
    }

    char* endPtr = nullptr;
    errno = 0;
    unsigned long long parsed = std::strtoull(trimmed.c_str(), &endPtr, 10);
    if (errno == ERANGE || endPtr == trimmed.c_str() || *endPtr != '\0') {
        return false;
    }
    outValue = static_cast<std::uint64_t>(parsed);
    return true;
}

bool ConsoleUI::readInt(const std::string& prompt, int& outValue) {
    std::string line;
    if (!readLine(prompt, line)) {
        return false;
    }
    return parseStrictInt(line, outValue);
}

bool ConsoleUI::readUint64(const std::string& prompt, std::uint64_t& outValue) {
    std::string line;
    if (!readLine(prompt, line)) {
        return false;
    }
    return parseStrictUint64(line, outValue);
}

void ConsoleUI::handleAddBook() {
    int id = 0;
    if (!readInt("Enter Book ID (positive integer): ", id)) {
        m_out << "Invalid book ID.\n";
        return;
    }

    std::string title;
    if (!readLine("Enter Title: ", title)) {
        return;
    }
    title = trimCopy(title);

    std::string author;
    if (!readLine("Enter Author: ", author)) {
        return;
    }
    author = trimCopy(author);

    Status st = m_manager.addBook(id, title, author);
    switch (st) {
        case Status::Ok:
            m_out << "Book added successfully.\n";
            break;
        case Status::Duplicate:
            m_out << "Duplicate Book ID. Book not added.\n";
            break;
        case Status::Full:
            m_out << "Catalogue is full. Cannot add more books.\n";
            break;
        case Status::Invalid:
            m_out << "Invalid book details (ID must be > 0 and title/author non-empty).\n";
            break;
        default:
            m_out << "Failed to add book (" << statusToString(st) << ").\n";
            break;
    }
}

void ConsoleUI::handleDeleteBook() {
    int id = 0;
    if (!readInt("Enter Book ID to delete: ", id)) {
        m_out << "Invalid book ID.\n";
        return;
    }

    Status st = m_manager.deleteBook(id);
    switch (st) {
        case Status::Ok:
            m_out << "Book deleted successfully.\n";
            break;
        case Status::NotFound:
            m_out << "Book not found.\n";
            break;
        case Status::Blocked:
            m_out << "Cannot delete: book is currently borrowed or has pending requests.\n";
            break;
        case Status::Invalid:
            m_out << "Invalid book ID.\n";
            break;
        default:
            m_out << "Delete failed (" << statusToString(st) << ").\n";
            break;
    }
}

void ConsoleUI::handleSearchBook() {
    int id = 0;
    if (!readInt("Enter Book ID to search: ", id)) {
        m_out << "Invalid book ID.\n";
        return;
    }

    Book book;
    Status st = m_manager.searchBook(id, book);
    if (st == Status::Ok) {
        m_out << "ID: " << book.id << "\n"
              << "Title: " << book.title << "\n"
              << "Author: " << book.author << "\n"
              << "Status: " << (book.available ? "Available" : "Borrowed") << "\n";
    } else if (st == Status::NotFound) {
        m_out << "Book not found.\n";
    } else {
        m_out << "Invalid book ID.\n";
    }
}

void ConsoleUI::handleDisplayBooks() const {
    const BookCatalogue& cat = m_manager.getCatalogue();
    if (cat.isEmpty()) {
        m_out << "Catalogue is empty.\n";
        return;
    }

    m_out << "\n--- Book Catalogue (" << cat.getCount() << "/" << cat.getCapacity() << ") ---\n";
    cat.traverse([&](const Book& b) {
        m_out << "ID: " << b.id
              << " | Title: " << b.title
              << " | Author: " << b.author
              << " | Status: " << (b.available ? "Available" : "Borrowed") << "\n";
    });
}

void ConsoleUI::handleBorrowBook() {
    int id = 0;
    if (!readInt("Enter Book ID to borrow: ", id)) {
        m_out << "Invalid book ID.\n";
        return;
    }

    BorrowAction action = BorrowAction::None;
    std::uint64_t txId = 0;
    Status st = m_manager.borrowBook(id, action, &txId);
    if (st == Status::Ok) {
        if (action == BorrowAction::Borrowed) {
            m_out << "Book borrowed successfully. (TxID: " << txId << ")\n";
        } else if (action == BorrowAction::Queued) {
            m_out << "Book is already borrowed. Request added to queue.\n";
        }
    } else if (st == Status::NotFound) {
        m_out << "Book not found.\n";
    } else if (st == Status::Full) {
        m_out << "Queue Overflow! Waiting request queue is full.\n";
    } else {
        m_out << "Borrow failed (" << statusToString(st) << ").\n";
    }
}

void ConsoleUI::handleReturnBook() {
    int id = 0;
    if (!readInt("Enter Book ID to return: ", id)) {
        m_out << "Invalid book ID.\n";
        return;
    }

    std::uint64_t txId = 0;
    Status st = m_manager.returnBook(id, &txId);
    switch (st) {
        case Status::Ok:
            m_out << "Book returned successfully. (TxID: " << txId << ")\n";
            break;
        case Status::NotFound:
            m_out << "Book not found.\n";
            break;
        case Status::Blocked:
            m_out << "Book is already available; cannot return.\n";
            break;
        case Status::Full:
            m_out << "Stack Overflow! Return stack is full; pop a recent return first.\n";
            break;
        default:
            m_out << "Return failed (" << statusToString(st) << ").\n";
            break;
    }
}

void ConsoleUI::handleShowHistory() const {
    const HistoryList& hist = m_manager.getHistory();
    if (hist.isEmpty()) {
        m_out << "History is empty.\n";
        return;
    }

    m_out << "\n--- Borrow/Return History (" << hist.getCount() << " records) ---\n";
    hist.traverse([&](const HistoryRecord& rec) {
        m_out << "[TxID: " << rec.transactionId << "] "
              << "Book ID: " << rec.bookId
              << " | Title: " << rec.title
              << " | Action: " << rec.action << "\n";
    });
}

void ConsoleUI::handleProcessRequest() {
    int bookId = 0;
    std::uint64_t txId = 0;
    Status st = m_manager.processNextRequest(bookId, &txId);
    switch (st) {
        case Status::Ok:
            m_out << "Processed waiting request for Book ID " << bookId
                  << ". Book borrowed successfully. (TxID: " << txId << ")\n";
            break;
        case Status::Empty:
            m_out << "Queue Underflow! No pending requests.\n";
            break;
        case Status::Blocked:
            m_out << "Head request (Book ID " << bookId
                  << ") is currently unavailable. Queue left unchanged.\n";
            break;
        default:
            m_out << "Process request failed (" << statusToString(st) << ").\n";
            break;
    }
}

void ConsoleUI::handleShowStack() const {
    const ReturnStack& stack = m_manager.getReturnStack();
    if (stack.isEmpty()) {
        m_out << "Return stack is empty.\n";
        return;
    }

    m_out << "Return stack (top to bottom):";
    stack.traverse([&](int id) {
        m_out << " " << id;
    });
    m_out << "\n";
}

void ConsoleUI::handlePopStack() {
    int poppedId = 0;
    Status st = m_manager.popReturnStack(poppedId);
    if (st == Status::Ok) {
        m_out << "Popped Book ID " << poppedId << " from return stack.\n";
    } else if (st == Status::Empty) {
        m_out << "Stack Underflow!\n";
    } else {
        m_out << "Pop failed (" << statusToString(st) << ").\n";
    }
}

void ConsoleUI::handleShowQueue() const {
    const RequestQueue& queue = m_manager.getRequestQueue();
    if (queue.isEmpty()) {
        m_out << "Waiting request queue is empty.\n";
        return;
    }

    m_out << "Waiting request queue (front to rear):";
    queue.traverse([&](int id) {
        m_out << " " << id;
    });
    m_out << "\n";
}

void ConsoleUI::handleCancelHeadRequest() {
    int cancelledId = 0;
    Status st = m_manager.cancelHeadRequest(cancelledId);
    if (st == Status::Ok) {
        m_out << "Cancelled head waiting request for Book ID " << cancelledId << ".\n";
    } else if (st == Status::Empty) {
        m_out << "Queue Underflow! No pending requests to cancel.\n";
    } else {
        m_out << "Cancel failed (" << statusToString(st) << ").\n";
    }
}

void ConsoleUI::handleEditBookMetadata() {
    int id = 0;
    if (!readInt("Enter Book ID to edit: ", id)) {
        m_out << "Invalid book ID.\n";
        return;
    }

    std::string newTitle;
    if (!readLine("Enter New Title: ", newTitle)) {
        return;
    }
    newTitle = trimCopy(newTitle);

    std::string newAuthor;
    if (!readLine("Enter New Author: ", newAuthor)) {
        return;
    }
    newAuthor = trimCopy(newAuthor);

    Status st = m_manager.editBookMetadata(id, newTitle, newAuthor);
    if (st == Status::Ok) {
        m_out << "Book metadata updated successfully.\n";
    } else if (st == Status::NotFound) {
        m_out << "Book not found.\n";
    } else {
        m_out << "Invalid metadata (title and author cannot be blank).\n";
    }
}

void ConsoleUI::handleMaintenanceMenu() {
    while (true) {
        printMaintenanceMenu();
        int subChoice = -1;
        if (!readInt("Enter maintenance choice: ", subChoice)) {
            if (m_in.eof()) {
                return;
            }
            m_out << "Invalid submenu input.\n";
            continue;
        }

        if (subChoice == 0) {
            return;
        }

        switch (subChoice) {
            case 1: {
                std::uint64_t txId = 0;
                if (!readUint64("Enter Transaction ID: ", txId)) {
                    m_out << "Invalid Transaction ID.\n";
                    break;
                }
                HistoryRecord rec;
                Status st = m_manager.findHistory(txId, rec);
                if (st == Status::Ok) {
                    m_out << "Found [TxID: " << rec.transactionId << "] Book ID: " << rec.bookId
                          << " | Title: " << rec.title << " | Action: " << rec.action << "\n";
                } else if (st == Status::Empty) {
                    m_out << "History is empty.\n";
                } else {
                    m_out << "History record not found.\n";
                }
                break;
            }
            case 2: {
                std::uint64_t txId = 0;
                if (!readUint64("Enter Transaction ID to edit: ", txId)) {
                    m_out << "Invalid Transaction ID.\n";
                    break;
                }
                std::string newTitle;
                if (!readLine("Enter Corrected Title: ", newTitle)) break;
                newTitle = trimCopy(newTitle);

                std::string newAction;
                if (!readLine("Enter Corrected Action (BORROW or RETURN): ", newAction)) break;
                newAction = trimCopy(newAction);

                Status st = m_manager.editHistory(txId, newTitle, newAction);
                if (st == Status::Ok) {
                    m_out << "History entry updated.\n";
                } else if (st == Status::Empty) {
                    m_out << "History is empty.\n";
                } else if (st == Status::NotFound) {
                    m_out << "History record not found.\n";
                } else {
                    m_out << "Invalid history correction (title non-blank, action BORROW or RETURN).\n";
                }
                break;
            }
            case 3: {
                std::uint64_t txId = 0;
                if (!readUint64("Enter Transaction ID to delete: ", txId)) {
                    m_out << "Invalid Transaction ID.\n";
                    break;
                }
                Status st = m_manager.deleteHistory(txId);
                if (st == Status::Ok) {
                    m_out << "History entry deleted.\n";
                } else if (st == Status::Empty) {
                    m_out << "History is empty.\n";
                } else if (st == Status::NotFound) {
                    m_out << "History record not found.\n";
                } else {
                    m_out << "Invalid Transaction ID.\n";
                }
                break;
            }
            case 4: {
                int bookId = 0;
                if (!readInt("Enter Book ID to search in Return Stack: ", bookId)) {
                    m_out << "Invalid Book ID.\n";
                    break;
                }
                size_t offset = 0;
                Status st = m_manager.findInReturnStack(bookId, offset);
                if (st == Status::Ok) {
                    m_out << "Found Book ID " << bookId << " at top-relative offset " << offset << ".\n";
                } else if (st == Status::Empty) {
                    m_out << "Return stack is empty.\n";
                } else {
                    m_out << "Book ID not found in return stack.\n";
                }
                break;
            }
            case 5: {
                int rawOffset = -1;
                if (!readInt("Enter top-relative offset (0 = top): ", rawOffset) || rawOffset < 0) {
                    m_out << "Invalid offset.\n";
                    break;
                }
                int newBookId = 0;
                if (!readInt("Enter New Book ID (must exist in catalogue): ", newBookId)) {
                    m_out << "Invalid Book ID.\n";
                    break;
                }
                Status st = m_manager.updateReturnStackSlot(static_cast<size_t>(rawOffset), newBookId);
                if (st == Status::Ok) {
                    m_out << "Return stack slot updated.\n";
                } else if (st == Status::Empty) {
                    m_out << "Return stack is empty.\n";
                } else if (st == Status::NotFound) {
                    m_out << "Target Book ID does not exist in catalogue.\n";
                } else {
                    m_out << "Invalid offset or Book ID.\n";
                }
                break;
            }
            case 6: {
                int bookId = 0;
                if (!readInt("Enter Book ID to search in Request Queue: ", bookId)) {
                    m_out << "Invalid Book ID.\n";
                    break;
                }
                size_t offset = 0;
                Status st = m_manager.findInRequestQueue(bookId, offset);
                if (st == Status::Ok) {
                    m_out << "Found Book ID " << bookId << " at front-relative offset " << offset << ".\n";
                } else if (st == Status::Empty) {
                    m_out << "Waiting request queue is empty.\n";
                } else {
                    m_out << "Book ID not found in waiting queue.\n";
                }
                break;
            }
            case 7: {
                int rawOffset = -1;
                if (!readInt("Enter front-relative offset (0 = front): ", rawOffset) || rawOffset < 0) {
                    m_out << "Invalid offset.\n";
                    break;
                }
                int newBookId = 0;
                if (!readInt("Enter New Book ID (must exist in catalogue): ", newBookId)) {
                    m_out << "Invalid Book ID.\n";
                    break;
                }
                Status st = m_manager.updateRequestQueueSlot(static_cast<size_t>(rawOffset), newBookId);
                if (st == Status::Ok) {
                    m_out << "Request queue slot updated.\n";
                } else if (st == Status::Empty) {
                    m_out << "Waiting request queue is empty.\n";
                } else if (st == Status::NotFound) {
                    m_out << "Target Book ID does not exist in catalogue.\n";
                } else {
                    m_out << "Invalid offset or Book ID.\n";
                }
                break;
            }
            default:
                m_out << "Invalid maintenance option. Choose 0 through 7.\n";
                break;
        }
    }
}
