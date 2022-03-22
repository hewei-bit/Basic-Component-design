cmake_minimum_required(VERSION 3.0)

project(connect_pool_HW)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_BUILD_TYPE Debug)

include_directories(${CMAKE_SOURCE_DIR}/src/redis-async)
include_directories(${CMAKE_SOURCE_DIR}/src/redis-async-hw)
include_directories(${CMAKE_SOURCE_DIR}/hiredis)

# add_executable(sync_hw src/practice/sync_hw.c)
# TARGET_LINK_LIBRARIES(sync_hw pthread)

# 并将名称保存到 DIR_LIB_SRCS 变量
AUX_SOURCE_DIRECTORY(./hiredis DIR_LIB_SRCS)
# 生成静态库链接库Dir1
ADD_LIBRARY (hiredis ${DIR_LIB_SRCS})

add_executable(sync src/redis-async/sync.c)
TARGET_LINK_LIBRARIES(sync hiredis)

add_executable(async src/redis-async/async.c)
TARGET_LINK_LIBRARIES(async hiredis)

add_executable(sync_hw src/redis-async-hw/sync.c)
TARGET_LINK_LIBRARIES(sync hiredis)

add_executable(async_hw src/redis-async-hw/async.c)
TARGET_LINK_LIBRARIES(async hiredis)