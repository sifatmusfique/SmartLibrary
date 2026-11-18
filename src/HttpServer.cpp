#include "HttpServer.hpp"

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  using SocketHandle = SOCKET;
  constexpr SocketHandle INVALID_SOCK = INVALID_SOCKET;
  inline void closeSocketHandle(SocketHandle s) { closesocket(s); }
#else
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <unistd.h>
  using SocketHandle = int;
  constexpr SocketHandle INVALID_SOCK = -1;
  inline void closeSocketHandle(SocketHandle s) { close(s); }
#endif

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

namespace {
std::string toLowerCopy(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

std::string buildHttpResponse(int statusCode,
                              const std::string& contentType,
                              const std::string& body) {
    const char* reason = "OK";
    switch (statusCode) {
        case 200: reason = "OK"; break;
        case 400: reason = "Bad Request"; break;
        case 404: reason = "Not Found"; break;
        case 405: reason = "Method Not Allowed"; break;
        case 409: reason = "Conflict"; break;
        case 500: reason = "Internal Server Error"; break;
    }

    std::ostringstream oss;
    oss << "HTTP/1.1 " << statusCode << " " << reason << "\r\n"
        << "Content-Type: " << contentType << "\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Cache-Control: no-store\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << body;
    return oss.str();
}
} // namespace

HttpServer::HttpServer(JsonApiAdapter& adapter, std::string webRootDir, std::uint16_t port)
    : m_adapter(adapter), m_webRootDir(std::move(webRootDir)), m_port(port) {}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::stop() noexcept {
    m_running.store(false);
    SocketHandle sock = static_cast<SocketHandle>(m_listenSocket);
    if (sock != INVALID_SOCK) {
        closeSocketHandle(sock);
        m_listenSocket = static_cast<std::uintptr_t>(INVALID_SOCK);
    }
}

bool HttpServer::readStaticFile(const std::string& relativePath,
                                std::string& outContent,
                                std::string& outContentType) const {
    if (relativePath.find("..") != std::string::npos) {
        return false; // Prevent path traversal
    }

    std::string cleanRel = (relativePath == "/" || relativePath.empty()) ? "/index.html" : relativePath;
    std::string fullPath = m_webRootDir + cleanRel;

    std::ifstream ifs(fullPath, std::ios::binary);
    if (!ifs.is_open()) {
        return false;
    }

    std::ostringstream ss;
    ss << ifs.rdbuf();
    outContent = ss.str();

    if (cleanRel.size() >= 5 && cleanRel.substr(cleanRel.size() - 5) == ".html") {
        outContentType = "text/html; charset=utf-8";
    } else if (cleanRel.size() >= 4 && cleanRel.substr(cleanRel.size() - 4) == ".css") {
        outContentType = "text/css; charset=utf-8";
    } else if (cleanRel.size() >= 3 && cleanRel.substr(cleanRel.size() - 3) == ".js") {
        outContentType = "application/javascript; charset=utf-8";
    } else {
        outContentType = "text/plain; charset=utf-8";
    }
    return true;
}

void HttpServer::handleClient(std::uintptr_t rawSocket) {
    SocketHandle client = static_cast<SocketHandle>(rawSocket);

    // Prevent idle speculative browser preconnect sockets from blocking forever
#ifdef _WIN32
    DWORD timeoutMs = 3000;
    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));
