cmake_minimum_required(VERSION 3.17)
enable_testing()
project(main)
# have binaries next to resource files. DIRTY but works.
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
# generate compile_commands.json
add_custom_target(
    copy-compile-commands ALL
    ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Catch2
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/deps/Catch2)
add_library(Catch2::Catch2 IMPORTED INTERFACE)
target_include_directories(Catch2::Catch2 INTERFACE ${CATCH_INCLUDE_DIR})
#set_property(Catch2::Catch2 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CATCH_INCLUDE_DIR}")
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/deps/Catch2/contrib)
include(Catch)

add_library(orgparse orgparse.cpp)
set_property(TARGET orgparse PROPERTY CXX_STANDARD 20)
add_executable(main main.cpp)
set_property(TARGET main PROPERTY CXX_STANDARD 20)
target_link_libraries(main orgparse)
add_executable(main_test test.cpp catch.cpp)
set_property(TARGET main_test PROPERTY CXX_STANDARD 20)
target_link_libraries(main_test Catch2::Catch2 orgparse)
catch_discover_tests(main_test)
