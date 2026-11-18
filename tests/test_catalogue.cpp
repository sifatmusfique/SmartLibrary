#include "BookCatalogue.hpp"
#include <iostream>
#include <stdexcept>
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

void test_T01_add_and_search() {
    std::cout << "\n=== Running T01: Add valid book and search ===\n";
    BookCatalogue catalogue(10);
    Book b1(101, "Data Structures", "Seymour Lipschutz", true);
    
    Status addStatus = catalogue.add(b1);
    TEST_ASSERT(addStatus == Status::Ok, "T01: Add book 101 returned Ok");
    TEST_ASSERT(catalogue.getCount() == 1, "T01: Catalogue count is 1");

    Book found;
    Status findStatus = catalogue.find(101, found);
    TEST_ASSERT(findStatus == Status::Ok, "T01: Find book 101 returned Ok");
    TEST_ASSERT(found.id == 101, "T01: Found book ID matches 101");
    TEST_ASSERT(found.title == "Data Structures", "T01: Found book title matches");
    TEST_ASSERT(found.author == "Seymour Lipschutz", "T01: Found book author matches");
    TEST_ASSERT(found.available == true, "T01: Found book status is Available");
}

void test_T06_search_nonexistent() {
    std::cout << "\n=== Running T06: Search nonexistent book 999 ===\n";
    BookCatalogue catalogue(10);
    catalogue.add(Book(101, "Data Structures", "Seymour Lipschutz"));

    Book outBook;
    Status status = catalogue.find(999, outBook);
    TEST_ASSERT(status == Status::NotFound, "T06: Search 999 returned NotFound");
    TEST_ASSERT(!catalogue.contains(999), "T06: contains(999) returned false");
}

void test_T07_invalid_inputs_and_duplicate() {
    std::cout << "\n=== Running T07: Duplicate ID, invalid ID, and blank fields ===\n";
    BookCatalogue catalogue(10);
    catalogue.add(Book(101, "Data Structures", "Seymour Lipschutz"));

    // 1. Duplicate ID
    Status dupStatus = catalogue.add(Book(101, "Duplicate Book", "Different Author"));
    TEST_ASSERT(dupStatus == Status::Duplicate, "T07: Duplicate ID 101 rejected with Duplicate status");
    TEST_ASSERT(catalogue.getCount() == 1, "T07: Count remains 1 after duplicate rejection");

    // 2. Non-positive ID
    Status zeroIdStatus = catalogue.add(Book(0, "Invalid Zero", "Author"));
    TEST_ASSERT(zeroIdStatus == Status::Invalid, "T07: ID 0 rejected with Invalid status");

    Status negIdStatus = catalogue.add(Book(-5, "Negative ID", "Author"));
    TEST_ASSERT(negIdStatus == Status::Invalid, "T07: Negative ID rejected with Invalid status");

    // 3. Blank or whitespace-only title or author
    Status blankTitle = catalogue.add(Book(102, "", "Author"));
    TEST_ASSERT(blankTitle == Status::Invalid, "T07: Blank title rejected with Invalid status");

    Status wsTitle = catalogue.add(Book(102, "   \t  ", "Author"));
    TEST_ASSERT(wsTitle == Status::Invalid, "T07: Whitespace-only title rejected with Invalid status");

    Status blankAuthor = catalogue.add(Book(103, "Valid Title", ""));
    TEST_ASSERT(blankAuthor == Status::Invalid, "T07: Blank author rejected with Invalid status");

    TEST_ASSERT(catalogue.getCount() == 1, "T07: Catalogue count preserved at 1 without corruption");
}

void test_T08_full_catalogue() {
    std::cout << "\n=== Running T08: Full catalogue bounded overflow ===\n";
    BookCatalogue smallCat(3); // Small capacity 3 for testing
    TEST_ASSERT(smallCat.getCapacity() == 3, "T08: Configurable capacity is 3");
    
    TEST_ASSERT(smallCat.add(Book(1, "Book 1", "Author 1")) == Status::Ok, "T08: Added book 1");
    TEST_ASSERT(smallCat.add(Book(2, "Book 2", "Author 2")) == Status::Ok, "T08: Added book 2");
    TEST_ASSERT(smallCat.add(Book(3, "Book 3", "Author 3")) == Status::Ok, "T08: Added book 3");
    TEST_ASSERT(smallCat.isFull(), "T08: Catalogue isFull() returns true");

    // Attempting to add 4th book
    Status fullStatus = smallCat.add(Book(4, "Book 4", "Author 4"));
    TEST_ASSERT(fullStatus == Status::Full, "T08: Adding past capacity returns Full status");
    TEST_ASSERT(smallCat.getCount() == 3, "T08: Count remains bounded at 3");
    TEST_ASSERT(!smallCat.contains(4), "T08: Rejected book 4 is not in catalogue");
}

