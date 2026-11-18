#pragma once

#include "LibraryManager.hpp"
#include <mutex>
#include <string>

/**
 * @brief Result returned by JsonApiAdapter for HTTP/UI consumption.
 */
struct ApiResponse {
    int httpStatus{200};
    std::string jsonBody;
};

/**
 * @brief Thread-safe JSON command & state adapter bridging the local HTTP GUI
 * directly to the authoritative C++ LibraryManager engine (zero duplicate logic).
 */
class JsonApiAdapter {
public:
    explicit JsonApiAdapter(LibraryManager& manager);

    /**
     * @brief Route an HTTP method + path + request body to LibraryManager.
     * Serializes mutations with a mutex to prevent concurrent state corruption.
     */
    ApiResponse handleRequest(const std::string& method,
                              const std::string& pathWithQuery,
                              const std::string& body);

    /**
     * @brief Serialize full state of all four data structures to JSON.
     */
    std::string serializeStateJson() const;

    // Helpers exposed for unit/parity testing
    static std::string escapeJson(const std::string& input);
    static bool extractJsonString(const std::string& json, const std::string& key, std::string& outValue);
    static bool extractJsonInt(const std::string& json, const std::string& key, int& outValue);
    static bool extractJsonUint64(const std::string& json, const std::string& key, std::uint64_t& outValue);

private:
    LibraryManager& m_manager;
    mutable std::mutex m_mutex;

    static ApiResponse makeMessageResponse(int httpStatus, bool ok, const std::string& statusName, const std::string& message);
};