#else
    struct timeval tv{};
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

    std::string rawRequest;
    char buffer[4096];
    size_t headerEndPos = std::string::npos;

    while (headerEndPos == std::string::npos) {
        int received = recv(client, buffer, sizeof(buffer), 0);
        if (received <= 0) {
            closeSocketHandle(client);
            return;
        }
        rawRequest.append(buffer, static_cast<size_t>(received));
        headerEndPos = rawRequest.find("\r\n\r\n");
        if (rawRequest.size() > 65536 && headerEndPos == std::string::npos) {
            std::string err = buildHttpResponse(400, "application/json", "{\"ok\":false,\"message\":\"Header too large\"}");
            send(client, err.c_str(), static_cast<int>(err.size()), 0);
            closeSocketHandle(client);
            return;
        }
    }

    std::string headerSection = rawRequest.substr(0, headerEndPos);
    std::string body = rawRequest.substr(headerEndPos + 4);

    // Parse request line
    std::istringstream headerStream(headerSection);
    std::string method, target, httpVersion;
    if (!(headerStream >> method >> target >> httpVersion)) {
        closeSocketHandle(client);
        return;
    }

    // Parse Content-Length
    size_t contentLength = 0;
    std::string line;
    std::getline(headerStream, line); // consume remainder of request line
    while (std::getline(headerStream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        std::string lowerLine = toLowerCopy(line);
        if (lowerLine.rfind("content-length:", 0) == 0) {
            contentLength = static_cast<size_t>(std::strtoull(lowerLine.c_str() + 15, nullptr, 10));
        }
    }

    // Read any remaining body bytes
    while (body.size() < contentLength && body.size() < 1048576) {
        int received = recv(client, buffer, sizeof(buffer), 0);
        if (received <= 0) {
            break;
        }
        body.append(buffer, static_cast<size_t>(received));
    }
    if (body.size() > contentLength) {
        body.resize(contentLength);
    }

    std::string responseStr;
    if (target.rfind("/api/", 0) == 0) {
        ApiResponse apiRes = m_adapter.handleRequest(method, target, body);
        responseStr = buildHttpResponse(apiRes.httpStatus, "application/json; charset=utf-8", apiRes.jsonBody);
    } else if (method == "GET") {
        std::string fileContent, contentType;
        size_t qPos = target.find('?');
        std::string cleanTarget = (qPos == std::string::npos) ? target : target.substr(0, qPos);
        if (readStaticFile(cleanTarget, fileContent, contentType)) {
            responseStr = buildHttpResponse(200, contentType, fileContent);
        } else {
            responseStr = buildHttpResponse(404, "text/plain; charset=utf-8", "404 Not Found");
        }
    } else {
        responseStr = buildHttpResponse(405, "text/plain; charset=utf-8", "405 Method Not Allowed");
    }

    size_t totalSent = 0;
    while (totalSent < responseStr.size()) {
        int sent = send(client,
                        responseStr.c_str() + totalSent,
                        static_cast<int>(responseStr.size() - totalSent),
                        0);
        if (sent <= 0) {
            break;
        }
        totalSent += static_cast<size_t>(sent);
    }

    closeSocketHandle(client);
}

bool HttpServer::run() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return false;
    }
#endif

    SocketHandle listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCK) {
        std::cerr << "Failed to create socket.\n";
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    int opt = 1;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
        closeSocketHandle(listenSock);
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    if (bind(listenSock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        std::cerr << "Failed to bind 127.0.0.1:" << m_port << "\n";
        closeSocketHandle(listenSock);
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    if (listen(listenSock, 16) != 0) {
        std::cerr << "Failed to listen on socket.\n";
        closeSocketHandle(listenSock);
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    m_listenSocket = static_cast<std::uintptr_t>(listenSock);
    m_running.store(true);

    std::cout << "SmartLibrary GUI server running at http://127.0.0.1:" << m_port << "\n";
    std::cout << "Serving static UI from: " << m_webRootDir << "\n";
    std::cout << "Press Ctrl+C to stop.\n" << std::flush;

    while (m_running.load()) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        SocketHandle clientSock = accept(listenSock, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
        if (clientSock == INVALID_SOCK) {
            if (!m_running.load()) {
                break;
            }
            continue;
        }
        std::uintptr_t clientHandle = static_cast<std::uintptr_t>(clientSock);
        std::thread([this, clientHandle]() {
            this->handleClient(clientHandle);
        }).detach();
    }

    stop();
#ifdef _WIN32
    WSACleanup();
#endif
    return true;
}
