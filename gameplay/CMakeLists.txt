file(GLOB_RECURSE GAMEPLAY CONFIGURE_DEPENDS "*.cpp")

add_library(gameplay_lib ${GAMEPLAY})

target_include_directories(gameplay_lib PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(gameplay_lib PUBLIC engine_lib)
