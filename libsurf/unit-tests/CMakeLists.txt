set(SURF_UNIT_TEST_SRC
    varbit.cpp
)

add_executable(surf-unit-tests ${SURF_UNIT_TEST_SRC})
target_link_libraries(surf-unit-tests PRIVATE surf fmt Catch2 Catch2WithMain)
