#pragma once

#include "JsonApiAdapter.hpp"
#include <atomic>
#include <cstdint>
#include <string>

/**
 * @brief Lightweight single-loopback-host HTTP/1.1 server for the optional Smart Library GUI.
 * Strictly binds to 127.0.0.1 and delegates all /api/ endpoints to JsonApiAdapter.
 */
class HttpServer {
public:
    HttpServer(JsonApiAdapter& adapter, std::string webRootDir, std::uint16_t port = 8080);
    ~HttpServer();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    /**
     * @brief Start listening on 127.0.0.1:port and handle requests until stop() is called.
     * @return true if server started and stopped cleanly; false if socket setup failed.
     */
    bool run();

    /**
     * @brief Signal the server loop to exit gracefully.
     */
    void stop() noexcept;

    std::uint16_t getPort() const noexcept { return m_port; }

private:
    JsonApiAdapter& m_adapter;
    std::string m_webRootDir;
    std::uint16_t m_port{8080};
    std::atomic<bool> m_running{false};
    std::uintptr_t m_listenSocket{static_cast<std::uintptr_t>(-1)};

    void handleClient(std::uintptr_t clientSocket);
    bool readStaticFile(const std::string& relativePath, std::string& outContent, std::string& outContentType) const;
};
