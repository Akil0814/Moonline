#pragma once

#include <functional>

#include "ui_image.h"
#include "../../core/interface/updatable.h"
#include "../../tools/timer.h"

struct SDL_Texture;

namespace elysia::ui
{
enum class UiBlinkImageMode
{
    VisibleFirst,
    HiddenFirst
};

class UiBlinkImage : public UiImage, public elysia::core::Updatable
{
public:
    using BlinkImageOnEnd = std::function<void()>;

    UiBlinkImage(SDL_Texture* texture, elysia::core::Vector2 pos, elysia::core::Vector2 size, int order = 0);
    UiBlinkImage(SDL_Texture* texture, elysia::core::Rect rect, int order = 0);
    UiBlinkImage(
        SDL_Texture* texture,
        elysia::core::Vector2 center,
        elysia::core::Vector2 image_size,
        UiFromCenterTag,
        int order = 0
    );
    UiBlinkImage(
        SDL_Texture* texture,
        elysia::core::Vector2 center,
        elysia::core::Vector2 source_size,
        elysia::core::Vector2 render_size,
        UiFromCenterTag,
        int order = 0
    );

    void reset() noexcept override;

    void configure_playback(
        UiBlinkImageMode mode,
        double hold_time,
        double visible_duration,
        double hidden_duration,
        int blink_cycles = -1
    );
    void play();

    void update(double delta) override;

    void set_on_end(BlinkImageOnEnd on_end);

private:
    void advance_phase();
    void schedule_current_phase();
    void finish();
    void apply_current_phase();

private:
    UiBlinkImageMode _mode = UiBlinkImageMode::VisibleFirst;
    BlinkImageOnEnd _on_end;

    elysia::tools::Timer _timer;

    double _hold_time = 0.0;
    double _visible_duration = 0.15;
    double _hidden_duration = 0.15;

    int _blink_cycles = -1;
    int _completed_cycles = 0;

    bool _is_playing = false;
    bool _hold_pending = false;
    bool _current_visible = true;
};

}