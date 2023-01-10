include_directories(../include)

aux_source_directory(. LIB_SRCS)
add_library(src ${LIB_SRCS})
