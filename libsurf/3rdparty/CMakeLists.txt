add_subdirectory(argparse)

option(FMT_DOC OFF)
option(FMT_TEST OFF)
option(FMT_INSTALL OFF)
add_subdirectory(fmt)

add_library(fpng fpng/src/fpng.cpp fpng/src/fpng.h)
target_compile_options(fpng PRIVATE -Wno-tautological-constant-out-of-range-compare)
if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
target_compile_options(fpng PRIVATE -msse4.1 -mpclmul)
endif()

set_target_properties(fpng PROPERTIES PUBLIC_HEADER fpng/src/fpng.h)
target_include_directories(fpng PUBLIC fpng-scoped-include)

add_subdirectory(lexy)
add_subdirectory(magic_enum)

add_library(thread-pool INTERFACE)
target_include_directories(thread-pool INTERFACE thread-pool)
