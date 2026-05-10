file(GLOB_RECURSE GAMEPLAY ./  *.cpp)

add_library(gameplay_lib ${GAMEPLAY}  "../framework/scene/scene_factory.cpp")
