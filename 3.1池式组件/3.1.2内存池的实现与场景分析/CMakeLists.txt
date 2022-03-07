cmake_minimum_required(VERSION 3.0)

project(mmpool_HW)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_BUILD_TYPE Debug)

include_directories(${CMAKE_SOURCE_DIR}/inc)

add_executable(mmpool_hw src/practice/mmpool_hw.c)

add_executable(mmpool src/example/mmpool.c)
