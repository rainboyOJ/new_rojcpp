cmake_minimum_required(VERSION 3.8)
project("test")
find_package(Threads REQUIRED)

add_compile_options(-std=c++20 -Wfatal-errors -g -march=native)
include_directories(../include)
include_directories(../libs)
include_directories(../libs)
include_directories(../libs/modern_cppdb/include)
include_directories(../libs/cppjson/include)
include_directories(../libs/tinyasync/include)

# 得到真的project dir
get_filename_component(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)

# 得到所有的源文件
aux_source_directory(${CMAKE_SOURCE_DIR}/src SRC_LIST)

# 删除src/main.cpp

list(REMOVE_ITEM SRC_LIST ${CMAKE_SOURCE_DIR}/src/main.cpp)

# 添加logger.cpp
list(APPEND SRC_LIST ${CMAKE_SOURCE_DIR}/libs/logger/logger.cpp)
# message("src: ", ${SRC_LIST})


add_subdirectory("judgeServerMediator")
