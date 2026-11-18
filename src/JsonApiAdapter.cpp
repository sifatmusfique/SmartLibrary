#include "JsonApiAdapter.hpp"
#include <cctype>
#include <cstdlib>
#include <errno.h>
#include <limits>
#include <sstream>

JsonApiAdapter::JsonApiAdapter(LibraryManager& manager)
    : m_manager(manager) {}

std::string JsonApiAdapter::escapeJson(const std::string& input) {
    std::ostringstream oss;
    for (unsigned char c : input) {
        switch (c) {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (c < 0x20) {
                    oss << "\\u00";
                    const char* hex = "0123456789abcdef";
                    oss << hex[(c >> 4) & 0xF] << hex[c & 0xF];
                } else {
                    oss << static_cast<char>(c);
                }
        }
    }
    return oss.str();
}

namespace {
bool skipJsonString(const std::string& json, size_t& pos) noexcept {
    if (pos >= json.size() || json[pos] != '"') {
        return false;
    }
    ++pos;
    while (pos < json.size()) {
        char c = json[pos++];
        if (c == '\\') {
            if (pos < json.size()) {
                ++pos;
            }
        } else if (c == '"') {
            return true;
        }
    }
    return false;
}

bool findJsonValueStart(const std::string& json, const std::string& key, size_t& outPos) {
    size_t pos = 0;
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
        ++pos;
    }
    if (pos >= json.size() || json[pos] != '{') {
        return false;
    }

    int depth = 0;
    while (pos < json.size()) {
        char c = json[pos];
        if (c == '{' || c == '[') {
            ++depth;
            ++pos;
        } else if (c == '}' || c == ']') {
            --depth;
            ++pos;
        } else if (c == '"') {
            size_t keyStart = pos + 1;
            if (!skipJsonString(json, pos)) {
                return false;
            }
            size_t keyEnd = pos - 1;
            std::string currentKey = json.substr(keyStart, keyEnd - keyStart);

            size_t colonPos = pos;
            while (colonPos < json.size() && std::isspace(static_cast<unsigned char>(json[colonPos]))) {
                ++colonPos;
            }
            if (depth == 1 && colonPos < json.size() && json[colonPos] == ':') {
                size_t valStart = colonPos + 1;
                while (valStart < json.size() && std::isspace(static_cast<unsigned char>(json[valStart]))) {
                    ++valStart;
                }
                if (currentKey == key && valStart < json.size()) {
                    outPos = valStart;
                    return true;
                }
            }
        } else {
            ++pos;
        }
    }
    return false;
}

bool parseQueryParamInt(const std::string& query, const std::string& paramName, int& outValue) noexcept {
    std::string prefix = paramName + "=";
    if (query.rfind(prefix, 0) != 0) {
        return false;
    }
    const char* raw = query.c_str() + prefix.size();
    if (*raw == '\0') {
        return false;
    }
    char* endPtr = nullptr;
    errno = 0;
    long parsed = std::strtol(raw, &endPtr, 10);
    if (errno == ERANGE || endPtr == raw || *endPtr != '\0' ||
        parsed < std::numeric_limits<int>::min() || parsed > std::numeric_limits<int>::max()) {
        return false;
    }
    outValue = static_cast<int>(parsed);
    return true;
}

bool parseQueryParamUint64(const std::string& query, const std::string& paramName, std::uint64_t& outValue) noexcept {
    std::string prefix = paramName + "=";
    if (query.rfind(prefix, 0) != 0) {
        return false;
    }
    const char* raw = query.c_str() + prefix.size();
    if (*raw == '\0' || *raw == '-') {
        return false;
    }
    char* endPtr = nullptr;
    errno = 0;
    unsigned long long parsed = std::strtoull(raw, &endPtr, 10);
    if (errno == ERANGE || endPtr == raw || *endPtr != '\0') {
        return false;
    }
    outValue = static_cast<std::uint64_t>(parsed);
    return true;
}
bool extractJsonNumberToken(const std::string& json, const std::string& key, bool& outQuoted, size_t& outStartPos) {
    size_t pos = 0;
    if (!findJsonValueStart(json, key, pos) || pos >= json.size()) {
        return false;
    }
    outQuoted = (json[pos] == '"');
    if (outQuoted) {
        ++pos;
    }
    if (pos >= json.size()) {
        return false;
    }
    outStartPos = pos;
    return true;
}
} // namespace

