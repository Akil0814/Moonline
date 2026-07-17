#pragma once

#include "../../engine/application/game_module.h"

namespace moonline::application
{
class MoonlineGameModule final : public elysia::application::IGameModule
{
public:
    [[nodiscard]] elysia::application::ApplicationDescriptor descriptor() const override;
    void register_scenes(elysia::scene::SceneManager& scene_manager) const override;
};
}
