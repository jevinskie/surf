set(SURF_SRC
    mmap.cpp
    render.cpp
    time.cpp
    trace.cpp
    varbit.cpp
    vcd.cpp
    vcd-parser.cpp
    utils.cpp
)

set(SURF_HDR_STANDALONE
    surf.h
    tagged_ptr.h
)

set(SURF_HDR_PRIVATE
    common-internal.h
)

set(SURF_HDR)
foreach(HDR ${SURF_HDR_STANDALONE})
    set(HDR "${CMAKE_CURRENT_SOURCE_DIR}/../include/surf/${HDR}")
    list(APPEND SURF_HDR ${HDR})
endforeach()

foreach(SRC ${SURF_SRC})
    get_filename_component(HDR_NAME ${SRC} NAME_WLE)
    set(HDR "${CMAKE_CURRENT_SOURCE_DIR}/../include/surf/${HDR_NAME}.h")
    if(EXISTS ${HDR})
        list(APPEND SURF_HDR ${HDR})
    endif()
    set(HDR "${CMAKE_CURRENT_SOURCE_DIR}/${HDR_NAME}.h")
    if(EXISTS ${HDR})
        list(APPEND SURF_HDR_PRIVATE ${HDR})
    endif()
endforeach()

set(SURF_PUBLIC_LIBS
    fmt
    hedley
    magic_enum
    mz::tagged_ptr
)

set(SURF_PRIVATE_LIBS
    fmt
    fpng
    foonathan::lexy
    thread-pool
    visit
)

# SURF_HDR added for Xcode project generation
add_library(surf ${SURF_SRC} ${SURF_HDR} ${SURF_HDR_PRIVATE})
set_target_properties(surf PROPERTIES PUBLIC_HEADER "${SURF_HDR}")

target_link_libraries(surf
PUBLIC
    ${SURF_PUBLIC_LIBS}
PRIVATE
    ${SURF_PRIVATE_LIBS}
)
target_compile_options(surf PRIVATE -Wall -Wextra -Wpedantic)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(surf PRIVATE -Wno-missing-field-initializers)
endif()

if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
    target_compile_options(surf PRIVATE
        # -Wno-unused-parameter
    )
endif()

target_include_directories(surf
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS surf
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/surf
)

add_library(surf_dump_types dump_types.cpp)
target_link_libraries(surf_dump_types surf)
target_compile_options(surf_dump_types PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(surf_dump_types
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
