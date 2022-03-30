cmake_minimum_required(VERSION 3.0)

project(lock_HW)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_BUILD_TYPE Debug)

add_executable(lock_hw src/practice/lock_hw.c)
TARGET_LINK_LIBRARIES(lock_hw pthread)

add_executable(lock src/example/lock.c)
TARGET_LINK_LIBRARIES(lock pthread)