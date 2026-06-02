#pragma once

#include "../ui_layout.h"

#include <SDL.h>

enum class UiEasing
{
    Linear,
    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,
    EaseOutBack
};

struct UiTransitionState
{
    UiLayoutTransform _transform;
    Uint8 _background_alpha = 255;
};

class UiTransition
{
public:
    void set_hidden_state(const UiTransitionState& state);
    [[nodiscard]] const UiTransitionState& hidden_state() const;

    void set_shown_state(const UiTransitionState& state);
    [[nodiscard]] const UiTransitionState& shown_state() const;

    void set_duration_seconds(double duration_seconds);
    [[nodiscard]] double duration_seconds() const;

    void set_easing(UiEasing easing);
    [[nodiscard]] UiEasing easing() const;

    void play_forward();
    void play_backward();
    void jump_to_hidden();
    void jump_to_shown();
    void update(double delta_seconds);

    [[nodiscard]] const UiTransitionState& current_state() const;
    [[nodiscard]] bool is_playing() const;
    [[nodiscard]] bool is_shown() const;
    [[nodiscard]] bool is_hidden() const;
    [[nodiscard]] bool is_forward() const;

private:
    static float apply_easing(UiEasing easing, float t);
    static UiTransitionState interpolate(
        const UiTransitionState& from,
        const UiTransitionState& to,
        float t
    );

private:
    UiTransitionState _hidden_state;
    UiTransitionState _shown_state;
    UiTransitionState _current_state;

    double _duration_seconds = 0.25;
    double _elapsed_seconds = 0.0;

    UiEasing _easing = UiEasing::EaseOutQuad;

    bool _is_playing = false;
    bool _is_forward = true;
};
