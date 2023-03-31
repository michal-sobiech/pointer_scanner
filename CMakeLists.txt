cmake_minimum_required(VERSION 3.0.0)
project(pointer_scanner VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(pointer_scanner main.cpp PointerScanner.cpp PointerScanner.hpp ProcessManager.cpp ProcessManager.hpp UI.cpp UI.hpp)
target_link_libraries(pointer_scanner user32 psapi) # iostream exception vector chrono bits/stdc++)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)