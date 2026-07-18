#pragma once

#include "../routing/scene_route.h"

#include <string>
#include <utility>

namespace elysia::scene::builtin
{
enum class ApplicationFailurePresentation
{
    StartupLoading,
    RuntimeFatal
};

struct ApplicationFailureScenePayload
{
    ApplicationFailurePresentation presentation =
        ApplicationFailurePresentation::RuntimeFatal;
    std::string category;
    std::string diagnostic_message;
};

[[nodiscard]] inline SceneRoute make_application_failure_route(
    ApplicationFailurePresentation presentation,
    std::string category,
    std::string diagnostic_message)
{
    return SceneRoute{
        .target = ApplicationFailure,
        .payload = ApplicationFailureScenePayload{
            .presentation = presentation,
            .category = std::move(category),
            .diagnostic_message = std::move(diagnostic_message)
        },
        .reload_mode = SceneReloadMode::Reuse
    };
}
}
