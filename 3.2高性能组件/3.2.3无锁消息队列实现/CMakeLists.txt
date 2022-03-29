cmake_minimum_required(VERSION 3.1)

project (C++11)
# 设置C++标准为 C++ 11
# set(CMAKE_CXX_STANDARD 11)
# 设置C++标准为 C++ 14
set(CMAKE_CXX_STANDARD 14)



set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g")

set(SRC_DIR src/cas_queue)
set(SRC_DIR_HW src/cas_queue_hw)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR_HW})

add_executable(1_test_i++ ${SRC_DIR}/1_test_i++.c)

add_executable(2_test_i++_threads ${SRC_DIR}/2_test_i++_threads.c)
target_link_libraries(2_test_i++_threads pthread)

add_executable(3_test_mutex_spin_lock ${SRC_DIR}/3_test_mutex_spin_lock.cpp)
target_link_libraries(3_test_mutex_spin_lock pthread)

add_executable(4_test_lock_free_queue ${SRC_DIR}/4_test_lock_free_queue.cpp)
target_link_libraries(4_test_lock_free_queue pthread)