void test_T09_delete_first_middle_last() {
    std::cout << "\n=== Running T09: Delete first, middle, last with left-shifting ===\n";
    BookCatalogue cat(5);
    cat.add(Book(10, "Book 10", "A10"));
    cat.add(Book(20, "Book 20", "A20"));
    cat.add(Book(30, "Book 30", "A30"));
    cat.add(Book(40, "Book 40", "A40"));
    cat.add(Book(50, "Book 50", "A50"));
    TEST_ASSERT(cat.getCount() == 5, "T09: Initial count is 5");

    // 1. Delete middle element (30 at index 2)
    Status delMiddle = cat.remove(30);
    TEST_ASSERT(delMiddle == Status::Ok, "T09: Remove middle element 30 returned Ok");
    TEST_ASSERT(cat.getCount() == 4, "T09: Count decreased to 4");
    TEST_ASSERT(!cat.contains(30), "T09: Book 30 no longer exists");
    // Verify shift: index 0=10, 1=20, 2=40, 3=50
    TEST_ASSERT(cat.getAt(2)->id == 40, "T09: Element 40 shifted left into index 2");
    TEST_ASSERT(cat.getAt(3)->id == 50, "T09: Element 50 shifted left into index 3");

    // 2. Delete first element (10 at index 0)
    Status delFirst = cat.remove(10);
    TEST_ASSERT(delFirst == Status::Ok, "T09: Remove first element 10 returned Ok");
    TEST_ASSERT(cat.getCount() == 3, "T09: Count decreased to 3");
    TEST_ASSERT(cat.getAt(0)->id == 20, "T09: Element 20 shifted left into index 0");

    // 3. Delete last element (50 at index 2)
    Status delLast = cat.remove(50);
    TEST_ASSERT(delLast == Status::Ok, "T09: Remove last element 50 returned Ok");
    TEST_ASSERT(cat.getCount() == 2, "T09: Count decreased to 2");
    TEST_ASSERT(cat.getAt(0)->id == 20, "T09: Remaining slot 0 is 20");
    TEST_ASSERT(cat.getAt(1)->id == 40, "T09: Remaining slot 1 is 40");
    TEST_ASSERT(cat.getAt(2) == nullptr, "T09: Out of bounds slot returns nullptr");

    // 4. Delete nonexistent element
    Status delNonexistent = cat.remove(999);
    TEST_ASSERT(delNonexistent == Status::NotFound, "T09: Removing non-existent ID returns NotFound");
    TEST_ASSERT(cat.getCount() == 2, "T09: Count unchanged after failed delete");
}

void test_T21_edit_metadata_and_update_status() {
    std::cout << "\n=== Running T21: Edit title/author and status update ===\n";
    BookCatalogue cat(10);
    cat.add(Book(101, "Old Title", "Old Author", true));

    // 1. Valid metadata edit
    Status editOk = cat.editMetadata(101, "New Modern Title", "New Verified Author");
    TEST_ASSERT(editOk == Status::Ok, "T21: Valid editMetadata returned Ok");

    Book updated;
    cat.find(101, updated);
    TEST_ASSERT(updated.title == "New Modern Title", "T21: Title successfully updated");
    TEST_ASSERT(updated.author == "New Verified Author", "T21: Author successfully updated");
    TEST_ASSERT(updated.id == 101, "T21: ID remained strictly immutable");
    TEST_ASSERT(updated.available == true, "T21: Availability status unchanged by metadata edit");

    // 2. Invalid blank edits
    TEST_ASSERT(cat.editMetadata(101, "", "Valid Author") == Status::Invalid, "T21: Blank title rejected");
    TEST_ASSERT(cat.editMetadata(101, "Valid Title", "") == Status::Invalid, "T21: Blank author rejected");

    // 3. Edit nonexistent ID
    TEST_ASSERT(cat.editMetadata(999, "Title", "Author") == Status::NotFound, "T21: Edit nonexistent ID returns NotFound");

    // 4. Update status primitive
    TEST_ASSERT(cat.updateStatus(101, false) == Status::Ok, "T21: Update status to false returned Ok");
    cat.find(101, updated);
    TEST_ASSERT(updated.available == false, "T21: Availability status is now false (Borrowed)");

    TEST_ASSERT(cat.updateStatus(101, true) == Status::Ok, "T21: Update status to true returned Ok");
    cat.find(101, updated);
    TEST_ASSERT(updated.available == true, "T21: Availability status is now true (Available)");

    TEST_ASSERT(cat.updateStatus(999, true) == Status::NotFound, "T21: Update status on missing ID returns NotFound");
}

