
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}) 

include_directories(../src)
file(GLOB TESTS "*.cpp")
foreach(f ${TESTS})
    string(REPLACE ".cpp" "" testname ${f})
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/ "" name ${testname})
    add_executable(${name} ${f})
    #target_link_libraries(${name} netcore pthread ${MYSQL_LIBS})
    #target_compile_options(${name} PUBLIC ${MYSQL_CFLAGS})
endforeach(f)
