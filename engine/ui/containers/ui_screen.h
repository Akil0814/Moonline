#pragma once

#include "../animation/ui_transition.h"
#include "ui_panel.h"
#include "../base/ui_focusable.h"

#include <memory>
#include <vector>

class UiScreen : public UiPanel
{
public:
    explicit UiScreen(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void on_update(double delta) override;
    void on_render(SDL_Renderer* renderer) override;
    void on_input(const InputSnapshot& input) override;
    void on_input_event(const InputEvent& event) override;
    void reset() override;

    void open();
    void close();
    void set_open(bool open);
    [[nodiscard]] bool is_open() const;

    void set_input_enabled(bool enabled);
    [[nodiscard]] bool is_input_enabled() const;

    void set_transition_enabled(bool enabled);
    [[nodiscard]] bool is_transition_enabled() const;

    void configure_transition(
        const UiTransitionState& hidden_state,
        const UiTransitionState& shown_state,
        double duration_seconds,
        UiEasing easing
    );
    void set_transition_duration(double duration_seconds);
    void set_transition_easing(UiEasing easing);
    [[nodiscard]] const UiTransition& transition() const;

    void register_focusable_control(const std::shared_ptr<UiFocusable>& control);
    void clear_focusable_controls();
    void focus_next_control();
    void focus_previous_control();
    void set_focused_control(int index);
    [[nodiscard]] int focused_control_index() const;

private:
    void apply_transition_state();
    [[nodiscard]] bool prepare_focusable_controls();
    [[nodiscard]] int normalized_focus_index(int index) const;
    void step_focus(int direction);
    void cleanup_focusable_controls();
    void apply_control_focus();
    [[nodiscard]] std::shared_ptr<UiFocusable> focused_control() const;
    [[nodiscard]] bool is_focus_navigation_event(const InputEvent& event) const;

private:
    bool _is_open = true;
    bool _is_closing = false;
    bool _input_enabled = true;
    bool _transition_enabled = false;
    bool _was_mouse_down = false;
    int _focused_control_index = -1;
    std::vector<std::weak_ptr<UiFocusable>> _focusable_controls;
    UiTransition _transition;
};
