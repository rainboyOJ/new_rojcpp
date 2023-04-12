# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目名称
project(modern_cppdb)

set(CPPDB_MAJOR 0)
set(CPPDB_MINOR 1)
set(CPPDB_PATCH 0)
set(CPPDB_VERSION ${CPPDB_MAJOR}.${CPPDB_MINOR}.${CPPDB_PATCH})
set(CPPDB_SOVERSION ${CPPDB_MAJOR})
add_definitions(
        -DCPPDB_MAJOR=${CPPDB_MAJOR}
        -DCPPDB_MINOR=${CPPDB_MINOR}
        -DCPPDB_PATCH=${CPPDB_PATCH}
        -DCPPDB_VERSION="${CPPDB_MAJOR}.${CPPDB_MINOR}.${CPPDB_PATCH}"
    )

# 指定编译flag
set(CMAKE_CXX_FLAGS "-std=c++20 -g")

# 设置宏
add_definitions(-DMODERN_CPPDB_DEBUG)

# 指定include 的头文件的位置
include_directories(include)

execute_process(COMMAND mysql_config --cflags
    OUTPUT_VARIABLE MYSQL_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND mysql_config --libs
    OUTPUT_VARIABLE MYSQL_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

add_executable(example example.cpp)
target_link_libraries(example pthread ${MYSQL_LIBS})
target_compile_options(example PUBLIC ${MYSQL_CFLAGS})

file(GLOB TESTS "tests/*.cpp")
foreach(f ${TESTS})
    string(REPLACE ".cpp" "" testname ${f})
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/tests/ "" name ${testname})
    add_executable(${name} ${f})
    target_link_libraries(${name} pthread ${MYSQL_LIBS})
    target_compile_options(${name} PUBLIC ${MYSQL_CFLAGS})
endforeach(f)