bool JsonApiAdapter::extractJsonString(const std::string& json, const std::string& key, std::string& outValue) {
    size_t pos = 0;
    if (!findJsonValueStart(json, key, pos) || pos >= json.size() || json[pos] != '"') {
        return false;
    }
    ++pos;
    std::string result;
    while (pos < json.size()) {
        char c = json[pos++];
        if (c == '"') {
            outValue = result;
            return true;
        }
        if (c == '\\' && pos < json.size()) {
            char esc = json[pos++];
            switch (esc) {
                case '"':  result.push_back('"'); break;
                case '\\': result.push_back('\\'); break;
                case '/':  result.push_back('/'); break;
                case 'b':  result.push_back('\b'); break;
                case 'f':  result.push_back('\f'); break;
                case 'n':  result.push_back('\n'); break;
                case 'r':  result.push_back('\r'); break;
                case 't':  result.push_back('\t'); break;
                default:   result.push_back(esc); break;
            }
        } else {
            result.push_back(c);
        }
    }
    return false;
}

bool JsonApiAdapter::extractJsonInt(const std::string& json, const std::string& key, int& outValue) {
    bool quoted = false;
    size_t pos = 0;
    if (!extractJsonNumberToken(json, key, quoted, pos)) {
        return false;
    }
    const char* start = json.c_str() + pos;
    char* endPtr = nullptr;
    errno = 0;
    long parsed = std::strtol(start, &endPtr, 10);
    if (errno == ERANGE || endPtr == start) {
        return false;
    }
    if (quoted && *endPtr != '"') {
        return false;
    }
    if (!quoted && *endPtr != '\0' && *endPtr != ',' && *endPtr != '}' &&
        !std::isspace(static_cast<unsigned char>(*endPtr))) {
        return false;
    }
    if (parsed < std::numeric_limits<int>::min() || parsed > std::numeric_limits<int>::max()) {
        return false;
    }
    outValue = static_cast<int>(parsed);
    return true;
}

bool JsonApiAdapter::extractJsonUint64(const std::string& json, const std::string& key, std::uint64_t& outValue) {
    bool quoted = false;
    size_t pos = 0;
    if (!extractJsonNumberToken(json, key, quoted, pos) || json[pos] == '-') {
        return false;
    }
    const char* start = json.c_str() + pos;
    char* endPtr = nullptr;
    errno = 0;
    unsigned long long parsed = std::strtoull(start, &endPtr, 10);
    if (errno == ERANGE || endPtr == start) {
        return false;
    }
    if (quoted && *endPtr != '"') {
        return false;
    }
    if (!quoted && *endPtr != '\0' && *endPtr != ',' && *endPtr != '}' &&
        !std::isspace(static_cast<unsigned char>(*endPtr))) {
        return false;
    }
    outValue = static_cast<std::uint64_t>(parsed);
    return true;
}

ApiResponse JsonApiAdapter::makeMessageResponse(int httpStatus, bool ok, const std::string& statusName, const std::string& message) {
    std::ostringstream oss;
    oss << "{\"ok\":" << (ok ? "true" : "false")
        << ",\"status\":\"" << escapeJson(statusName) << "\""
        << ",\"message\":\"" << escapeJson(message) << "\"}";
    return ApiResponse{httpStatus, oss.str()};
}

