#pragma once
#include "../game_object.h"
#include "../base/timer.h"

enum class FadeMode
{
    FadeIn,
    FadeOut,
    FadeInOut
};

class FadeImage : public GameObject
{
    FadeImage(SDL_Texture* texture, Vector2 pos, Vector2 size);

    void set_play(FadeMode mode,double hold_time, float fade_in_duration, float fade_out_duration);

    void on_update(double delta)override;
    void on_render(SDL_Renderer* renderer)override;

private:
    Timer _time;

    SDL_Texture* _texture = nullptr;

    float _duration = 1.0f;
    float _elapsed = 0.0f;
    float _alpha = 0.0f;

    bool _finished = false;
};