include_directories(../include)
include_directories(../../third/proto)

aux_source_directory(. LIB_SRCS)
add_library(core ${LIB_SRCS})
