cmake_minimum_required(VERSION 3.0)

project(DEADLOCK_HW)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_BUILD_TYPE Debug)

include_directories(${CMAKE_SOURCE_DIR}/inc)

add_executable(deadlock_success src/practice/deadlock_success.c)
TARGET_LINK_LIBRARIES(deadlock_hw pthread)

add_executable(deadlock src/example/deadlockøŒ…œ ÷–¥.c)
TARGET_LINK_LIBRARIES(deadlock pthread)