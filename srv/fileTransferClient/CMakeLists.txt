project(MClient)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)

set(MODULE_MAIN FileTransferClient)

add_definitions("-g")
# add_definitions("-D_DEBUG_LOG_MSG")
add_definitions("-D_DEBUG_ERR_MSG")

include_directories(../../core/include)
include_directories(../../third/proto)
include_directories(../../common)
include_directories(include)

# aux_source_directory(. ALL_SRCS)
# add_subdirectory(../../core/src ${CMAKE_CURRENT_BINARY_DIR}/core)
# add_subdirectory(../../common ${CMAKE_CURRENT_BINARY_DIR}/common)
add_subdirectory(../../third ${CMAKE_CURRENT_BINARY_DIR}/third)
# add_subdirectory(src)

# set(LIBS core common third src protobuf)
set(LIBS third protobuf)

file(GLOB_RECURSE SRC_FILES
    main.cpp
    src/*.cpp
    ../../core/src/*.cpp
    ../../common/*.cpp
)

add_executable(${MODULE_MAIN} ${SRC_FILES})

target_link_libraries(${MODULE_MAIN} ${LIBS})