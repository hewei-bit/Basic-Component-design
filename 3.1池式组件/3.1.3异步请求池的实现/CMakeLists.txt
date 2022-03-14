cmake_minimum_required(VERSION 3.0)

project(async)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_BUILD_TYPE Debug)

include_directories(${CMAKE_SOURCE_DIR}/inc)

add_executable(async_dns_client_noblock src/async/async_dns_client_noblock.c)
TARGET_LINK_LIBRARIES(async_dns_client_noblock pthread)

add_executable(sync_dns_client src/async/sync_dns_client.c)

add_executable(async_dns_client_noblock_hw src/practice/async_dns_client_noblock_hw.c)
TARGET_LINK_LIBRARIES(async_dns_client_noblock_hw pthread)

add_executable(sync_dns_client_hw src/practice/sync_dns_client_hw.c)
