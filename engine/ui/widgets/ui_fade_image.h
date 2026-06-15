#pragma once
#include <cstdint>
#include <functional>

#include "../core/ui_element.h"
#include "../../core/interface/updatable.h"
#include "../../tools/timer.h"

enum class UiFadeImageMode
{
    FadeIn,
    FadeOut,
    FadeInOut
};

struct SDL_Texture;

class UiFadeImage : public UiElement, public Updatable
{
public:
    using FadeImageOnEnd = std::function<void()>;

    UiFadeImage(SDL_Texture* texture, Vector2 pos, Vector2 size, int order = 0);
    UiFadeImage(SDL_Texture* texture, Rect rect ,int order = 0);

    void reset() noexcept override;
    void submit_ui_render_commands(std::vector<UiRenderCommand>& out_commands) const override;

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
    double ratio(double value, double max_value) const;

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

    Timer _timer;

    SDL_Texture* _texture = nullptr;

    double _elapsed = 0.0;
    double _hold_time = 0.0;

    double _fade_in_duration = 1.0;
    double _fade_out_duration = 1.0;

    std::uint8_t _alpha = 255;
};
