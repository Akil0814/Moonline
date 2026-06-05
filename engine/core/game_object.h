#pragma once
#include <SDL.h>
#include <algorithm>
#include "geometry/vector2.h"
#include "geometry/rect.h"

#include "render/render_command.h"
#include "depth_layer.h"
#include "../input/input_system.h"

class GameObject
{
public:
    explicit GameObject( DepthLayer layer, int order = 0 )
        :_depth_layer(layer), _order_in_layer(order){}

    virtual ~GameObject() = default;

    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;
    GameObject(GameObject&&) = default;
    GameObject& operator=(GameObject&&) = default;

    virtual void update(double delta) {}
    void submit_render_commands(std::vector<RenderCommand>& out_commands) const;

    virtual void reset()
    {
        _visible = true;
        _active = true;
        _destroyed = false;
        _update_when_paused = false;
        _input_when_paused = false;
        _time_scale = 1.0;
    }

    void destroy() { _destroyed = true; }
    bool is_destroyed() const { return _destroyed; }

    virtual void set_world_position(const Vector2& position)
    {
        _world_pos = position;
        sync_rect();
    }
    const Vector2& position() const { return _world_pos; }

    void set_center_pos(const Vector2& center_pos)
    {
        _world_pos = { center_pos.x - _size.x * 0.5f, center_pos.y - _size.y * 0.5f };
        sync_rect();
    }

    Vector2 center() const
    {
        return { _world_pos.x + _size.x * 0.5f, _world_pos.y + _size.y * 0.5f };
    }

    const Vector2& size() const { return _size; }
    virtual void set_size(const Vector2& size)
    {
        _size = size;
        sync_rect();
    }

    const SDL_Rect& rect() const { return _obj_rect; }

    DepthLayer depth_layer() const { return _depth_layer; }
    int order_in_layer() const { return _order_in_layer; }

    void set_visible(bool visible) { _visible = visible; }
    bool is_visible() const { return _visible; }

    void set_active(bool active) { _active = active; }
    bool is_active()const { return _active; }

    void set_update_when_paused(bool enable) { _update_when_paused = enable; }
    bool will_update_when_paused() const { return _update_when_paused; }

    void set_input_when_paused(bool enable) { _input_when_paused = enable; }
    bool will_input_when_paused() const { return _input_when_paused; }

    void set_time_scale(double scale) { _time_scale = std::max(0.0, scale); }
    [[nodiscard]] double time_scale() const { return _time_scale; }
    [[nodiscard]] double scaled_delta(double parent_delta) const
    {
        return parent_delta * _time_scale;
    }

private:
    void sync_rect()
    {
        _obj_rect._x = static_cast<int>(_world_pos.x);
        _obj_rect._y = static_cast<int>(_world_pos.y);

        const int width = static_cast<int>(_size.x);
        const int height = static_cast<int>(_size.y);
        _obj_rect._w = width > 0 ? width : 0;
        _obj_rect._h = height > 0 ? height : 0;
    }

    Vector2 _world_pos{ 0 , 0 };
    Vector2 _size { 0 , 0 };

    Rect _obj_rect{ 0 };

    DepthLayer _depth_layer = DepthLayer::Item;
    int _order_in_layer = 0;

    bool _destroyed = false;
    bool _visible = true;
    bool _active = true;

    bool _update_when_paused = false;
    bool _input_when_paused = false;
    double _time_scale = 1.0;
};
