#pragma once

#include "../engine/tools/termination_manager.h"

#include <exception>
#include <utility>

namespace moonline::application
{
// Executes one application phase without allowing user callbacks to escape the main loop.
template <typename Callable>
bool run_event_boundary(const char* phase,Callable&& callable) noexcept
{
    try
    {
        std::forward<Callable>(callable)();
        return true;
    }
    catch (const std::exception& error)
    {
        elysia::tools::TerminationManager::instance()->request_termination(
            elysia::tools::TerminationReason::UnhandledException,phase,error.what());
    }
    catch (...)
    {
        elysia::tools::TerminationManager::instance()->request_termination(
            elysia::tools::TerminationReason::UnhandledException,phase,"Unhandled non-standard exception");
    }
    return false;
}
}
