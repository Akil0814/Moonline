#pragma once

#include <functional>

#include "ui_image.h"
#include "../../core/interface/updatable.h"
#include "../../tools/timer.h"

struct SDL_Texture;

namespace elysia::ui
{
enum class UiPulseImageMode
{
    MinToMax,
    MaxToMin
};

class UiPulseImage : public UiImage, public elysia::core::Updatable
{
public:
    using PulseImageOnEnd = std::function<void()>;

    UiPulseImage(SDL_Texture* texture, elysia::core::Vector2 pos, elysia::core::Vector2 size, int order = 0);
    UiPulseImage(SDL_Texture* texture, elysia::core::Rect rect, int order = 0);
    UiPulseImage(
        SDL_Texture* texture,
        elysia::core::Vector2 center,
        elysia::core::Vector2 image_size,
        UiImageCenterTag,
        int order = 0
    );
    UiPulseImage(
        SDL_Texture* texture,
        elysia::core::Vector2 center,
        elysia::core::Vector2 source_size,
        elysia::core::Vector2 render_size,
        UiImageCenterTag,
        int order = 0
    );

    void reset() noexcept override;

    void configure_playback(
        UiPulseImageMode mode,
        double hold_time,
        double pulse_in_duration,
        double pulse_out_duration,
        int pulse_cycles = -1,
        std::uint8_t min_alpha = 96,
        std::uint8_t max_alpha = 255
    );
    void play();

    void update(double delta) override;

    void set_on_end(PulseImageOnEnd on_end);

private:
    enum class PulseState
    {
        Idle,
        PulsingUp,
        HoldingMax,
        PulsingDown,
        HoldingMin,
        Finished
    };

private:
    void update_pulse_up(double delta);
    void update_pulse_down(double delta);
    void start_hold(bool at_max);
    void finish();

    [[nodiscard]] bool started_from_min() const;
    [[nodiscard]] bool complete_cycle_if_needed(bool reached_max);
    [[nodiscard]] double ratio(double value, double max_value) const;
    [[nodiscard]] double ease_in_out(double t) const;
    [[nodiscard]] std::uint8_t lerp_opacity(std::uint8_t from, std::uint8_t to, double t) const;

private:
    UiPulseImageMode _mode = UiPulseImageMode::MinToMax;
    PulseState _state = PulseState::Idle;
    PulseImageOnEnd _on_end;

    elysia::tools::Timer _timer;

    double _elapsed = 0.0;
    double _hold_time = 0.0;
    double _pulse_in_duration = 1.0;
    double _pulse_out_duration = 1.0;

    int _pulse_cycles = -1;
    int _completed_cycles = 0;

    std::uint8_t _min_alpha = 96;
    std::uint8_t _max_alpha = 255;
};

}
