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

    void set_content_size(const elysia::core::Vector2& content_size) noexcept;
    [[nodiscard]] elysia::core::Vector2 content_size() const noexcept;
    void set_content_width(float content_width) noexcept;
    [[nodiscard]] float content_width() const noexcept;
    void set_content_height(float content_height) noexcept;
    [[nodiscard]] float content_height() const noexcept;

    void set_scroll_offset(const elysia::core::Vector2& scroll_offset) noexcept;
    void set_scroll_offset(float scroll_offset) noexcept;
    [[nodiscard]] float scroll_offset() const noexcept;
    void set_scroll_offset_x(float scroll_offset_x) noexcept;
    [[nodiscard]] float scroll_offset_x() const noexcept;
    void set_scroll_offset_y(float scroll_offset_y) noexcept;
    [[nodiscard]] float scroll_offset_y() const noexcept;

    void set_scroll_step(const elysia::core::Vector2& scroll_step) noexcept;
    void set_scroll_step(float scroll_step) noexcept;
    [[nodiscard]] float scroll_step() const noexcept;
    void set_scroll_step_x(float scroll_step_x) noexcept;
    [[nodiscard]] float scroll_step_x() const noexcept;
    void set_scroll_step_y(float scroll_step_y) noexcept;
    [[nodiscard]] float scroll_step_y() const noexcept;

    void scroll_to_left() noexcept;
    void scroll_to_right() noexcept;
    void scroll_to_top() noexcept;
    void scroll_to_bottom() noexcept;

protected:
    void rebuild_layout() override;

private:
    [[nodiscard]] elysia::core::Vector2 effective_content_size() const noexcept;
    [[nodiscard]] elysia::core::Vector2 clamp_scroll_offset(const elysia::core::Vector2& scroll_offset) const noexcept;
    [[nodiscard]] float max_scroll_offset_x() const noexcept;
    [[nodiscard]] float max_scroll_offset_y() const noexcept;
    [[nodiscard]] float max_scroll_offset() const noexcept;

private:
    elysia::core::Vector2 _content_size{ 0.0f,0.0f };
    elysia::core::Vector2 _scroll_offset{ 0.0f,0.0f };
    elysia::core::Vector2 _scroll_step{ 24.0f,24.0f };
};
}
