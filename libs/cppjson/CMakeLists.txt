# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目名称
project(cppjson)

# 指定编译flag
set(CMAKE_CXX_FLAGS "-std=c++17 -g")

# 设置宏
add_definitions(-D DEBUG)

add_subdirectory(tests)
