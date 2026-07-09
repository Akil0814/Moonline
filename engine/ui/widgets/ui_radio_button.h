#pragma once

#include "../../core/render/colors.h"
#include "../core/ui_control.h"
#include "../style/ui_style.h"
#include "../style/ui_interaction_style.h"
#include "../text/ui_text_content.h"
#include "../text/ui_typography.h"
#include "ui_labeled_checkbox.h"

#include <functional>
#include <optional>
#include <string>

namespace elysia::ui
{
enum class UiRadioButtonLabelPlacement
{
    Left,
    Right
};

enum class UiRadioButtonTextPlacement
{
    NearIndicator,
    FarEdge
};

// Sound keys played as radio-button focus and selection state change.
struct UiRadioButtonSounds
{
    std::string focus;
    std::string press;
    std::string select;
};

// Visual styling for radio-button chrome, mark, and label text.
struct UiRadioButtonStyle
{
    UiChromeStyle chrome{};
    UiEnabledDisabledColors mark{};
    UiEnabledDisabledColors text{};
};

// Bundles sounds, visuals, and label placement for a radio button.
struct UiRadioButtonConfig
{
    std::optional<UiRadioButtonSounds> sounds = std::nullopt;
    std::optional<UiRadioButtonStyle> style = std::nullopt;
    UiTextContent text_content{};
    UiRadioButtonLabelPlacement label_placement = UiRadioButtonLabelPlacement::Right;
    float label_spacing = 8.0f;
    UiRadioButtonTextPlacement text_placement = UiRadioButtonTextPlacement::NearIndicator;
    std::optional<elysia::core::Color> text_color = std::nullopt;
    std::optional<elysia::core::Color> disabled_text_color = std::nullopt;
    std::optional<int> padding = std::nullopt;
    std::optional<int> label_padding = std::nullopt;
    bool draw_background = false;
    bool draw_border = false;
};

using UiRadioButtonSelectedCallback = std::function<void()>;

class UiRadioButton : public UiControl
{
public:
    explicit UiRadioButton(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiRadioButton(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiRadioButton(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;

    UiRadioButton(const elysia::core::Rect& rect,const UiRadioButtonConfig& config,int order = 0) noexcept;
    UiRadioButton(const elysia::core::Vector2& position,const elysia::core::Vector2& size,const UiRadioButtonConfig& config,int order = 0) noexcept;
    UiRadioButton(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,const UiRadioButtonConfig& config,int order = 0) noexcept;

    ~UiRadioButton() override = default;

    void reset() noexcept override;

    void set_enabled(bool enabled) override;
    void set_focused(bool focused) override;

    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

    // Applies sounds, visuals, and label behavior as one configuration update.
    void set_radio_button_config(const UiRadioButtonConfig& config);

    // Sets the selection state without implying group-level coordination.
    void set_selected(bool selected) noexcept;
    [[nodiscard]] bool is_selected() const noexcept;
    // Selects this button and emits the selected callback when the state changes.
    void select();
    void set_on_selected(UiRadioButtonSelectedCallback on_selected);

    void set_text_content(UiTextContent text_content);
    [[nodiscard]] const UiTextContent& text_content() const noexcept;

    void set_typography_role(UiTypographyRole role) noexcept;
    [[nodiscard]] UiTypographyRole typography_role() const noexcept;

    void set_style(const UiRadioButtonStyle& style) noexcept;
    [[nodiscard]] const UiRadioButtonStyle& style() const noexcept;
    [[nodiscard]] bool has_style_override() const noexcept;
    void clear_style_override() noexcept;

private:
    // Applies the config payload without exposing intermediate label visuals.
    void apply_radio_button_config(const UiRadioButtonConfig& config);
    // Mirrors the outer radio state into the owned labeled checkbox before input or render.
    void sync_checkbox_state() const;
    // Updates selection and emits callbacks only when the value actually changes.
    [[nodiscard]] bool set_selected_internal(bool selected,bool notify) noexcept;
    // Plays a configured sound only when the corresponding key is present.
    void play_sound_if_set(const std::string& sound_key) const;
    [[nodiscard]] UiCheckboxStyle checkbox_style() const noexcept;
    [[nodiscard]] UiCheckboxSounds checkbox_sounds() const noexcept;
    [[nodiscard]] static UiLabeledCheckboxLabelPlacement to_checkbox_label_placement(UiRadioButtonLabelPlacement placement) noexcept;
    [[nodiscard]] static UiLabeledCheckboxTextPlacement to_checkbox_text_placement(UiRadioButtonTextPlacement placement) noexcept;
    void apply_theme(const UiTheme& theme) override;

private:
    mutable UiLabeledCheckbox _checkbox;
    UiRadioButtonSelectedCallback _on_selected;
    std::optional<UiRadioButtonSounds> _sounds;
    UiStyleState<UiRadioButtonStyle> _style_state;
    UiTextContent _text_content;
    UiTypographyRole _typography_role = UiTypographyRole::RadioLabel;
    UiRadioButtonLabelPlacement _label_placement = UiRadioButtonLabelPlacement::Right;
    float _label_spacing = 8.0f;
    UiRadioButtonTextPlacement _text_placement = UiRadioButtonTextPlacement::NearIndicator;
    elysia::core::Color _text_color{};
    elysia::core::Color _disabled_text_color{};
    int _padding = 0;
    int _label_padding = 0;
    bool _selected = false;
    bool _draw_background = false;
    bool _draw_border = false;
};
}
