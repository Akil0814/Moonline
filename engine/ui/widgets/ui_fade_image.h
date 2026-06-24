#pragma once
#include <functional>

#include "ui_image.h"
#include "../../core/interface/updatable.h"
#include "../../tools/timer.h"

struct SDL_Texture;

namespace elysia::ui
{
enum class UiFadeImageMode
{
    FadeIn,
    FadeOut,
    FadeInOut
};

class UiFadeImage : public UiImage, public elysia::core::Updatable
{
public:
    using FadeImageOnEnd = std::function<void()>;

    UiFadeImage(SDL_Texture* texture, elysia::core::Vector2 pos, elysia::core::Vector2 size, int order = 0);
    UiFadeImage(SDL_Texture* texture, elysia::core::Rect rect ,int order = 0);
    UiFadeImage(
        SDL_Texture* texture,
        elysia::core::Vector2 center,
        elysia::core::Vector2 image_size,
        UiFromCenterTag,
        int order = 0
    );
    UiFadeImage(
        SDL_Texture* texture,
        elysia::core::Vector2 center,
        elysia::core::Vector2 source_size,
        elysia::core::Vector2 render_size,
        UiFromCenterTag,
        int order = 0
    );

    void reset() noexcept override;

    void configure_playback(
        UiFadeImageMode mode,double hold_time,
        double fade_in_duration,double fade_out_duration
    );
    void play();

    void update(double delta)override;

    void set_on_end(FadeImageOnEnd on_end);

private:
    void update_fade_in(double delta);
    void update_fade_out(double delta);

    void start_hold();

private:
    enum class FadeState
    {
        Idle,
        FadingIn,
        Holding,
        FadingOut,
        Finished
    };

private:
    UiFadeImageMode _mode = UiFadeImageMode::FadeIn;
    FadeState _state = FadeState::Idle;

    FadeImageOnEnd _on_end;

    elysia::tools::Timer _timer;

    double _elapsed = 0.0;
    double _hold_time = 0.0;

    double _fade_in_duration = 1.0;
    double _fade_out_duration = 1.0;
};

}