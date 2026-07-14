#pragma once

#include "../../engine/tools/termination_manager.h"

#include <source_location>

namespace arcneco::scene
{
inline void request_startup_content_load_termination(
    std::source_location location = std::source_location::current()
) noexcept
{
    elysia::tools::TerminationManager::instance()->request_termination(
        elysia::tools::TerminationReason::FatalRuntimeFailure,
        "startup",
        "Startup content loading failed",
        location);
}
}
