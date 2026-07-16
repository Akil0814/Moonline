include(CMakeParseArguments)

function(add_moonline_test TARGET_NAME)
    set(options SDL)
    set(one_value_args LABEL)
    set(multi_value_args SOURCES LIBRARIES)
    cmake_parse_arguments(TEST "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    add_executable(${TARGET_NAME} ${TEST_SOURCES})
    target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR})
    target_link_libraries(${TARGET_NAME} PRIVATE engine_lib ${TEST_LIBRARIES})

    if(TEST_SDL AND WIN32)
        foreach(SDL2_RUNTIME_FILE IN LISTS SDL2_RUNTIME_FILES)
            if(EXISTS "${SDL2_RUNTIME_FILE}")
                add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${SDL2_RUNTIME_FILE}"
                        "$<TARGET_FILE_DIR:${TARGET_NAME}>"
                )
            endif()
        endforeach()
    endif()

    add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
    if(TEST_LABEL)
        set_tests_properties(${TARGET_NAME} PROPERTIES LABELS "${TEST_LABEL}")
    endif()
endfunction()

find_package(Threads REQUIRED)

add_moonline_test(glyph_run_layout_tests
    LABEL core
    SOURCES core/glyph_run_layout_tests.cpp
)

foreach(INPUT_TEST_TARGET IN ITEMS
    input_action_mapping_tests
    gameplay_support_tests
    gameplay_scene_input_tests
)
    add_moonline_test(${INPUT_TEST_TARGET}
        LABEL input
        SOURCES input/${INPUT_TEST_TARGET}.cpp
        LIBRARIES ${SDL2_LIBRARIES}
        SDL
    )
endforeach()

add_moonline_test(termination_manager_tests
    LABEL application
    SOURCES application/termination_manager_tests.cpp
    LIBRARIES Threads::Threads
)

add_moonline_test(camera_manager_tests
    LABEL camera
    SOURCES camera/camera_manager_tests.cpp
    LIBRARIES ${SDL2_LIBRARIES}
    SDL
)

add_moonline_test(physics_framework_contract_tests
    LABEL physics
    SOURCES physics/physics_framework_contract_tests.cpp
)

add_moonline_test(sound_playback_scheduler_tests
    LABEL audio
    SOURCES audio/sound_playback_scheduler_tests.cpp
    LIBRARIES ${SDL2_LIBRARIES}
    SDL
)

add_moonline_test(animation_effect_tests
    LABEL effects
    SOURCES effects/animation_effect_tests.cpp
    LIBRARIES ${SDL2_LIBRARIES}
    SDL
)

add_moonline_test(floating_number_glyph_cache_tests
    LABEL effects
    SOURCES effects/floating_number_glyph_cache_tests.cpp
    LIBRARIES ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES}
    SDL
)

add_moonline_test(floating_number_effect_tests
    LABEL effects
    SOURCES effects/floating_number_effect_tests.cpp
    LIBRARIES ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES}
    SDL
)

foreach(LOADING_TEST_TARGET IN ITEMS
	animation_config_loader_tests
    atlas_build_preparer_tests
    content_load_pipeline_tests
    effect_definition_config_loader_tests
    horizontal_strip_animation_tests
	game_content_loader_config_tests
    resource_key_builder_tests
    resource_load_plan_validator_tests
    resource_request_assembler_tests
)
    add_moonline_test(${LOADING_TEST_TARGET}
        LABEL loading
        SOURCES loading/${LOADING_TEST_TARGET}.cpp
        LIBRARIES ${SDL2_LIBRARIES}
        SDL
    )
endforeach()

foreach(UI_TEST_TARGET IN ITEMS
    ui_layout_tests
    ui_style_tests
    ui_popup_lifecycle_tests
    ui_selection_controls_tests
    ui_focus_lifecycle_tests
    ui_focus_tree_tests
    ui_focus_routing_tests
    ui_presentation_tests
    ui_callback_safety_tests
    ui_text_input_rendering_tests
    ui_number_rendering_tests
)
    add_moonline_test(${UI_TEST_TARGET}
        LABEL ui
        SOURCES ui/${UI_TEST_TARGET}.cpp
        LIBRARIES ${SDL2_LIBRARIES}
        SDL
    )
endforeach()

add_moonline_test(logging_tests
    LABEL tools
    SOURCES tools/logging_tests.cpp
    LIBRARIES ${SDL2_LIBRARIES}
    SDL
)

foreach(CONFIG_TEST_TARGET IN ITEMS app_config_tests user_config_tests config_service_tests bootstrap_config_integration_tests)
    add_moonline_test(${CONFIG_TEST_TARGET}
        LABEL config
        SOURCES config/${CONFIG_TEST_TARGET}.cpp
        LIBRARIES ${SDL2_LIBRARIES}
        SDL
    )
endforeach()
