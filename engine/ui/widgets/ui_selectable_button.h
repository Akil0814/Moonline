#pragma once

#include "ui_text_button.h"

#include <functional>
#include <vector>

class UiSelectableButton;
using UiSelectableButtonListenerId = size_t;

enum class UiSelectableActivationBehavior
{
    Select,
    Toggle
};

using UiSelectableButtonSelectionChangedCallback = std::function<void(
    UiSelectableButton& button,
    bool selected
)>;

class UiSelectableButton : public UiTextButton
{
public:
    explicit UiSelectableButton(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void reset() override;

    void set_selected(bool selected);
    [[nodiscard]] bool is_selected() const;

    UiSelectableButtonListenerId add_selection_listener(UiSelectableButtonSelectionChangedCallback callback);
    bool remove_selection_listener(UiSelectableButtonListenerId listener_id);
    void clear_selection_listeners();
    void set_activation_behavior(UiSelectableActivationBehavior activation_behavior);
    [[nodiscard]] UiSelectableActivationBehavior activation_behavior() const;

protected:
    void on_activated() override;
    [[nodiscard]] Status visual_status() const override;

private:
    struct SelectionListenerEntry
    {
        UiSelectableButtonListenerId _id = 0;
        UiSelectableButtonSelectionChangedCallback _callback;
    };

    void set_selected_internal(bool selected, bool notify);

private:
    std::vector<SelectionListenerEntry> _selection_listeners;
    UiSelectableButtonListenerId _next_listener_id = 1;
    UiSelectableActivationBehavior _activation_behavior = UiSelectableActivationBehavior::Select;
    bool _selected = false;
};
