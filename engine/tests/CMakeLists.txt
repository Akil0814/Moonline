add_moonline_test(engine_i18n_assets_tests
    LABEL localization
    SOURCES localization/engine_i18n_assets_tests.cpp
)
target_compile_definitions(engine_i18n_assets_tests
    PRIVATE MOONLINE_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)

add_moonline_test(localization_engine_assist_fallback_tests
    LABEL localization
    SOURCES localization/localization_engine_assist_fallback_tests.cpp
    LIBRARIES ${SDL2_LIBRARIES}
    SDL
)
target_compile_definitions(localization_engine_assist_fallback_tests
    PRIVATE MOONLINE_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)

add_moonline_test(engine_assist_catalog_tests
    LABEL bootstrap
    SOURCES assist/engine_assist_catalog_tests.cpp
)
target_compile_definitions(engine_assist_catalog_tests
    PRIVATE MOONLINE_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)

add_moonline_test(engine_assist_cache_tests
    LABEL bootstrap
    SOURCES assist/engine_assist_cache_tests.cpp
    LIBRARIES ${SDL2_LIBRARIES}
    SDL
)
target_compile_definitions(engine_assist_cache_tests
    PRIVATE MOONLINE_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)
