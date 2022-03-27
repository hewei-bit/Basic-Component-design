cmake_minimum_required(VERSION 3.0)

project(lock_HW)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_BUILD_TYPE Debug)

# include_directories(${CMAKE_SOURCE_DIR}/inc)

# add_executable(mmpool_hw src/practice/mmpool_hw.c)
# TARGET_LINK_LIBRARIES(mmpool_hw pthread)

add_executable(lock src/example/lock.c)
TARGET_LINK_LIBRARIES(lock pthread)