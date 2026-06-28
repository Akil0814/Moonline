#pragma once

#include "ui_container.h"

namespace elysia::ui
{
enum class UiListDirection
{
    Vertical,
    Horizontal
};

class UiListContainer : public UiContainer
{
public:
    explicit UiListContainer(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiListContainer(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiListContainer(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiListContainer() override = default;

    void reset() noexcept override;

    void set_direction(UiListDirection direction) noexcept;
    [[nodiscard]] UiListDirection direction() const noexcept;
    void set_item_spacing(float item_spacing) noexcept;
    [[nodiscard]] float item_spacing() const noexcept;

protected:
    void rebuild_layout() override;

private:
    UiListDirection _direction = UiListDirection::Vertical;
    float _item_spacing = 0.0f;
};
}
