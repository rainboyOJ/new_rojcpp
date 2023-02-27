cmake_minimum_required (VERSION 3.8)
add_executable(main "gzip.cpp")
target_link_libraries(main z)
