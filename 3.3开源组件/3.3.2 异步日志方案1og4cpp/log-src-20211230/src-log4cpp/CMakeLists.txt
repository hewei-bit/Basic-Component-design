cmake_minimum_required(VERSION 3.1)

project (log4cpptest)
# 设置C++标准为 C++ 11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -W -Wall -Wextra -g")
include_directories(/usr/local/include/)
link_directories(/usr/local/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# test1
add_executable(1-file_test  1-file_test.cpp)
target_link_libraries(1-file_test)
# test2
add_executable(2-test_log4cpp  2-test_log4cpp.cpp)
target_link_libraries(2-test_log4cpp log4cpp pthread)
# test3
add_executable(3-test_log4cpp  3-test_log4cpp.cpp)
target_link_libraries(3-test_log4cpp log4cpp pthread)
# test4
add_executable(4-RollingFileAppender  4-RollingFileAppender.cpp)
target_link_libraries(4-RollingFileAppender log4cpp pthread)
# test5
add_executable(5-StringQueueAppender  5-StringQueueAppender.cpp)
target_link_libraries(5-StringQueueAppender log4cpp pthread)