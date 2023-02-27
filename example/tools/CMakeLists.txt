cmake_minimum_required (VERSION 3.8)
aux_source_directory(. CPP_LIST)

foreach(cpp ${CPP_LIST})
    get_filename_component(name ${cpp} NAME_WE)
    add_executable(${name} ${cpp})
endforeach(cpp ${CPP_LIST})


# add_compile_options(-std=c++20 -Wfatal-errors -g -march=native)

# include_directories(../include)

# add_subdirectory("some_fold")

# add_executable(test "test.cpp")

# target_link_libraries(test PRIVATE Threads::Threads)
