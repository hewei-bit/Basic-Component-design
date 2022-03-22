cmake_minimum_required(VERSION 2.8)
PROJECT(test_CachePool)

SET(SRC_LIST
    async.c 
    dict.c
    hiredis.c
    net.c
    read.c
    sds.c
    CachePool.cpp
)

SET(EXECUTABLE_OUTPUT_PATH  ./)
INCLUDE_DIRECTORIES(./)

ADD_EXECUTABLE(test_CachePool test_CachePool.cpp ${SRC_LIST})
ADD_EXECUTABLE(test_ThreadPool test_ThreadPool.cpp ${SRC_LIST})

TARGET_LINK_LIBRARIES(test_CachePool pthread) 
TARGET_LINK_LIBRARIES(test_ThreadPool pthread) 