add_custom_command(OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/vcd-lexer.cpp
    COMMAND /opt/homebrew/opt/re2c/bin/re2c ${CMAKE_CURRENT_SOURCE_DIR}/vcd-lexer.re -o ${CMAKE_CURRENT_BINARY_DIR}/vcd-lexer.cpp --case-ranges  --debug-output -i -Werror
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vcd-lexer.re
)

set(SURF_SRC
    mmap.cpp
    render.cpp
    time.cpp
    trace.cpp
    vcd.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/vcd-lexer.cpp
    vcd-parser.cpp
    utils.cpp
)

set(SURF_HDR_STANDALONE
    surf.h
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
)

set(SURF_PRIVATE_LIBS
    fmt
    fpng
    lexy
    thread-pool
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
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/vcd-lexer.cpp PROPERTIES COMPILE_FLAGS -Wno-gnu-case-range)

target_include_directories(surf
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS surf
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/surf
)