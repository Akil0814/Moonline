#pragma once

#include "../../core/render/colors.h"
#include "../core/ui_control.h"
#include "../style/ui_interaction_style.h"
#include "label/ui_label.h"

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

struct UiRadioButtonSounds
{
    std::string focus;
    std::string press;
    std::string select;
};

struct UiRadioButtonStyle
{
    UiChromeStyle chrome{};
    UiEnabledDisabledColors mark{};
    UiEnabledDisabledColors text{};
};

struct UiRadioButtonConfig
{
    std::optional<UiRadioButtonSounds> sounds = std::nullopt;
    std::optional<UiRadioButtonStyle> style = std::nullopt;
    std::string text_key{};
    UiRadioButtonLabelPlacement label_placement = UiRadioButtonLabelPlacement::Right;
    float label_spacing = 8.0f;
    UiRadioButtonTextPlacement text_placement = UiRadioButtonTextPlacement::NearIndicator;
    std::optional<elysia::core::Color> text_color = std::nullopt;
    std::optional<elysia::core::Color> disabled_text_color = std::nullopt;
    std::optional<int> text_point_size = std::nullopt;
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

    void set_radio_button_config(const UiRadioButtonConfig& config);

    void set_selected(bool selected) noexcept;
    [[nodiscard]] bool is_selected() const noexcept;
    void select();
    void set_on_selected(UiRadioButtonSelectedCallback on_selected);

    void set_text_key(std::string text_key);
    [[nodiscard]] const std::string& text_key() const noexcept;

    void set_label_placement(UiRadioButtonLabelPlacement placement) noexcept;
    [[nodiscard]] UiRadioButtonLabelPlacement label_placement() const noexcept;

    void set_label_spacing(float spacing) noexcept;
    [[nodiscard]] float label_spacing() const noexcept;

    void set_text_placement(UiRadioButtonTextPlacement placement) noexcept;
    [[nodiscard]] UiRadioButtonTextPlacement text_placement() const noexcept;

    void set_style(const UiRadioButtonStyle& style) noexcept;
    [[nodiscard]] const UiRadioButtonStyle& style() const noexcept;

    void set_text_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color text_color() const noexcept;

    void set_disabled_text_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color disabled_text_color() const noexcept;

    void set_sounds(const UiRadioButtonSounds& sounds);
    void clear_sounds() noexcept;
    [[nodiscard]] const std::optional<UiRadioButtonSounds>& sounds() const noexcept;

    void set_text_point_size(int point_size) noexcept;
    [[nodiscard]] int text_point_size() const noexcept;

    void set_padding(int padding) noexcept;
    [[nodiscard]] int padding() const noexcept;

    void set_label_padding(int padding) noexcept;
    [[nodiscard]] int label_padding() const noexcept;

    void set_draw_background(bool draw_background) noexcept;
    [[nodiscard]] bool draws_background() const noexcept;
    void set_draw_border(bool draw_border) noexcept;
    [[nodiscard]] bool draws_border() const noexcept;

private:
    void apply_radio_button_config(const UiRadioButtonConfig& config);
    [[nodiscard]] bool set_selected_internal(bool selected,bool notify) noexcept;
    [[nodiscard]] bool can_interact() const noexcept;
    [[nodiscard]] bool can_receive_pointer() const noexcept;
    [[nodiscard]] bool contains_pointer(int mouse_x,int mouse_y) const noexcept;
    [[nodiscard]] bool is_primary_pointer_event(const UiInputEvent& event) const noexcept;
    void clear_pushed_state() noexcept;
    void play_sound_if_set(const std::string& sound_key) const;
    void sync_label_visuals() const;
    [[nodiscard]] elysia::core::Rect content_rect() const noexcept;
    [[nodiscard]] elysia::core::Rect indicator_rect() const noexcept;
    [[nodiscard]] elysia::core::Rect label_rect() const noexcept;
    [[nodiscard]] elysia::core::Color current_background_color() const noexcept;
    [[nodiscard]] elysia::core::Color current_border_color() const noexcept;
    [[nodiscard]] elysia::core::Color current_mark_color() const noexcept;
    [[nodiscard]] elysia::core::Color current_text_color() const noexcept;

private:
    mutable UiLabel _label;
    UiRadioButtonSelectedCallback _on_selected;
    std::optional<UiRadioButtonSounds> _sounds;
    UiRadioButtonStyle _style{};
    std::string _text_key;
    UiRadioButtonLabelPlacement _label_placement = UiRadioButtonLabelPlacement::Right;
    float _label_spacing = 8.0f;
    UiRadioButtonTextPlacement _text_placement = UiRadioButtonTextPlacement::NearIndicator;
    int _padding = 4;
    bool _selected = false;
    bool _draw_background = false;
    bool _draw_border = false;
    bool _is_pushed = false;
};
}
