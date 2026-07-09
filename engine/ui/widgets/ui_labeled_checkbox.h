#pragma once

#include "ui_checkbox.h"
#include "label/ui_label.h"

#include <string>

namespace elysia::ui
{
enum class UiLabeledCheckboxLabelPlacement
{
    Left,
    Right
};

enum class UiLabeledCheckboxTextPlacement
{
    NearBox,
    FarEdge
};

// Bundles checkbox config with label text and label placement behavior.
struct UiLabeledCheckboxConfig
{
    UiCheckboxConfig checkbox{};
    std::string text_key{};
    UiLabeledCheckboxLabelPlacement label_placement = UiLabeledCheckboxLabelPlacement::Right;
    float label_spacing = 8.0f;
    UiLabeledCheckboxTextPlacement text_placement = UiLabeledCheckboxTextPlacement::NearBox;
    bool draw_background = false;
    bool draw_border = false;
};

class UiLabeledCheckbox : public UiCheckbox
{
public:
    explicit UiLabeledCheckbox(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiLabeledCheckbox(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiLabeledCheckbox(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;

    UiLabeledCheckbox(const elysia::core::Rect& rect,const UiLabeledCheckboxConfig& config,int order = 0) noexcept;
    UiLabeledCheckbox(const elysia::core::Vector2& position,const elysia::core::Vector2& size,const UiLabeledCheckboxConfig& config,int order = 0) noexcept;
    UiLabeledCheckbox(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,const UiLabeledCheckboxConfig& config,int order = 0) noexcept;

    ~UiLabeledCheckbox() override = default;

    void reset() noexcept override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

    // Applies checkbox behavior together with label content and label placement rules.
    void set_labeled_checkbox_config(const UiLabeledCheckboxConfig& config);

    void set_text_key(std::string text_key);
    [[nodiscard]] const std::string& text_key() const noexcept;

    void set_label_placement(UiLabeledCheckboxLabelPlacement placement) noexcept;
    [[nodiscard]] UiLabeledCheckboxLabelPlacement label_placement() const noexcept;

    void set_label_spacing(float spacing) noexcept;
    [[nodiscard]] float label_spacing() const noexcept;

    void set_text_placement(UiLabeledCheckboxTextPlacement placement) noexcept;
    [[nodiscard]] UiLabeledCheckboxTextPlacement text_placement() const noexcept;

    void set_text_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color text_color() const noexcept;

    void set_disabled_text_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color disabled_text_color() const noexcept;

    void set_text_point_size(int point_size) noexcept;
    [[nodiscard]] int text_point_size() const noexcept;

    void set_label_padding(int padding) noexcept;
    [[nodiscard]] int label_padding() const noexcept;

    void set_draw_background(bool draw_background) noexcept;
    [[nodiscard]] bool draws_background() const noexcept;
    void set_draw_border(bool draw_border) noexcept;
    [[nodiscard]] bool draws_border() const noexcept;

protected:
    // Shrinks the inherited checkbox indicator rect to make room for the label.
    [[nodiscard]] elysia::core::Rect checkbox_rect() const noexcept override;

private:
    // Applies the config payload without exposing intermediate label visuals.
    void apply_labeled_checkbox_config(const UiLabeledCheckboxConfig& config);
    // Mirrors checkbox enabled/focus visuals into the owned label widget.
    void sync_label_visuals() const;
    // Returns the rect used to render the label beside the checkbox indicator.
    [[nodiscard]] elysia::core::Rect label_rect() const noexcept;

private:
    mutable UiLabel _label;
    std::string _text_key;
    UiLabeledCheckboxLabelPlacement _label_placement = UiLabeledCheckboxLabelPlacement::Right;
    float _label_spacing = 8.0f;
    UiLabeledCheckboxTextPlacement _text_placement = UiLabeledCheckboxTextPlacement::NearBox;
    elysia::core::Color _text_color{};
    elysia::core::Color _disabled_text_color{};
    bool _draw_background = false;
    bool _draw_border = false;
};
}


