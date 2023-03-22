cmake_minimum_required (VERSION 3.8)

add_definitions(-D__USE_ASYNC_UTILS__)
add_executable(echo_server "echo_server.cpp")
add_executable(echo_server_timeout "echo_server_timeout.cpp")


target_link_libraries(echo_server PRIVATE Threads::Threads)
target_link_libraries(echo_server_timeout PRIVATE Threads::Threads)