void test_T31_capacity_validation_and_edge_cases() {
    std::cout << "\n=== Running T31: Capacity validation and single-slot catalogue ===\n";
    // 1. Zero capacity rejection
    bool caughtZeroCapacity = false;
    try {
        BookCatalogue invalidCat(0);
    } catch (const std::invalid_argument&) {
        caughtZeroCapacity = true;
    }
    TEST_ASSERT(caughtZeroCapacity, "T31: Constructing BookCatalogue with 0 capacity throws std::invalid_argument");

    // 2. Capacity 1 works properly
    BookCatalogue singleSlot(1);
    TEST_ASSERT(singleSlot.getCapacity() == 1, "T31: Single-slot capacity is 1");
    TEST_ASSERT(singleSlot.isEmpty(), "T31: Initially empty");
    TEST_ASSERT(singleSlot.add(Book(1, "Solo Book", "Solo Author")) == Status::Ok, "T31: Added to single slot");
    TEST_ASSERT(singleSlot.isFull(), "T31: Now full");
    TEST_ASSERT(singleSlot.add(Book(2, "Extra Book", "Extra Author")) == Status::Full, "T31: Extra add rejected as Full");
    TEST_ASSERT(singleSlot.remove(1) == Status::Ok, "T31: Single element removed");
    TEST_ASSERT(singleSlot.isEmpty(), "T31: Cleanly empty again");
}

void test_memory_and_traversal() {
    std::cout << "\n=== Running Memory Safety & Traversal Checks ===\n";
    BookCatalogue original(5);
    original.add(Book(1, "T1", "A1"));
    original.add(Book(2, "T2", "A2"));
    original.add(Book(3, "T3", "A3"));

    // 1. Traversal visitor
    int visitCount = 0;
    int idSum = 0;
    original.traverse([&](const Book& b) {
        ++visitCount;
        idSum += b.id;
    });
    TEST_ASSERT(visitCount == 3, "Traversal: Visited all 3 items");
    TEST_ASSERT(idSum == 6, "Traversal: Sum of IDs (1+2+3) == 6");

    // 2. Copy Constructor
    BookCatalogue copy(original);
    TEST_ASSERT(copy.getCount() == 3, "Copy constructor: count matches 3");
    TEST_ASSERT(copy.getCapacity() == 5, "Copy constructor: capacity matches 5");
    TEST_ASSERT(copy.contains(2), "Copy constructor: contains book 2");
    // Verify deep copy: mutate original, copy must not change
    original.remove(2);
    TEST_ASSERT(!original.contains(2), "Deep copy check: book 2 removed from original");
    TEST_ASSERT(copy.contains(2), "Deep copy check: book 2 still safely present in copy");

    // 3. Copy Assignment Operator
    BookCatalogue assigned(2);
    assigned = copy;
    TEST_ASSERT(assigned.getCount() == 3, "Copy assignment: count matches 3");
    TEST_ASSERT(assigned.getCapacity() == 5, "Copy assignment: capacity matches 5");
    TEST_ASSERT(assigned.contains(2), "Copy assignment: contains book 2");

    // 4. Move Constructor
    BookCatalogue moved(std::move(copy));
    TEST_ASSERT(moved.getCount() == 3, "Move constructor: moved count is 3");
    TEST_ASSERT(moved.contains(2), "Move constructor: moved contains book 2");
    TEST_ASSERT(copy.getCount() == 0, "Move constructor: source count cleared to 0");
    TEST_ASSERT(copy.getCapacity() == 0, "Move constructor: source capacity cleared to 0");
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "  SmartLibrary SL-001 BookCatalogue Test Suite     \n";
    std::cout << "====================================================\n";

    test_T01_add_and_search();
    test_T06_search_nonexistent();
    test_T07_invalid_inputs_and_duplicate();
    test_T08_full_catalogue();
    test_T09_delete_first_middle_last();
    test_T21_edit_metadata_and_update_status();
    test_T31_capacity_validation_and_edge_cases();
    test_memory_and_traversal();

    std::cout << "\n====================================================\n";
    std::cout << "Test Summary: " << g_tests_passed << " / " << g_tests_run << " passed.\n";
    std::cout << "====================================================\n";

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}
