#pragma once

#include "scene_payload.h"

enum class SceneRequestType
{
    None,
    Switch,
    Quit
};

enum class SceneId
{
    None,
    StartupLoading,
    MainMenu,
    Game
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
    SceneId target = SceneId::None;
    SceneReloadMode reload_mode = SceneReloadMode::None;

    ScenePayload payload{};
};