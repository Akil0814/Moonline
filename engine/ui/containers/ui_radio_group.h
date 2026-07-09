#pragma once

#include "ui_list_container.h"
#include "../widgets/ui_radio_button.h"

#include <cstddef>
#include <functional>
#include <optional>

namespace elysia::ui
{
using UiRadioGroupSelectionChangedCallback = std::function<void(std::optional<std::size_t> selected_index)>;

class UiRadioGroup : public UiListContainer
{
public:
    explicit UiRadioGroup(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiRadioGroup(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiRadioGroup(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiRadioGroup() override = default;

    void reset() noexcept override;
    void update(double delta) override;
    void on_ui_input_frame(const UiInputFrame& input) override;
    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

    [[nodiscard]] std::optional<std::size_t> selected_index() const noexcept;
    // Selects one radio button and clears selection from the rest of the group.
    [[nodiscard]] bool set_selected_index(std::size_t index);
    void set_on_selection_changed(UiRadioGroupSelectionChangedCallback on_selection_changed);

private:
    // Mirrors button-level selection back into the group and emits one callback when needed.
    void sync_selection(bool notify);
    [[nodiscard]] UiRadioButton* radio_button_at(std::size_t index) const noexcept;

private:
    std::optional<std::size_t> _selected_index = std::nullopt;
    UiRadioGroupSelectionChangedCallback _on_selection_changed;
    bool _is_syncing_selection = false;
};
}
