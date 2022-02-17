cmake_minimum_required(VERSION 3.0)

project(thread_pool_HW)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_BUILD_TYPE Debug)

include_directories(${CMAKE_SOURCE_DIR}/inc)



add_executable(thread_pool src/example/thread_pool.c)
TARGET_LINK_LIBRARIES(thread_pool pthread)