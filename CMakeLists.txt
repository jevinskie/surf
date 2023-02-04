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

if (UNIX)
   if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      add_compile_options(-stdlib=libc++)
      add_link_options(-stdlib=libc++ -fuse-ld=lld)
   endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   add_compile_options(-Wno-missing-field-initializers)
endif()

if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
   add_compile_options(-Wno-unused-parameter)
endif()

add_compile_options(-ggdb3)
# add_compile_options(-g0)

add_compile_options(-O0 -fno-omit-frame-pointer)
# add_compile_options(-fno-inline)

# add_compile_options(-Oz -fvisibility=hidden -fvisibility-inlines-hidden -ffunction-sections -fdata-sections)
# add_link_options(-Wl,-dead_strip)

add_compile_options(-O0 -ggdb3 -fsanitize=address)
add_link_options(-O0 -ggdb3 -fsanitize=address)

# add_compile_options(-fsanitize=memory)
# add_link_options(-fsanitize=memory)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS NO)

add_subdirectory(libsurf)
