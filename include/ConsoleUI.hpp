#pragma once

#include "LibraryManager.hpp"
#include <cstdint>
#include <iostream>
#include <string>

/**
 * @brief Menu-driven console interface wrapping LibraryManager.
 * Accepts configurable input/output streams so both interactive execution
 * and automated scripted console tests (T18, T34, T36) run identically.
 */
class ConsoleUI {
public:
    ConsoleUI(LibraryManager& manager, std::istream& in = std::cin, std::ostream& out = std::cout);

    /**
     * @brief Run the interactive main menu loop until Exit (0) or EOF.
     */
    void run();

private:
    LibraryManager& m_manager;
    std::istream& m_in;
    std::ostream& m_out;

    void printMainMenu() const;
    void printMaintenanceMenu() const;
    void printHelp() const;

    // Input helpers using whole-line parsing to prevent infinite loops and handle EOF safely
    bool readLine(const std::string& prompt, std::string& outLine);
    bool readInt(const std::string& prompt, int& outValue);
    bool readUint64(const std::string& prompt, std::uint64_t& outValue);

    static bool parseStrictInt(const std::string& raw, int& outValue) noexcept;
    static bool parseStrictUint64(const std::string& raw, std::uint64_t& outValue) noexcept;

    // Main menu action handlers
    void handleAddBook();
    void handleDeleteBook();
    void handleSearchBook();
    void handleDisplayBooks() const;
    void handleBorrowBook();
    void handleReturnBook();
    void handleShowHistory() const;
    void handleProcessRequest();
    void handleShowStack() const;
    void handlePopStack();
    void handleShowQueue() const;
    void handleCancelHeadRequest();
    void handleEditBookMetadata();
    void handleMaintenanceMenu();
};
