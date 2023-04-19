# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目名称
project(judgeServer)

# 指定编译flag
set(CMAKE_CXX_FLAGS "-std=c++17")

# 选项
option(JUDGE_SERVER_DEBUG "是否开启debug" OFF)
if(JUDGE_SERVER_DEBUG)
    add_definitions(-DJUDGE_SERVER_DEBUG)
endif(JUDGE_SERVER_DEBUG)



# 设置宏
#add_definitions(-D DEBUG)

# 指定include 的头文件的位置
#include_directories(src/include)

# 添加代码
include_directories(src)
#include_directories(lib)
aux_source_directory(src DIR_SRCS)

if(EXISTS ${JUDGE_SERVER_USER_CONFIG_PATH})
    message(STATUS "JUDGE_SERVER 使用用户的配置文件 ${JUDGE_SERVER_USER_CONFIG_PATH}")
    add_definitions(-D USE_JUDGE_SERVER_USER_CONFIG)
    add_compile_options(-include ${JUDGE_SERVER_USER_CONFIG_PATH})
else()
    message(STATUS "使用默认配置文件 ${PROJECT_SOURCE_DIR}/DEFAULT_CONFIG.hpp")
    add_compile_options(-include ${PROJECT_SOURCE_DIR}/DEFAULT_CONFIG.hpp)
endif(EXISTS ${JUDGE_SERVER_USER_CONFIG_PATH})


# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
add_executable(server server.cpp ${DIR_SRCS})
add_executable(client client.cpp ${DIR_SRCS})

