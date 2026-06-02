#pragma once

#include "ui_selectable_button_group_utils.h"
#include "ui_selectable_button.h"

#include <functional>
#include <memory>
#include <vector>

using UiButtonGroupSelectionChangedCallback = std::function<void(
    int index,
    UiSelectableButton* button
)>;

class UiButtonGroup
{
public:
    ~UiButtonGroup();

    void add_button(const std::shared_ptr<UiSelectableButton>& button);
    bool remove_button(const UiSelectableButton* button);
    void clear_buttons();

    [[nodiscard]] size_t button_count() const;

    void set_selected_index(int index, bool notify = true);
    [[nodiscard]] int selected_index() const;
    [[nodiscard]] std::shared_ptr<UiSelectableButton> selected_button() const;

    void set_on_selection_changed(UiButtonGroupSelectionChangedCallback on_selection_changed);

private:
    void cleanup_buttons();
    void handle_button_selection_changed(UiSelectableButton* button, bool selected);
    [[nodiscard]] int button_index(const UiSelectableButton* button) const;

private:
    std::vector<UiSelectableButtonRegistration> _buttons;
    UiButtonGroupSelectionChangedCallback _on_selection_changed;
    int _selected_index = -1;
    bool _is_syncing_selection = false;
};
