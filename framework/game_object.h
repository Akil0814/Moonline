#pragma once

class GameObject
{
    void render(SDL_Renderer* renderer);

    void set_visible(bool visible) { _visible = visible; }
    bool is_visible() const { return _visible; }

private:
    bool _visible = true;
    bool _active = true;
};