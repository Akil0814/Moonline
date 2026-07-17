#pragma once

#include <string>

namespace elysia::scene::builtin
{
struct StartupFailureScenePayload
{
    std::string diagnostic_message;
};
}
