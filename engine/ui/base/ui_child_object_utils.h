#pragma once

#include "../../core/game_object.h"
#include "../../input/input_types.h"

#include <SDL.h>

#include <memory>
#include <vector>

namespace ui_child_object_utils
{
    bool remove_destroyed_children(std::vector<std::shared_ptr<GameObject>>& children);

    void update_children(const std::vector<std::shared_ptr<GameObject>>& children, double delta);
    void render_children(const std::vector<std::shared_ptr<GameObject>>& children, SDL_Renderer* renderer);
    void input_children(const std::vector<std::shared_ptr<GameObject>>& children, const InputSnapshot& input);
    void input_event_children(const std::vector<std::shared_ptr<GameObject>>& children, const InputEvent& event);
    void reset_children(const std::vector<std::shared_ptr<GameObject>>& children);
}
