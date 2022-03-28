cmake_minimum_required(VERSION 3.0)

project(timers_HW)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_BUILD_TYPE Debug)

set(timers_example_minheap "timers_src/example/minheap")
set(timers_example_rbtree "timers_src/example/rbtree")
set(timers_example_skiplist "timers_src/example/skiplist")
set(timers_example_timewheel "timers_src/example/timewheel")
set(timers_example_timewheel_single "timers_src/example/timewheel_single")

include_directories(${CMAKE_SOURCE_DIR}/${timers_example_minheap})
include_directories(${CMAKE_SOURCE_DIR}/${timers_example_rbtree})
include_directories(${CMAKE_SOURCE_DIR}/${timers_example_skiplist})
include_directories(${CMAKE_SOURCE_DIR}/${timers_example_timewheel})
include_directories(${CMAKE_SOURCE_DIR}/${timers_example_timewheel_single})


add_executable(mh ${timers_example_minheap}/minheap.c ${timers_example_minheap}/mh-timer.c)

add_executable(rbt ${timers_example_rbtree}/rbtree.c ${timers_example_rbtree}/rbt-timer.c)

add_executable(skl ${timers_example_skiplist}/skiplist.c ${timers_example_skiplist}/skl-timer.c)

add_executable(stw ${timers_example_timewheel_single}/single_tw.c)

add_executable(tw ${timers_example_timewheel}/timewheel.c ${timers_example_timewheel}/tw-timer.c)
TARGET_LINK_LIBRARIES(tw pthread)