add_subdirectory(argparse)

option(FMT_DOC OFF)
option(FMT_TEST OFF)
option(FMT_INSTALL OFF)
add_subdirectory(fmt)

add_library(thread-pool INTERFACE)
target_include_directories(thread-pool INTERFACE thread-pool)
