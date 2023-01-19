cmake_minimum_required(VERSION 3.22)

project(surf CXX)

set(BUILD_SHARED_LIBS OFF)

# set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)

add_compile_options(-ggdb3)
# add_compile_options(-g0)

add_compile_options(-O0 -fno-omit-frame-pointer)

# add_compile_options(-Os -fvisibility=hidden -fvisibility-inlines-hidden -ffunction-sections -fdata-sections)
# add_link_options(-Os -Wl,-dead_strip)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS NO)

add_subdirectory(3rdparty)
add_subdirectory(lib)
add_subdirectory(test)
add_subdirectory(tools)
add_subdirectory(unit-tests)
