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

struct UiLabeledCheckboxConfig
{
    UiCheckboxConfig checkbox{};
    std::string text_key{};
    UiLabeledCheckboxLabelPlacement label_placement = UiLabeledCheckboxLabelPlacement::Right;
    float label_spacing = 8.0f;
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

    void set_labeled_checkbox_config(const UiLabeledCheckboxConfig& config);

    void set_text_key(std::string text_key);
    [[nodiscard]] const std::string& text_key() const noexcept;

    void set_label_placement(UiLabeledCheckboxLabelPlacement placement) noexcept;
    [[nodiscard]] UiLabeledCheckboxLabelPlacement label_placement() const noexcept;

    void set_label_spacing(float spacing) noexcept;
    [[nodiscard]] float label_spacing() const noexcept;

    void set_text_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color text_color() const noexcept;

    void set_disabled_text_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color disabled_text_color() const noexcept;

    void set_text_point_size(int point_size) noexcept;
    [[nodiscard]] int text_point_size() const noexcept;

    void set_label_padding(int padding) noexcept;
    [[nodiscard]] int label_padding() const noexcept;

protected:
    [[nodiscard]] elysia::core::Rect checkbox_rect() const noexcept override;

private:
    void apply_labeled_checkbox_config(const UiLabeledCheckboxConfig& config);
    void sync_label_visuals() const;
    [[nodiscard]] elysia::core::Rect label_rect() const noexcept;

private:
    mutable UiLabel _label;
    std::string _text_key;
    UiLabeledCheckboxLabelPlacement _label_placement = UiLabeledCheckboxLabelPlacement::Right;
    float _label_spacing = 8.0f;
    elysia::core::Color _text_color = elysia::core::colors::white;
    elysia::core::Color _disabled_text_color = elysia::core::colors::gray_300;
};
}
