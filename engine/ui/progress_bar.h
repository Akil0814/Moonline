#pragma once
#include <SDL.h>
#include "../core/game_object.h"
#include "bar.h"

class ProgressBar : public GameObject
{
public:
    ProgressBar(Vector2 position, Vector2 size);
    ~ProgressBar()=default;

    void on_render(SDL_Renderer* renderer)override;
    void reset()override;
    void set_progress(float value);
    [[nodiscard]] float progress() const;

    Bar& bar();
    const Bar& bar() const;

private:
    Bar _bar;
};
