#include "HttpServer.hpp"
#include "JsonApiAdapter.hpp"
#include "LibraryManager.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace {
std::string resolveWebDir(const std::string& explicitDir) {
    if (!explicitDir.empty() && fs::exists(fs::path(explicitDir) / "index.html")) {
        return explicitDir;
    }
    const char* candidates[] = {"web", "../web", "../../web", "SmartLibrary/web"};
    for (const char* cand : candidates) {
        if (fs::exists(fs::path(cand) / "index.html")) {
            return fs::weakly_canonical(fs::path(cand)).string();
        }
    }
#ifdef SMARTLIBRARY_WEB_DIR
    if (fs::exists(fs::path(SMARTLIBRARY_WEB_DIR) / "index.html")) {
        return SMARTLIBRARY_WEB_DIR;
    }
#endif
    return "web";
}
} // namespace

int main(int argc, char* argv[]) {
    std::uint16_t port = 8080;
    std::string explicitWebDir;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            long p = std::strtol(argv[++i], nullptr, 10);
            if (p > 0 && p <= 65535) {
                port = static_cast<std::uint16_t>(p);
            }
        } else if (arg == "--web-dir" && i + 1 < argc) {
            explicitWebDir = argv[++i];
        }
    }

    LibraryManager manager;
    JsonApiAdapter adapter(manager);
    std::string webDir = resolveWebDir(explicitWebDir);
    HttpServer server(adapter, webDir, port);

    if (!server.run()) {
        std::cerr << "Failed to start SmartLibrary GUI server.\n";
        return 1;
    }
    return 0;
}
