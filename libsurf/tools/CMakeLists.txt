add_executable(surf-tool surf-tool.cpp)
target_compile_options(surf-tool PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(surf-tool PRIVATE argparse fmt surf libassert::assert)

install(TARGETS surf-tool
    RUNTIME DESTINATION bin
)