std::string JsonApiAdapter::serializeStateJson() const {
    const BookCatalogue& cat = m_manager.getCatalogue();
    const HistoryList& hist = m_manager.getHistory();
    const ReturnStack& stack = m_manager.getReturnStack();
    const RequestQueue& queue = m_manager.getRequestQueue();

    std::ostringstream oss;
    oss << "{";

    // 1. Catalogue
    oss << "\"catalogue\":{\"count\":" << cat.getCount()
        << ",\"capacity\":" << cat.getCapacity() << ",\"items\":[";
    bool first = true;
    cat.traverse([&](const Book& b) {
        if (!first) oss << ",";
        first = false;
        oss << "{\"id\":" << b.id
            << ",\"title\":\"" << escapeJson(b.title) << "\""
            << ",\"author\":\"" << escapeJson(b.author) << "\""
            << ",\"available\":" << (b.available ? "true" : "false") << "}";
    });
    oss << "]},";

    // 2. HistoryList
    oss << "\"history\":{\"count\":" << hist.getCount()
        << ",\"nextTxId\":" << hist.getNextTransactionId() << ",\"items\":[";
    first = true;
    hist.traverse([&](const HistoryRecord& rec) {
        if (!first) oss << ",";
        first = false;
        oss << "{\"transactionId\":" << rec.transactionId
            << ",\"bookId\":" << rec.bookId
            << ",\"title\":\"" << escapeJson(rec.title) << "\""
            << ",\"action\":\"" << escapeJson(rec.action) << "\"}";
    });
    oss << "]},";

    // 3. ReturnStack (top to bottom)
    oss << "\"returnStack\":{\"count\":" << stack.getCount()
        << ",\"capacity\":" << stack.getCapacity() << ",\"items\":[";
    first = true;
    stack.traverse([&](int id) {
        if (!first) oss << ",";
        first = false;
        oss << id;
    });
    oss << "]},";

    // 4. RequestQueue (front to rear + frontIndex for circular visualization)
    oss << "\"requestQueue\":{\"count\":" << queue.getCount()
        << ",\"capacity\":" << queue.getCapacity()
        << ",\"frontIndex\":" << queue.getFrontIndex() << ",\"items\":[";
    first = true;
    queue.traverse([&](int id) {
        if (!first) oss << ",";
        first = false;
        oss << id;
    });
    oss << "]}";

    oss << "}";
    return oss.str();
}

