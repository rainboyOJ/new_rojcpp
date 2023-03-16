cmake_minimum_required (VERSION 3.8)
add_executable(test_phr "test_phr.cpp" "../../src/picohttpparser.c")
message(${CMAKE_CURRENT_BINARY_DIR})
message(${CMAKE_SOURCE_DIR})
add_custom_command(
        TARGET test_phr  POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/picohttparser/header1.txt
                ${CMAKE_CURRENT_BINARY_DIR}/header1.txt)
