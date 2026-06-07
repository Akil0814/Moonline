#pragma once

#include "scene_key.h"
#include "scene_payload.h"

enum class SceneRequestType
{
    None,
    Switch,
    Quit
};

enum class SceneReloadMode
{
    None,
    Reset,
    Recreate
};

struct SceneRequest
{
    SceneRequestType type = SceneRequestType::None;
    SceneKey target = SceneKeys::Invalid;
    SceneReloadMode reload_mode = SceneReloadMode::None;

    ScenePayload payload{};
};
