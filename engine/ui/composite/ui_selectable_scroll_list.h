#pragma once

#include "../containers/ui_scroll_panel.h"
#include "../base/ui_focusable.h"

class UiSelectableScrollList : public UiScrollPanel, public UiFocusable
{
public:
    explicit UiSelectableScrollList(
        Vector2 position = Vector2::zero(),
        Vector2 size = Vector2::zero(),
        int order = 0
    );

    void on_input_event(const InputEvent& event) override;
    void reset() override;

    [[nodiscard]] int selected_index() const;
    void set_selected_index(int index);

    void set_selection_view_padding(float selection_view_padding);
    [[nodiscard]] float selection_view_padding() const;

    void set_enabled(bool enabled) override;
    [[nodiscard]] bool is_enabled() const override;
    void set_focused(bool focused) override;
    [[nodiscard]] bool is_focused() const override;
    [[nodiscard]] GameObject* game_object() override;
    [[nodiscard]] const GameObject* game_object() const override;

protected:
    [[nodiscard]] bool handle_navigation_input_event(const InputEvent& event);
    void reset_selection_state();
    void sync_selection_index();
    void sync_selection_view();

private:
    [[nodiscard]] virtual size_t selectable_item_count() const = 0;
    [[nodiscard]] virtual bool selectable_item_enabled(int index) const = 0;
    [[nodiscard]] virtual const GameObject* selected_item_view_target() const = 0;
    virtual void sync_selection_state() = 0;

private:
    float _selection_view_padding = 24.0f;
    int _selected_index = -1;
    bool _enabled = true;
    bool _is_focused = false;
};
