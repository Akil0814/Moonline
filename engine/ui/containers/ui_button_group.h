#pragma once

#include "ui_list_container.h"
#include "../widgets/ui_button.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>

namespace elysia::ui
{
using UiButtonGroupSelectionChangedCallback = std::function<void(std::optional<std::size_t> selected_index)>;

// A list of buttons that maintains exactly one selected button while it has members.
class UiButtonGroup : public UiListContainer
{
public:
    explicit UiButtonGroup(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiButtonGroup(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiButtonGroup(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiButtonGroup() override = default;

    void reset() noexcept override;
    void update(double delta) override;
    void on_ui_input_frame(const UiInputFrame& input) override;
    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

    // Adds a button and assigns the group-owned click handler that selects it.
    UiButton* add_button(std::unique_ptr<UiButton> button);
    // Returns the selected button's current child position without repairing group state.
    [[nodiscard]] std::optional<std::size_t> selected_index() const noexcept;
    [[nodiscard]] bool set_selected_index(std::size_t index);
    void set_on_selection_changed(UiButtonGroupSelectionChangedCallback on_selection_changed);

private:
    [[nodiscard]] bool select_button(UiButton* button);
    [[nodiscard]] std::optional<std::size_t> find_button_index(const UiButton* button) const noexcept;
    [[nodiscard]] UiButton* button_at(std::size_t index) const noexcept;
    // Repairs selection after child removal or reordering and optionally notifies on identity changes.
    void sync_selection(bool notify);
    // Applies the selected/unselected theme roles to all live button members.
    void refresh_button_styles() noexcept;
    void notify_selection_changed();

private:
    UiButton* _selected_button = nullptr;
    UiButtonGroupSelectionChangedCallback _on_selection_changed;
    bool _is_syncing_selection = false;
};
}
