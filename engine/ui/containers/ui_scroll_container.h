#pragma once

#include "ui_container.h"

namespace elysia::ui
{
class UiScrollContainer : public UiContainer
{
public:
    explicit UiScrollContainer(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiScrollContainer(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiScrollContainer(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiScrollContainer() override = default;

    void reset() noexcept override;
    bool on_ui_input_event(const UiInputEvent& event) override;

    UiElement* set_content(std::unique_ptr<UiElement> content);
    [[nodiscard]] UiElement* content() noexcept;
    [[nodiscard]] const UiElement* content() const noexcept;
    void clear_content();

    void set_content_height(float content_height) noexcept;
    [[nodiscard]] float content_height() const noexcept;
    void set_scroll_offset(float scroll_offset) noexcept;
    [[nodiscard]] float scroll_offset() const noexcept;
    void set_scroll_step(float scroll_step) noexcept;
    [[nodiscard]] float scroll_step() const noexcept;
    void scroll_to_top() noexcept;
    void scroll_to_bottom() noexcept;

protected:
    void rebuild_layout() override;

private:
    [[nodiscard]] float max_scroll_offset() const noexcept;

private:
    float _content_height = 0.0f;
    float _scroll_offset = 0.0f;
    float _scroll_step = 24.0f;
};
}
