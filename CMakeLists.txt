cmake_minimum_required (VERSION 3.8)

project ("rojcpp")

find_package(Threads REQUIRED)

message("Compiler: ${CMAKE_CXX_COMPILER_ID}")

add_compile_options(-std=c++20 -Wfatal-errors -g -march=native)

include_directories(include)
include_directories(libs)
include_directories(libs/tinyasync/include)

# add_subdirectory("some_fold")

aux_source_directory(src SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

# target_link_libraries(test PRIVATE Threads::Threads)
