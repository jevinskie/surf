cmake_minimum_required(VERSION 3.22)

project(surf CXX)

option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." ON)

if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
       add_compile_options(-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
       add_compile_options(-fcolor-diagnostics)
    endif ()
endif ()

set(BUILD_SHARED_LIBS OFF)

# set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)

# To enable building with upstream LLVM/Clang that doesn't support -index-store-path option
# set(CMAKE_XCODE_ATTRIBUTE_COMPILER_INDEX_STORE_ENABLE NO)

add_compile_options(-ggdb3)
# add_compile_options(-g0)

add_compile_options(-O0 -fno-omit-frame-pointer)
# add_compile_options(-fno-inline)

# add_compile_options(-Os -fvisibility=hidden -fvisibility-inlines-hidden -ffunction-sections -fdata-sections)
# add_link_options(-Os -Wl,-dead_strip)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS NO)

add_subdirectory(libsurf)
