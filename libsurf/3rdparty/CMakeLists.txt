add_subdirectory(argparse EXCLUDE_FROM_ALL)

add_subdirectory(Catch2 EXCLUDE_FROM_ALL)

option(FMT_DOC OFF)
option(FMT_TEST OFF)
option(FMT_INSTALL OFF)
add_subdirectory(fmt EXCLUDE_FROM_ALL)

add_library(fpng fpng/src/fpng.cpp fpng/src/fpng.h)
target_compile_options(fpng PRIVATE -Wno-tautological-constant-out-of-range-compare)
if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    target_compile_options(fpng PRIVATE -msse4.1 -mpclmul)
endif()

set_target_properties(fpng PROPERTIES PUBLIC_HEADER fpng-scoped-include/fpng.h)
target_include_directories(fpng PUBLIC fpng-scoped-include)


add_library(hedley INTERFACE)
target_include_directories(hedley INTERFACE hedley-scoped-include)

add_subdirectory(lexy EXCLUDE_FROM_ALL)
add_subdirectory(magic_enum EXCLUDE_FROM_ALL)

add_library(mz_tagged_ptr INTERFACE)
target_include_directories(mz_tagged_ptr INTERFACE tagged_ptr/include)
add_library(mz::tagged_ptr ALIAS mz_tagged_ptr)

add_library(thread-pool INTERFACE)
target_include_directories(thread-pool INTERFACE thread-pool)

add_subdirectory(visit EXCLUDE_FROM_ALL)

add_subdirectory(libassert EXCLUDE_FROM_ALL)