cmake_minimum_required(VERSION 3.16)
project(SmartLibrary LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(MINGW)
        add_link_options(-static)
    endif()
endif()

include_directories(include)

# Static library for pure core data structures and manager (UI-agnostic)
add_library(smartlibrary_core STATIC
    src/BookCatalogue.cpp
    src/HistoryList.cpp
    src/ReturnStack.cpp
    src/RequestQueue.cpp
    src/LibraryManager.cpp
)
target_include_directories(smartlibrary_core PUBLIC include)

# Console UI library & executable
add_library(smartlibrary_console_ui STATIC
    src/ConsoleUI.cpp
)
target_link_libraries(smartlibrary_console_ui PUBLIC smartlibrary_core)

add_executable(smartlibrary src/main.cpp)
target_link_libraries(smartlibrary PRIVATE smartlibrary_console_ui)

# Optional GUI HTTP/JSON adapter & local loopback server (SL-007)
add_library(smartlibrary_web_adapter STATIC
    src/JsonApiAdapter.cpp
    src/HttpServer.cpp
)
target_link_libraries(smartlibrary_web_adapter PUBLIC smartlibrary_core)
if(WIN32)
    target_link_libraries(smartlibrary_web_adapter PUBLIC ws2_32)
endif()

add_executable(smartlibrary_gui src/gui_main.cpp)
target_link_libraries(smartlibrary_gui PRIVATE smartlibrary_web_adapter)
target_compile_definitions(smartlibrary_gui PRIVATE SMARTLIBRARY_WEB_DIR="${CMAKE_CURRENT_SOURCE_DIR}/web")

# Test suite executables
enable_testing()

add_executable(catalogue_tests tests/test_catalogue.cpp)
target_link_libraries(catalogue_tests PRIVATE smartlibrary_core)
add_test(NAME CatalogueTests COMMAND catalogue_tests)

add_executable(structure_tests tests/test_structures.cpp)
target_link_libraries(structure_tests PRIVATE smartlibrary_core)
add_test(NAME StructureTests COMMAND structure_tests)

add_executable(manager_tests tests/test_manager.cpp)
target_link_libraries(manager_tests PRIVATE smartlibrary_core)
add_test(NAME ManagerTests COMMAND manager_tests)

add_executable(console_tests tests/test_console.cpp)
target_link_libraries(console_tests PRIVATE smartlibrary_console_ui)
add_test(NAME ConsoleTests COMMAND console_tests)

add_executable(gui_parity_tests tests/test_gui_parity.cpp)
target_link_libraries(gui_parity_tests PRIVATE smartlibrary_web_adapter)
add_test(NAME GuiParityTests COMMAND gui_parity_tests)
