cmake_minimum_required (VERSION 3.8)

project ("rojcpp")

find_package(Threads REQUIRED)

message("Compiler: ${CMAKE_CXX_COMPILER_ID}")

add_compile_options(-std=c++20 -Wfatal-errors -g -march=native)

include_directories(../include)
include_directories(../libs)
include_directories(../libs/tinyasync/include)

add_subdirectory("gzip")
add_subdirectory("tools")
add_subdirectory("all_headers")
add_subdirectory("buffers")


# target_link_libraries(test PRIVATE Threads::Threads)
