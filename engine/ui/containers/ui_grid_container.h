#pragma once

#include "ui_container.h"

namespace elysia::ui
{
class UiGridContainer : public UiContainer
{
public:
    explicit UiGridContainer(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiGridContainer(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiGridContainer(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiGridContainer() override = default;

    void reset() noexcept override;

    void set_column_count(int column_count) noexcept;
    [[nodiscard]] int column_count() const noexcept;
    void set_cell_spacing(const elysia::core::Vector2& spacing) noexcept;
    [[nodiscard]] elysia::core::Vector2 cell_spacing() const noexcept;
    void set_fill_by_row(bool fill_by_row) noexcept;
    [[nodiscard]] bool fills_by_row() const noexcept;

protected:
    void rebuild_layout() override;

private:
    int _column_count = 1;
    elysia::core::Vector2 _cell_spacing{ 0.0f,0.0f };
    bool _fill_by_row = true;
};
}