ApiResponse JsonApiAdapter::handleRequest(const std::string& method,
                                          const std::string& pathWithQuery,
                                          const std::string& body) {
    std::lock_guard<std::mutex> lock(m_mutex);

    size_t qPos = pathWithQuery.find('?');
    std::string path = (qPos == std::string::npos) ? pathWithQuery : pathWithQuery.substr(0, qPos);
    std::string query = (qPos == std::string::npos) ? "" : pathWithQuery.substr(qPos + 1);

    if (method == "GET" && path == "/api/state") {
        return ApiResponse{200, serializeStateJson()};
    }

    if (method == "GET" && path == "/api/books/search") {
        int id = 0;
        if (!parseQueryParamInt(query, "id", id)) {
            return makeMessageResponse(400, false, "Invalid", "Invalid or missing book ID query parameter.");
        }

        Book b;
        Status st = m_manager.searchBook(id, b);
        if (st == Status::Ok) {
            std::ostringstream oss;
            oss << "{\"ok\":true,\"status\":\"Ok\",\"book\":{"
                << "\"id\":" << b.id
                << ",\"title\":\"" << escapeJson(b.title) << "\""
                << ",\"author\":\"" << escapeJson(b.author) << "\""
                << ",\"available\":" << (b.available ? "true" : "false") << "}}";
            return ApiResponse{200, oss.str()};
        }
        if (st == Status::NotFound) {
            return makeMessageResponse(404, false, "NotFound", "Book not found.");
        }
        return makeMessageResponse(400, false, statusToString(st), "Invalid book ID.");
    }

    if (method == "GET" && path == "/api/maintenance/history/search") {
        std::uint64_t txId = 0;
        if (!parseQueryParamUint64(query, "txId", txId)) {
            return makeMessageResponse(400, false, "Invalid", "Invalid or missing txId parameter.");
        }
        HistoryRecord rec;
        Status st = m_manager.findHistory(txId, rec);
        if (st == Status::Ok) {
            std::ostringstream oss;
            oss << "{\"ok\":true,\"status\":\"Ok\",\"message\":\"Found Tx#" << rec.transactionId
                << " (Book " << rec.bookId << ": " << escapeJson(rec.title) << " [" << escapeJson(rec.action) << "])\","
                << "\"record\":{\"transactionId\":" << rec.transactionId
                << ",\"bookId\":" << rec.bookId
                << ",\"title\":\"" << escapeJson(rec.title) << "\""
                << ",\"action\":\"" << escapeJson(rec.action) << "\"}}";
            return ApiResponse{200, oss.str()};
        }
        if (st == Status::Empty) return makeMessageResponse(409, false, "Empty", "History is empty.");
        if (st == Status::NotFound) return makeMessageResponse(404, false, "NotFound", "History record not found.");
        return makeMessageResponse(400, false, statusToString(st), "Invalid Transaction ID.");
    }

    if (method == "GET" && path == "/api/maintenance/stack/search") {
        int bookId = 0;
        if (!parseQueryParamInt(query, "id", bookId)) {
            return makeMessageResponse(400, false, "Invalid", "Invalid or missing Book ID parameter.");
        }
        size_t offset = 0;
        Status st = m_manager.findInReturnStack(bookId, offset);
        if (st == Status::Ok) {
            std::ostringstream oss;
            oss << "{\"ok\":true,\"status\":\"Ok\",\"offset\":" << offset
                << ",\"message\":\"Found Book ID " << bookId << " at top-relative offset " << offset << ".\"}";
            return ApiResponse{200, oss.str()};
        }
        if (st == Status::Empty) return makeMessageResponse(409, false, "Empty", "Return stack is empty.");
        if (st == Status::NotFound) return makeMessageResponse(404, false, "NotFound", "Book ID not found in return stack.");
        return makeMessageResponse(400, false, statusToString(st), "Invalid Book ID.");
    }

    if (method == "GET" && path == "/api/maintenance/queue/search") {
        int bookId = 0;
        if (!parseQueryParamInt(query, "id", bookId)) {
            return makeMessageResponse(400, false, "Invalid", "Invalid or missing Book ID parameter.");
        }
        size_t offset = 0;
        Status st = m_manager.findInRequestQueue(bookId, offset);
        if (st == Status::Ok) {
            std::ostringstream oss;
            oss << "{\"ok\":true,\"status\":\"Ok\",\"offset\":" << offset
                << ",\"message\":\"Found Book ID " << bookId << " at front-relative offset " << offset << ".\"}";
            return ApiResponse{200, oss.str()};
        }
        if (st == Status::Empty) return makeMessageResponse(409, false, "Empty", "Waiting request queue is empty.");
        if (st == Status::NotFound) return makeMessageResponse(404, false, "NotFound", "Book ID not found in waiting queue.");
        return makeMessageResponse(400, false, statusToString(st), "Invalid Book ID.");
    }

    if (method != "POST") {
        return makeMessageResponse(405, false, "Invalid", "Method not allowed.");
    }

    if (path == "/api/books/add") {
        int id = 0;
        std::string title, author;
        if (!extractJsonInt(body, "id", id) ||
            !extractJsonString(body, "title", title) ||
            !extractJsonString(body, "author", author)) {
            return makeMessageResponse(400, false, "Invalid", "Expected numeric id, title, and author.");
        }
        Status st = m_manager.addBook(id, title, author);
        if (st == Status::Ok) return makeMessageResponse(200, true, "Ok", "Book added successfully.");
        if (st == Status::Duplicate) return makeMessageResponse(409, false, "Duplicate", "Duplicate Book ID. Book not added.");
        if (st == Status::Full) return makeMessageResponse(409, false, "Full", "Catalogue is full. Cannot add more books.");
        return makeMessageResponse(400, false, statusToString(st), "Invalid book details (ID must be > 0 and title/author non-empty).");
    }

    if (path == "/api/books/delete") {
        int id = 0;
        if (!extractJsonInt(body, "id", id)) {
            return makeMessageResponse(400, false, "Invalid", "Invalid book ID.");
        }
        Status st = m_manager.deleteBook(id);
        if (st == Status::Ok) return makeMessageResponse(200, true, "Ok", "Book deleted successfully.");
        if (st == Status::NotFound) return makeMessageResponse(404, false, "NotFound", "Book not found.");
        if (st == Status::Blocked) return makeMessageResponse(409, false, "Blocked", "Cannot delete: book is currently borrowed or has pending requests.");
        return makeMessageResponse(400, false, statusToString(st), "Invalid book ID.");
    }

    if (path == "/api/books/edit") {
        int id = 0;
        std::string title, author;
        if (!extractJsonInt(body, "id", id) ||
            !extractJsonString(body, "title", title) ||
            !extractJsonString(body, "author", author)) {
            return makeMessageResponse(400, false, "Invalid", "Expected id, title, and author.");
        }
        Status st = m_manager.editBookMetadata(id, title, author);
        if (st == Status::Ok) return makeMessageResponse(200, true, "Ok", "Book metadata updated successfully.");
        if (st == Status::NotFound) return makeMessageResponse(404, false, "NotFound", "Book not found.");
        return makeMessageResponse(400, false, statusToString(st), "Invalid metadata (title and author cannot be blank).");
    }

    if (path == "/api/circulation/borrow") {
        int id = 0;
        if (!extractJsonInt(body, "id", id)) {
            return makeMessageResponse(400, false, "Invalid", "Invalid book ID.");
        }
        BorrowAction action = BorrowAction::None;
        std::uint64_t txId = 0;
        Status st = m_manager.borrowBook(id, action, &txId);
        if (st == Status::Ok) {
            if (action == BorrowAction::Borrowed) {
                return makeMessageResponse(200, true, "Borrowed", "Book borrowed successfully.");
            }
            return makeMessageResponse(200, true, "Queued", "Book is already borrowed. Request added to queue.");
        }
        if (st == Status::NotFound) return makeMessageResponse(404, false, "NotFound", "Book not found.");
        if (st == Status::Full) return makeMessageResponse(409, false, "Full", "Queue Overflow! Waiting request queue is full.");
        return makeMessageResponse(400, false, statusToString(st), "Invalid book ID.");
    }

    if (path == "/api/circulation/return") {
        int id = 0;
        if (!extractJsonInt(body, "id", id)) {
            return makeMessageResponse(400, false, "Invalid", "Invalid book ID.");
        }
        Status st = m_manager.returnBook(id);
        if (st == Status::Ok) return makeMessageResponse(200, true, "Ok", "Book returned successfully.");
        if (st == Status::NotFound) return makeMessageResponse(404, false, "NotFound", "Book not found.");
        if (st == Status::Blocked) return makeMessageResponse(409, false, "Blocked", "Book is already available; cannot return.");
        if (st == Status::Full) return makeMessageResponse(409, false, "Full", "Stack Overflow! Return stack is full; pop a recent return first.");
        return makeMessageResponse(400, false, statusToString(st), "Invalid book ID.");
    }

    if (path == "/api/circulation/process") {
        int bookId = 0;
        Status st = m_manager.processNextRequest(bookId);
        if (st == Status::Ok) {
            return makeMessageResponse(200, true, "Ok", "Processed waiting request for Book ID " + std::to_string(bookId) + ". Book borrowed successfully.");
        }
        if (st == Status::Empty) return makeMessageResponse(409, false, "Empty", "Queue Underflow! No pending requests.");
        if (st == Status::Blocked) return makeMessageResponse(409, false, "Blocked", "Head request (Book ID " + std::to_string(bookId) + ") is currently unavailable. Queue left unchanged.");
        return makeMessageResponse(400, false, statusToString(st), "Failed to process request.");
    }

    if (path == "/api/circulation/cancel") {
        int bookId = 0;
        Status st = m_manager.cancelHeadRequest(bookId);
        if (st == Status::Ok) {
            return makeMessageResponse(200, true, "Ok", "Cancelled head waiting request for Book ID " + std::to_string(bookId) + ".");
        }
        return makeMessageResponse(409, false, "Empty", "Queue Underflow! No pending requests to cancel.");
    }

    if (path == "/api/circulation/pop-stack") {
        int bookId = 0;
        Status st = m_manager.popReturnStack(bookId);
        if (st == Status::Ok) {
            return makeMessageResponse(200, true, "Ok", "Popped Book ID " + std::to_string(bookId) + " from return stack.");
        }
        return makeMessageResponse(409, false, "Empty", "Stack Underflow!");
    }

    if (path == "/api/maintenance/history/edit") {
        std::uint64_t txId = 0;
        std::string title, action;
        if (!extractJsonUint64(body, "txId", txId) ||
            !extractJsonString(body, "title", title) ||
            !extractJsonString(body, "action", action)) {
            return makeMessageResponse(400, false, "Invalid", "Expected txId, title, and action (BORROW/RETURN).");
        }
        Status st = m_manager.editHistory(txId, title, action);
        if (st == Status::Ok) return makeMessageResponse(200, true, "Ok", "History entry updated.");
        if (st == Status::Empty) return makeMessageResponse(409, false, "Empty", "History is empty.");
        if (st == Status::NotFound) return makeMessageResponse(404, false, "NotFound", "History record not found.");
        return makeMessageResponse(400, false, statusToString(st), "Invalid history correction.");
    }

    if (path == "/api/maintenance/history/delete") {
        std::uint64_t txId = 0;
        if (!extractJsonUint64(body, "txId", txId)) {
            return makeMessageResponse(400, false, "Invalid", "Expected valid txId.");
        }
        Status st = m_manager.deleteHistory(txId);
        if (st == Status::Ok) return makeMessageResponse(200, true, "Ok", "History entry deleted.");
        if (st == Status::Empty) return makeMessageResponse(409, false, "Empty", "History is empty.");
        if (st == Status::NotFound) return makeMessageResponse(404, false, "NotFound", "History record not found.");
        return makeMessageResponse(400, false, statusToString(st), "Invalid Transaction ID.");
    }

    if (path == "/api/maintenance/stack/update") {
        int offset = -1, bookId = 0;
        if (!extractJsonInt(body, "offset", offset) || offset < 0 || !extractJsonInt(body, "bookId", bookId)) {
            return makeMessageResponse(400, false, "Invalid", "Expected non-negative offset and bookId.");
        }
        Status st = m_manager.updateReturnStackSlot(static_cast<size_t>(offset), bookId);
        if (st == Status::Ok) return makeMessageResponse(200, true, "Ok", "Return stack slot updated.");
        if (st == Status::Empty) return makeMessageResponse(409, false, "Empty", "Return stack is empty.");
        if (st == Status::NotFound) return makeMessageResponse(404, false, "NotFound", "Target Book ID does not exist in catalogue.");
        return makeMessageResponse(400, false, statusToString(st), "Invalid offset or Book ID.");
    }

    if (path == "/api/maintenance/queue/update") {
        int offset = -1, bookId = 0;
        if (!extractJsonInt(body, "offset", offset) || offset < 0 || !extractJsonInt(body, "bookId", bookId)) {
            return makeMessageResponse(400, false, "Invalid", "Expected non-negative offset and bookId.");
        }
        Status st = m_manager.updateRequestQueueSlot(static_cast<size_t>(offset), bookId);
        if (st == Status::Ok) return makeMessageResponse(200, true, "Ok", "Request queue slot updated.");
        if (st == Status::Empty) return makeMessageResponse(409, false, "Empty", "Waiting request queue is empty.");
        if (st == Status::NotFound) return makeMessageResponse(404, false, "NotFound", "Target Book ID does not exist in catalogue.");
        return makeMessageResponse(400, false, statusToString(st), "Invalid offset or Book ID.");
    }

    return makeMessageResponse(404, false, "NotFound", "Endpoint not found.");
}
