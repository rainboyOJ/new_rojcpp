cmake_minimum_required (VERSION 3.8)

project ("rojcpp")

find_package(Threads REQUIRED)

message("Compiler: ${CMAKE_CXX_COMPILER_ID}")

add_compile_options(-std=c++20 -Wfatal-errors -g -march=native)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
    message( STATUS "No CMAKE_BUILD_TYPE selected, defaulting to ${CMAKE_BUILD_TYPE}")
endif()

string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)

if(build_type MATCHES debug)
    message( STATUS "Debug mode <<<<")
    add_compile_options(-DDEBUG)
elseif(build_type MATCHES release)
    message( STATUS "Release mode <<<<")
endif()


# 得到mysql的lib的配置
execute_process(COMMAND mysql_config --cflags
    OUTPUT_VARIABLE MYSQL_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND mysql_config --libs
    OUTPUT_VARIABLE MYSQL_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

include_directories(include)
include_directories(libs)
include_directories(libs/tinyasync/include)
include_directories(libs/modern_cppdb/include)
include_directories(libs/cppjson/include)
include_directories(libs/judgercpp)

# add_subdirectory("some_fold")

aux_source_directory(src SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST} "./libs/logger/logger.cpp")

# target_link_libraries(test PRIVATE Threads::Threads)

target_link_libraries(${PROJECT_NAME} pthread ${MYSQL_LIBS} hiredis)
target_compile_options(${PROJECT_NAME} PUBLIC ${MYSQL_CFLAGS})


# judgeServer
add_subdirectory(libs/judgercpp/judge_server)
