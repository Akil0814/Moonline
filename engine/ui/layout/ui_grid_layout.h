#pragma once

#include "../ui_layout.h"

#include <memory>
#include <vector>

class UiGridLayout : public UiElement
{
public:
    explicit UiGridLayout(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void set_world_position(const Vector2& position);
    void set_size(const Vector2& size);

    void add_child(const std::shared_ptr<GameObject>& child);
    void clear_children();

    void set_column_count(int column_count);
    [[nodiscard]] int column_count() const;

    void set_cell_spacing(float horizontal_spacing, float vertical_spacing);
    [[nodiscard]] float horizontal_spacing() const;
    [[nodiscard]] float vertical_spacing() const;

    void set_padding(const UiLayoutPadding& padding);
    [[nodiscard]] const UiLayoutPadding& padding() const;

    void set_cell_align(UiLayoutAlign horizontal_align, UiLayoutAlign vertical_align);
    [[nodiscard]] UiLayoutAlign horizontal_align() const;
    [[nodiscard]] UiLayoutAlign vertical_align() const;

    void on_update(double delta) override;
    void on_render(SDL_Renderer* renderer) override;
    void on_input(const InputSnapshot& input) override;
    void on_input_event(const InputEvent& event) override;
    void reset() override;

    void relayout();

private:
    void mark_dirty();
    void apply_layout();
    [[nodiscard]] float aligned_offset(float cell_extent, float child_extent, UiLayoutAlign align) const;
    void apply_theme(const UiTheme& theme) override;

private:
    std::vector<std::shared_ptr<GameObject>> _children;
    UiLayoutPadding _padding;

    float _horizontal_spacing = 0.0f;
    float _vertical_spacing = 0.0f;

    int _column_count = 1;

    bool _layout_dirty = true;

    UiLayoutAlign _horizontal_align = UiLayoutAlign::Start;
    UiLayoutAlign _vertical_align = UiLayoutAlign::Start;
};
