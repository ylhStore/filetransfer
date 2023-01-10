project(MServer)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)

set(MODULE_MAIN FileTransferServer)

add_definitions("-g")
#add_definitions("-D_DEBUG_LOG_MSG")
add_definitions("-D_DEBUG_ERR_MSG")

include_directories(../../core/include)
include_directories(../../third/proto)
include_directories(../../common)
include_directories(include)


aux_source_directory(. ALL_SRCS)
add_subdirectory(../../core/src ${CMAKE_CURRENT_BINARY_DIR}/core)
add_subdirectory(../../common ${CMAKE_CURRENT_BINARY_DIR}/common)
add_subdirectory(../../third ${CMAKE_CURRENT_BINARY_DIR}/third)
add_subdirectory(src)

set(LIBS core common third src protobuf)

add_executable(${MODULE_MAIN} ${ALL_SRCS})

target_link_libraries(${MODULE_MAIN} ${LIBS})