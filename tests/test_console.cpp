#include "ConsoleUI.hpp"
#include "LibraryManager.hpp"
#include <iostream>
#include <sstream>
#include <string>

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        ++g_tests_run; \
        if (cond) { \
            ++g_tests_passed; \
            std::cout << "  [PASS] " << msg << "\n"; \
        } else { \
            std::cerr << "  [FAIL] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
        } \
    } while (0)

void test_T36_original_sample_scripts() {
    std::cout << "\n=== Console Tests (T36: Original Group C Paper Sample Inputs) ===\n";
    LibraryManager mgr;
    std::istringstream input(
        "1\n101\nData Structures\nSeymour Lipschutz\n" // Add 101
        "3\n101\n"                                     // Search 101
        "5\n101\n"                                     // Borrow 101
        "5\n101\n"                                     // Borrow 101 again -> Queue
        "12\n"                                         // Cancel head request
        "6\n101\n"                                     // Return 101
        "9\n"                                          // Show stack
        "10\n"                                         // Pop stack (101)
        "10\n"                                         // Pop empty stack -> Stack Underflow!
        "3\n999\n"                                     // Search missing 999 -> Book not found.
        "0\n"                                          // Exit
    );
    std::ostringstream output;

    ConsoleUI ui(mgr, input, output);
    ui.run();

    std::string out = output.str();
    TEST_ASSERT(out.find("Book added successfully.") != std::string::npos, "T36: Output confirms book added");
    TEST_ASSERT(out.find("ID: 101") != std::string::npos &&
                out.find("Title: Data Structures") != std::string::npos &&
                out.find("Author: Seymour Lipschutz") != std::string::npos &&
                out.find("Status: Available") != std::string::npos,
                "T36: Search displays exact 101 book details");
    TEST_ASSERT(out.find("Book borrowed successfully.") != std::string::npos, "T36: Output confirms borrow");
    TEST_ASSERT(out.find("Book is already borrowed. Request added to queue.") != std::string::npos,
                "T36: Output confirms repeat borrow queued");
    TEST_ASSERT(out.find("Book returned successfully.") != std::string::npos, "T36: Output confirms return");
    TEST_ASSERT(out.find("Return stack (top to bottom): 101") != std::string::npos,
                "T36: Output displays stack with 101");
    TEST_ASSERT(out.find("Stack Underflow!") != std::string::npos, "T36: Output displays Stack Underflow!");
    TEST_ASSERT(out.find("Book not found.") != std::string::npos, "T36: Output displays Book not found.");
    TEST_ASSERT(out.find("Exiting Smart Library. Goodbye!") != std::string::npos, "T36: Clean exit message");
}

void test_T18_T34_malformed_input_overflow_and_eof() {
    std::cout << "\n=== Console Tests (T18, T34: Malformed Input, Integer Overflow, Long Text, EOF) ===\n";
    LibraryManager mgr;
    std::string longTitle(500, 'X');
    std::istringstream input(
        "abc\n"                       // Malformed non-numeric menu
        "99\n"                        // Out-of-range menu option
        "12abc\n"                     // Trailing characters in integer
        "99999999999999999999999\n"   // Integer overflow (> INT_MAX)
        "1\n105\n" + longTitle + "\nAuthor Long\n" // Long title add
        "13\n105\nUpdated Title\nUpdated Author\n" // Menu 13 edit metadata
        "14\n1\n999\n0\n"             // Maintenance submenu search empty history, exit submenu
        "15\n"                        // Help menu
        // End stream abruptly WITHOUT 0 -> tests EOF handling!
    );
    std::ostringstream output;

    ConsoleUI ui(mgr, input, output);
    ui.run();

    std::string out = output.str();
    TEST_ASSERT(out.find("Invalid menu input.") != std::string::npos, "T18: Rejects malformed/overflow menu lines without infinite loop");
    TEST_ASSERT(out.find("Invalid menu option.") != std::string::npos, "T18: Rejects out-of-range menu option 99");
    TEST_ASSERT(out.find("Book added successfully.") != std::string::npos, "T34: Handles 500-char long title safely");
    TEST_ASSERT(out.find("Book metadata updated successfully.") != std::string::npos, "Menu 13 metadata edit works via console");
    TEST_ASSERT(out.find("EOF reached. Exiting cleanly.") != std::string::npos, "T18/T34: Terminates cleanly on EOF");

    Book edited;
    TEST_ASSERT(mgr.searchBook(105, edited) == Status::Ok && edited.title == "Updated Title",
                "Console mutations persisted properly in LibraryManager");
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "  SmartLibrary SL-004 ConsoleUI Test Suite          \n";
    std::cout << "====================================================\n";

    test_T36_original_sample_scripts();
    test_T18_T34_malformed_input_overflow_and_eof();

    std::cout << "\n====================================================\n";
    std::cout << "Test Summary: " << g_tests_passed << " / " << g_tests_run << " passed.\n";
    std::cout << "====================================================\n";

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}
