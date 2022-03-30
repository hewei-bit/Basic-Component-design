cmake_minimum_required (VERSION 2.8.7)
project (log_test)

###
# compilation options
###
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -W -Wall -Wextra -g")


###
# variables
###
set(PROJECT_ROOT_DIR ${PROJECT_SOURCE_DIR})


###
# includes
###
include_directories(.
         ${INCLUDES_DIRECTORY})
aux_source_directory(. LOG_TEST_SOURCES_DIRECTORY)


#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_ROOT_DIR}/bin)

 
###
# executable
###
add_executable(main_log_test ${LOG_TEST_SOURCES_DIRECTORY})


###
# link librarys
###
target_link_libraries(main_log_test pthread)