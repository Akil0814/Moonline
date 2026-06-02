#include "ui_child_object_utils.h"

#include <algorithm>

namespace ui_child_object_utils
{
    bool remove_destroyed_children(std::vector<std::shared_ptr<GameObject>>& children)
    {
        const size_t original_size = children.size();
        children.erase(
            std::remove_if(
                children.begin(),
                children.end(),
                [](const std::shared_ptr<GameObject>& child)
                {
                    return !child || child->is_destroyed();
                }
            ),
            children.end()
        );

        return children.size() != original_size;
    }

    void update_children(const std::vector<std::shared_ptr<GameObject>>& children, double delta)
    {
        for (const std::shared_ptr<GameObject>& child : children)
        {
            if (!child || child->is_destroyed() || !child->is_active())
            {
                continue;
            }

            child->on_update(child->scaled_delta(delta));
        }
    }

    void render_children(const std::vector<std::shared_ptr<GameObject>>& children, SDL_Renderer* renderer)
    {
        for (const std::shared_ptr<GameObject>& child : children)
        {
            if (!child || child->is_destroyed() || !child->is_visible())
            {
                continue;
            }

            child->on_render(renderer);
        }
    }

    void input_children(const std::vector<std::shared_ptr<GameObject>>& children, const InputSnapshot& input)
    {
        for (const std::shared_ptr<GameObject>& child : children)
        {
            if (!child || child->is_destroyed() || !child->is_active())
            {
                continue;
            }

            child->on_input(input);
        }
    }

    void input_event_children(const std::vector<std::shared_ptr<GameObject>>& children, const InputEvent& event)
    {
        for (const std::shared_ptr<GameObject>& child : children)
        {
            if (!child || child->is_destroyed() || !child->is_active())
            {
                continue;
            }

            child->on_input_event(event);
        }
    }

    void reset_children(const std::vector<std::shared_ptr<GameObject>>& children)
    {
        for (const std::shared_ptr<GameObject>& child : children)
        {
            if (child)
            {
                child->reset();
            }
        }
    }
}
