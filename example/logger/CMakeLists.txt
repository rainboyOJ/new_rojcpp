cmake_minimum_required (VERSION 3.8)
add_executable(test_logStream "test_logStream.cpp")
add_executable(test_logger "test_logger.cpp" "../../libs/logger/logger.cpp")
add_executable(test_AsyncLogger "test_AsyncLogger.cpp" "../../libs/logger/logger.cpp")
