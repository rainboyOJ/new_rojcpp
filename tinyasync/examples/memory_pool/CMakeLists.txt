cmake_minimum_required (VERSION 3.8)

add_definitions(-D__USE_ASYNC_UTILS__)
add_executable(memory_pool "memory_pool.cpp")
add_executable(PoolResource "PoolResource.cpp")
add_executable(test_pmr_vector.cpp "test_pmr_vector.cpp")

target_link_libraries(memory_pool PRIVATE Threads::Threads)
