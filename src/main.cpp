#include "ConsoleUI.hpp"
#include "LibraryManager.hpp"

int main() {
    LibraryManager manager;
    ConsoleUI ui(manager);
    ui.run();
    return 0;
}
