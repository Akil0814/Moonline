#include "ui_grid_layout.h"

#include <algorithm>

UiGridLayout::UiGridLayout(Vector2 position, Vector2 size, int order)
    : UiElement(position, size, order)
{
}

void UiGridLayout::set_world_position(const Vector2& position)
{
    GameObject::set_world_position(position);
    mark_dirty();
}

void UiGridLayout::set_size(const Vector2& size)
{
    GameObject::set_size(size);
    mark_dirty();
}

void UiGridLayout::add_child(const std::shared_ptr<GameObject>& child)
{
    if (!child)
    {
        return;
    }

    _children.push_back(child);
    mark_dirty();
}

void UiGridLayout::clear_children()
{
    _children.clear();
    mark_dirty();
}

void UiGridLayout::set_column_count(int column_count)
{
    _column_count = std::max(1, column_count);
    mark_dirty();
}

int UiGridLayout::column_count() const
{
    return _column_count;
}

void UiGridLayout::set_cell_spacing(float horizontal_spacing, float vertical_spacing)
{
    _horizontal_spacing = std::max(0.0f, horizontal_spacing);
    _vertical_spacing = std::max(0.0f, vertical_spacing);
    mark_dirty();
}

float UiGridLayout::horizontal_spacing() const
{
    return _horizontal_spacing;
}

float UiGridLayout::vertical_spacing() const
{
    return _vertical_spacing;
}

void UiGridLayout::set_padding(const LayoutPadding& padding)
{
    _padding.left = std::max(0.0f, padding.left);
    _padding.top = std::max(0.0f, padding.top);
    _padding.right = std::max(0.0f, padding.right);
    _padding.bottom = std::max(0.0f, padding.bottom);
    mark_dirty();
}

const LayoutPadding& UiGridLayout::padding() const
{
    return _padding;
}

void UiGridLayout::set_cell_align(LayoutAlign horizontal_align, LayoutAlign vertical_align)
{
    _horizontal_align = horizontal_align;
    _vertical_align = vertical_align;
    mark_dirty();
}

LayoutAlign UiGridLayout::horizontal_align() const
{
    return _horizontal_align;
}

LayoutAlign UiGridLayout::vertical_align() const
{
    return _vertical_align;
}

void UiGridLayout::on_update(double delta)
{
    refresh_theme_if_needed();
    apply_layout();

    for (const std::shared_ptr<GameObject>& child : _children)
    {
        if (!child || child->is_destroyed() || !child->is_active())
        {
            continue;
        }

        child->on_update(child->scaled_delta(delta));
    }
}

void UiGridLayout::on_render(SDL_Renderer* renderer)
{
    refresh_theme_if_needed();
    apply_layout();

    for (const std::shared_ptr<GameObject>& child : _children)
    {
        if (!child || child->is_destroyed() || !child->is_visible())
        {
            continue;
        }

        child->on_render(renderer);
    }
}

void UiGridLayout::on_input(const InputSnapshot& input)
{
    refresh_theme_if_needed();
    apply_layout();

    for (const std::shared_ptr<GameObject>& child : _children)
    {
        if (!child || child->is_destroyed() || !child->is_active())
        {
            continue;
        }

        child->on_input(input);
    }
}

void UiGridLayout::on_input_event(const InputEvent& event)
{
    refresh_theme_if_needed();
    apply_layout();

    for (const std::shared_ptr<GameObject>& child : _children)
    {
        if (!child || child->is_destroyed() || !child->is_active())
        {
            continue;
        }

        child->on_input_event(event);
    }
}

void UiGridLayout::reset()
{
    UiElement::reset();
    _padding = LayoutPadding{};
    _horizontal_spacing = 0.0f;
    _vertical_spacing = 0.0f;
    _column_count = 1;
    _horizontal_align = LayoutAlign::Start;
    _vertical_align = LayoutAlign::Start;
    _layout_dirty = true;

    for (const std::shared_ptr<GameObject>& child : _children)
    {
        if (child)
        {
            child->reset();
        }
    }
}

void UiGridLayout::relayout()
{
    apply_layout();
}

void UiGridLayout::mark_dirty()
{
    _layout_dirty = true;
}

void UiGridLayout::apply_layout()
{
    if (!_layout_dirty || _children.empty())
    {
        return;
    }

    const float inner_width = std::max(0.0f, size().x - _padding.left - _padding.right);
    const float inner_height = std::max(0.0f, size().y - _padding.top - _padding.bottom);
    const float column_gap_total = _horizontal_spacing * static_cast<float>(std::max(0, _column_count - 1));
    const float cell_width = _column_count > 0
        ? std::max(0.0f, (inner_width - column_gap_total) / static_cast<float>(_column_count))
        : inner_width;

    float current_y = position().y + _padding.top;
    int index = 0;
    while (index < static_cast<int>(_children.size()))
    {
        float row_height = 0.0f;
        for (int column = 0; column < _column_count && index + column < static_cast<int>(_children.size()); ++column)
        {
            const std::shared_ptr<GameObject>& child = _children[static_cast<size_t>(index + column)];
            if (!child)
            {
                continue;
            }

            row_height = std::max(row_height, child->size().y);
        }

        row_height = std::min(row_height, inner_height);

        for (int column = 0; column < _column_count && index < static_cast<int>(_children.size()); ++column, ++index)
        {
            const std::shared_ptr<GameObject>& child = _children[static_cast<size_t>(index)];
            if (!child)
            {
                continue;
            }

            const float cell_x = position().x + _padding.left + column * (cell_width + _horizontal_spacing);
            const float child_x = cell_x + aligned_offset(cell_width, child->size().x, _horizontal_align);
            const float child_y = current_y + aligned_offset(row_height, child->size().y, _vertical_align);
            child->set_world_position({ child_x, child_y });
        }

        current_y += row_height + _vertical_spacing;
    }

    _layout_dirty = false;
}

float UiGridLayout::aligned_offset(float cell_extent, float child_extent, LayoutAlign align) const
{
    switch (align)
    {
    case LayoutAlign::Start:
        return 0.0f;
    case LayoutAlign::Center:
        return (cell_extent - child_extent) * 0.5f;
    case LayoutAlign::End:
        return cell_extent - child_extent;
    }

    return 0.0f;
}

void UiGridLayout::apply_theme(const UiTheme& theme)
{
    (void)theme;
}
