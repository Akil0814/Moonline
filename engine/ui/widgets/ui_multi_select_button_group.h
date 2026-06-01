#pragma once

#include "ui_selectable_button.h"

#include <functional>
#include <memory>
#include <vector>

using UiMultiSelectButtonGroupSelectionChangedCallback = std::function<void(
    int index,
    UiSelectableButton* button,
    bool selected
)>;

class UiMultiSelectButtonGroup
{
public:
    ~UiMultiSelectButtonGroup();

    void add_button(const std::shared_ptr<UiSelectableButton>& button);
    bool remove_button(const UiSelectableButton* button);
    void clear_buttons();
    void clear_selection();

    [[nodiscard]] size_t button_count() const;
    [[nodiscard]] bool is_button_selected(int index) const;
    bool set_button_selected(int index, bool selected);
    bool toggle_button_selected(int index);
    [[nodiscard]] std::vector<int> selected_indices() const;
    [[nodiscard]] std::vector<std::shared_ptr<UiSelectableButton>> selected_buttons() const;

    void set_on_selection_changed(UiMultiSelectButtonGroupSelectionChangedCallback on_selection_changed);

private:
    struct ButtonRegistration
    {
        std::weak_ptr<UiSelectableButton> _button;
        UiSelectableButtonListenerId _listener_id = 0;
    };

private:
    void cleanup_buttons();
    void handle_button_selection_changed(UiSelectableButton* button, bool selected);
    [[nodiscard]] int button_index(const UiSelectableButton* button) const;
    [[nodiscard]] std::shared_ptr<UiSelectableButton> button_at(int index) const;

private:
    std::vector<ButtonRegistration> _buttons;
    UiMultiSelectButtonGroupSelectionChangedCallback _on_selection_changed;
    bool _is_syncing_selection = false;
};
