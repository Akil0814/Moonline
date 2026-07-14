add_executable(ui_lifecycle_tests
    "ui_lifecycle_tests.cpp"
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

add_executable(ui_focus_container_tests
    "ui_focus_container_tests.cpp"
)

target_link_libraries(ui_focus_container_tests PRIVATE engine_lib ${SDL2_LIBRARIES})

if(WIN32)
    foreach(SDL2_RUNTIME_FILE IN LISTS SDL2_RUNTIME_FILES)
        if(EXISTS "${SDL2_RUNTIME_FILE}")
            add_custom_command(TARGET ui_focus_container_tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${SDL2_RUNTIME_FILE}"
                    "$<TARGET_FILE_DIR:ui_focus_container_tests>"
            )
        endif()
    endforeach()
endif()

add_test(NAME ui_focus_container_tests COMMAND ui_focus_container_tests)

add_executable(ui_focus_routing_tests
    "ui_focus_routing_tests.cpp"
)

target_link_libraries(ui_focus_routing_tests PRIVATE engine_lib ${SDL2_LIBRARIES})

if(WIN32)
    foreach(SDL2_RUNTIME_FILE IN LISTS SDL2_RUNTIME_FILES)
        if(EXISTS "${SDL2_RUNTIME_FILE}")
            add_custom_command(TARGET ui_focus_routing_tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${SDL2_RUNTIME_FILE}"
                    "$<TARGET_FILE_DIR:ui_focus_routing_tests>"
            )
        endif()
    endforeach()
endif()

add_test(NAME ui_focus_routing_tests COMMAND ui_focus_routing_tests)

find_package(Threads REQUIRED)

add_executable(termination_manager_tests
    "termination_manager_tests.cpp"
)

target_link_libraries(termination_manager_tests PRIVATE engine_lib Threads::Threads)

add_test(NAME termination_manager_tests COMMAND termination_manager_tests)

add_executable(animation_manifest_tests
    "animation_manifest_tests.cpp"
)

target_link_libraries(animation_manifest_tests PRIVATE engine_lib ${SDL2_LIBRARIES})

if(WIN32)
    foreach(SDL2_RUNTIME_FILE IN LISTS SDL2_RUNTIME_FILES)
        if(EXISTS "${SDL2_RUNTIME_FILE}")
            add_custom_command(TARGET animation_manifest_tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${SDL2_RUNTIME_FILE}"
                    "$<TARGET_FILE_DIR:animation_manifest_tests>"
            )
        endif()
    endforeach()
endif()

add_test(NAME animation_manifest_tests COMMAND animation_manifest_tests)

add_executable(animation_effect_tests
    "animation_effect_tests.cpp"
)

target_link_libraries(animation_effect_tests PRIVATE engine_lib ${SDL2_LIBRARIES})

if(WIN32)
    foreach(SDL2_RUNTIME_FILE IN LISTS SDL2_RUNTIME_FILES)
        if(EXISTS "${SDL2_RUNTIME_FILE}")
            add_custom_command(TARGET animation_effect_tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${SDL2_RUNTIME_FILE}"
                    "$<TARGET_FILE_DIR:animation_effect_tests>"
            )
        endif()
    endforeach()
endif()

add_test(NAME animation_effect_tests COMMAND animation_effect_tests)

add_executable(effect_digit_cache_tests
    "effect_digit_cache_tests.cpp"
)

target_link_libraries(effect_digit_cache_tests PRIVATE engine_lib ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})

if(WIN32)
    foreach(SDL2_RUNTIME_FILE IN LISTS SDL2_RUNTIME_FILES)
        if(EXISTS "${SDL2_RUNTIME_FILE}")
            add_custom_command(TARGET effect_digit_cache_tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${SDL2_RUNTIME_FILE}"
                    "$<TARGET_FILE_DIR:effect_digit_cache_tests>"
            )
        endif()
    endforeach()
endif()

add_test(NAME effect_digit_cache_tests COMMAND effect_digit_cache_tests)

add_executable(sound_playback_scheduler_tests
    "sound_playback_scheduler_tests.cpp"
)

target_link_libraries(sound_playback_scheduler_tests PRIVATE engine_lib ${SDL2_LIBRARIES})

if(WIN32)
    foreach(SDL2_RUNTIME_FILE IN LISTS SDL2_RUNTIME_FILES)
        if(EXISTS "${SDL2_RUNTIME_FILE}")
            add_custom_command(TARGET sound_playback_scheduler_tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${SDL2_RUNTIME_FILE}"
                    "$<TARGET_FILE_DIR:sound_playback_scheduler_tests>"
            )
        endif()
    endforeach()
endif()

add_test(NAME sound_playback_scheduler_tests COMMAND sound_playback_scheduler_tests)
