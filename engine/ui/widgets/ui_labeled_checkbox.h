#pragma once

#include "ui_checkbox.h"
#include "label/ui_label.h"
#include "../text/ui_text_content.h"
#include "../text/ui_typography.h"

#include <optional>

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
    UiTextContent text_content{};
    UiLabeledCheckboxLabelPlacement label_placement = UiLabeledCheckboxLabelPlacement::Right;
    float label_spacing = 8.0f;
    UiLabeledCheckboxTextPlacement text_placement = UiLabeledCheckboxTextPlacement::NearBox;
    std::optional<UiEnabledDisabledColors> text_colors = std::nullopt;
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

    void set_text_content(UiTextContent text_content);
    [[nodiscard]] const UiTextContent& text_content() const noexcept;

    void set_label_placement(UiLabeledCheckboxLabelPlacement placement) noexcept;
    [[nodiscard]] UiLabeledCheckboxLabelPlacement label_placement() const noexcept;

    void set_label_spacing(float spacing) noexcept;
    [[nodiscard]] float label_spacing() const noexcept;

    void set_text_placement(UiLabeledCheckboxTextPlacement placement) noexcept;
    [[nodiscard]] UiLabeledCheckboxTextPlacement text_placement() const noexcept;

    void set_typography_role(UiTypographyRole role) noexcept;
    [[nodiscard]] UiTypographyRole typography_role() const noexcept;

    void set_label_padding(int padding) noexcept;
    [[nodiscard]] int label_padding() const noexcept;

protected:
    // Shrinks the inherited checkbox indicator rect to make room for the label.
    [[nodiscard]] elysia::core::Rect checkbox_rect() const noexcept override;
    void apply_theme(const UiTheme& theme) override;

private:
    // Applies the config payload without exposing intermediate label visuals.
    void apply_labeled_checkbox_config(const UiLabeledCheckboxConfig& config);
    // Mirrors checkbox enabled/focus visuals into the owned label widget.
    void sync_label_visuals() const;
    // Returns the rect used to render the label beside the checkbox indicator.
    [[nodiscard]] elysia::core::Rect label_rect() const noexcept;

private:
    mutable UiLabel _label;
    UiTextContent _text_content;
    UiTypographyRole _typography_role = UiTypographyRole::CheckboxLabel;
    UiLabeledCheckboxLabelPlacement _label_placement = UiLabeledCheckboxLabelPlacement::Right;
    float _label_spacing = 8.0f;
    UiLabeledCheckboxTextPlacement _text_placement = UiLabeledCheckboxTextPlacement::NearBox;
    elysia::core::Color _text_color{};
    elysia::core::Color _disabled_text_color{};
    bool _draw_background = false;
    bool _draw_border = false;
};
}


