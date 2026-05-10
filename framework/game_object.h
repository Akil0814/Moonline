#pragma once
#include <SDL.h>
#include "base/vector2.h"
#include "base/depth_layer.h"
using GameObjectID = std::uint64_t;

class GameObject
{
public:
    explicit GameObject( DepthLayer layer, int order = 0 )
        : _id(generate_id()),_depth_layer(layer), _order_in_layer(order){}

    virtual ~GameObject() = default;

    GameObjectID id() const { return _id; }

    virtual void on_update(double delta) {}
    virtual void on_render(SDL_Renderer* renderer) {}
    virtual void on_input(const SDL_Event& event) {}

    virtual void reset()
    {
        _visible = true;
        _active = true;
        _destroyed = false;
        _update_when_paused = false;
    }

    void destroy() { _destroyed = true; }
    bool is_destroyed() const { return _destroyed; }

    void set_position(const Vector2& position) { _position = position; }
    const Vector2& position() const { return _position; }

    void set_center(const Vector2& center) { _center = center; }
    const Vector2& center() const { return _center; }

    void set_size(const Vector2& size) { _size = size; }
    const Vector2& size() const { return _size; }

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

private:
    static GameObjectID generate_id()
    {
        static GameObjectID next_id = 1;
        return next_id++;
    }

private:
    GameObjectID _id = 0;

    Vector2 _position{ 0 , 0 };
    Vector2 _center{ 0 , 0 };
    Vector2 _size { 0 , 0 };

    DepthLayer _depth_layer = DepthLayer::Object;
    int _order_in_layer = 0;

    bool _destroyed = false;
    bool _visible = true;
    bool _active = true;

    bool _update_when_paused = false;
    bool _input_when_paused = false;
};