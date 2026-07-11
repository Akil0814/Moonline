add_executable(ui_lifecycle_tests
    ui_lifecycle_tests.cpp
)

target_link_libraries(ui_lifecycle_tests PRIVATE engine_lib ${SDL2_LIBRARIES})

if(WIN32)
    foreach(SDL2_RUNTIME_FILE IN LISTS SDL2_RUNTIME_FILES)
        if(EXISTS "${SDL2_RUNTIME_FILE}")
            add_custom_command(TARGET ui_lifecycle_tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${SDL2_RUNTIME_FILE}"
                    "$<TARGET_FILE_DIR:ui_lifecycle_tests>"
            )
        endif()
    endforeach()
endif()

add_test(NAME ui_lifecycle_tests COMMAND ui_lifecycle_tests)
