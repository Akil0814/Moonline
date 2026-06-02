#include "ui_layout.h"

#include <algorithm>

UiLayout::UiLayout(Vector2 position, Vector2 size, int order)
    : UiElement(position, size, order)
{
}

void UiLayout::set_world_position(const Vector2& position)
{
    GameObject::set_world_position(position);
    mark_dirty();
}

void UiLayout::set_size(const Vector2& size)
{
    GameObject::set_size(size);
    mark_dirty();
}

void UiLayout::add_child(const std::shared_ptr<GameObject>& child)
{
    add_child(child, UiLayoutChildOptions{});
}

void UiLayout::add_child(const std::shared_ptr<GameObject>& child, const UiLayoutChildOptions& options)
{
    if (!child)
    {
        return;
    }

    LayoutChild layout_child;
    layout_child._object = child;
    layout_child._base_size = child->size();
    layout_child._options = options;
    _children.push_back(std::move(layout_child));
    mark_dirty();
}

bool UiLayout::remove_child(const GameObject* child)
{
    const size_t original_size = _children.size();

    _children.erase(
        std::remove_if(_children.begin(), _children.end(),
            [child](const LayoutChild& item)
            {
                return item._object.get() == child;
            }),
        _children.end()
    );

    const bool removed = _children.size() != original_size;
    if (removed)
    {
        mark_dirty();
    }

    return removed;
}

void UiLayout::clear_children()
{
    _children.clear();
    mark_dirty();
}

bool UiLayout::set_child_options(const GameObject* child, const UiLayoutChildOptions& options)
{
    for (LayoutChild& layout_child : _children)
    {
        if (layout_child._object.get() != child)
        {
            continue;
        }

        layout_child._options = options;
        mark_dirty();
        return true;
    }

    return false;
}

bool UiLayout::try_get_child_options(const GameObject* child, UiLayoutChildOptions& out_options) const
{
    for (const LayoutChild& layout_child : _children)
    {
        if (layout_child._object.get() != child)
        {
            continue;
        }

        out_options = layout_child._options;
        return true;
    }

    return false;
}

size_t UiLayout::child_count() const
{
    return _children.size();
}

void UiLayout::set_spacing(float spacing)
{
    _spacing = std::max(0.0f, spacing);
    mark_dirty();
}

float UiLayout::spacing() const
{
    return _spacing;
}

void UiLayout::set_padding(float left, float top, float right, float bottom)
{
    UiLayoutPadding padding;
    padding.left = std::max(0.0f, left);
    padding.top = std::max(0.0f, top);
    padding.right = std::max(0.0f, right);
    padding.bottom = std::max(0.0f, bottom);
    set_padding(padding);
}

void UiLayout::set_padding(const UiLayoutPadding& padding)
{
    _padding = padding;
    _padding.left = std::max(0.0f, _padding.left);
    _padding.top = std::max(0.0f, _padding.top);
    _padding.right = std::max(0.0f, _padding.right);
    _padding.bottom = std::max(0.0f, _padding.bottom);
    mark_dirty();
}

const UiLayoutPadding& UiLayout::padding() const
{
    return _padding;
}

void UiLayout::set_anchor(UiLayoutAnchor anchor)
{
    _anchor = anchor;
    mark_dirty();
}

UiLayoutAnchor UiLayout::anchor() const
{
    return _anchor;
}

void UiLayout::set_direction(UiLayoutDirection direction)
{
    _direction = direction;
    mark_dirty();
}

UiLayoutDirection UiLayout::direction() const
{
    return _direction;
}

void UiLayout::set_cross_align(UiLayoutAlign align)
{
    _cross_align = align;
    mark_dirty();
}

UiLayoutAlign UiLayout::cross_align() const
{
    return _cross_align;
}

void UiLayout::set_transform(const UiLayoutTransform& transform)
{
    _transform = transform;
    _transform.scale.x = std::max(0.0f, _transform.scale.x);
    _transform.scale.y = std::max(0.0f, _transform.scale.y);
    mark_dirty();
}

const UiLayoutTransform& UiLayout::transform() const
{
    return _transform;
}

void UiLayout::set_transform_translation(const Vector2& translation)
{
    _transform.translation = translation;
    mark_dirty();
}

const Vector2& UiLayout::transform_translation() const
{
    return _transform.translation;
}

void UiLayout::set_transform_scale(const Vector2& scale)
{
    _transform.scale.x = std::max(0.0f, scale.x);
    _transform.scale.y = std::max(0.0f, scale.y);
    mark_dirty();
}

const Vector2& UiLayout::transform_scale() const
{
    return _transform.scale;
}

void UiLayout::set_content_offset(const Vector2& offset)
{
    _content_offset.x = std::max(0.0f, offset.x);
    _content_offset.y = std::max(0.0f, offset.y);
    mark_dirty();
}

const Vector2& UiLayout::content_offset() const
{
    return _content_offset;
}

void UiLayout::set_auto_size(bool auto_width, bool auto_height)
{
    _auto_width = auto_width;
    _auto_height = auto_height;
    mark_dirty();
}

bool UiLayout::auto_sizes_width() const
{
    return _auto_width;
}

bool UiLayout::auto_sizes_height() const
{
    return _auto_height;
}

Vector2 UiLayout::content_view_size() const
{
    return available_content_area();
}

Vector2 UiLayout::measure_content_size()
{
    sync_child_sizes();
    return content_size(available_content_area());
}

bool UiLayout::try_get_child_rect(const GameObject* child, SDL_Rect& out_rect) const
{
    for (const LayoutChild& layout_child : _children)
    {
        if (layout_child._object.get() != child)
        {
            continue;
        }

        out_rect = layout_child._object->rect();
        return true;
    }

    return false;
}

void UiLayout::relayout()
{
    apply_layout();
}

void UiLayout::on_update(double delta)
{
    refresh_theme_if_needed();
    remove_destroyed_children();
    apply_layout();

    std::vector<std::shared_ptr<GameObject>> child_objects;
    child_objects.reserve(_children.size());
    for (const LayoutChild& child : _children)
    {
        child_objects.push_back(child._object);
    }

    for (const std::shared_ptr<GameObject>& child_object : child_objects)
    {
        if (!child_object || child_object->is_destroyed())
        {
            continue;
        }

        if (!child_object->is_active())
        {
            continue;
        }

        child_object->on_update(child_object->scaled_delta(delta));
    }

    remove_destroyed_children();
}

void UiLayout::on_render(SDL_Renderer* renderer)
{
    refresh_theme_if_needed();
    remove_destroyed_children();
    apply_layout();

    std::vector<std::shared_ptr<GameObject>> child_objects;
    child_objects.reserve(_children.size());
    for (const LayoutChild& child : _children)
    {
        child_objects.push_back(child._object);
    }

    for (const std::shared_ptr<GameObject>& child_object : child_objects)
    {
        if (!child_object || child_object->is_destroyed())
        {
            continue;
        }

        if (!child_object->is_visible())
        {
            continue;
        }

        child_object->on_render(renderer);
    }
}

void UiLayout::on_input(const InputSnapshot& input)
{
    refresh_theme_if_needed();
    remove_destroyed_children();
    apply_layout();

    std::vector<std::shared_ptr<GameObject>> child_objects;
    child_objects.reserve(_children.size());
    for (const LayoutChild& child : _children)
    {
        child_objects.push_back(child._object);
    }

    for (const std::shared_ptr<GameObject>& child_object : child_objects)
    {
        if (!child_object || child_object->is_destroyed())
        {
            continue;
        }

        if (!child_object->is_active())
        {
            continue;
        }

        child_object->on_input(input);
    }
}

void UiLayout::on_input_event(const InputEvent& event)
{
    refresh_theme_if_needed();
    remove_destroyed_children();
    apply_layout();

    std::vector<std::shared_ptr<GameObject>> child_objects;
    child_objects.reserve(_children.size());
    for (const LayoutChild& child : _children)
    {
        child_objects.push_back(child._object);
    }

    for (const std::shared_ptr<GameObject>& child_object : child_objects)
    {
        if (!child_object || child_object->is_destroyed())
        {
            continue;
        }

        if (!child_object->is_active())
        {
            continue;
        }

        child_object->on_input_event(event);
    }
}

void UiLayout::reset()
{
    UiElement::reset();

    std::vector<std::shared_ptr<GameObject>> child_objects;
    child_objects.reserve(_children.size());
    for (const LayoutChild& child : _children)
    {
        child_objects.push_back(child._object);
    }

    for (const std::shared_ptr<GameObject>& child_object : child_objects)
    {
        if (child_object)
        {
            child_object->reset();
        }
    }

    mark_dirty();
    apply_layout();
}

void UiLayout::apply_theme(const UiTheme& theme)
{
    (void)theme;
}

void UiLayout::mark_dirty()
{
    _layout_dirty = true;
}

void UiLayout::sync_child_sizes()
{
    for (LayoutChild& child : _children)
    {
        if (!child._object)
        {
            continue;
        }

        const Vector2 current_size = child._object->size();
        if (!current_size.nearly_equals(child._applied_size))
        {
            child._base_size = current_size;
        }
    }
}

void UiLayout::remove_destroyed_children()
{
    const size_t original_size = _children.size();

    _children.erase(
        std::remove_if(_children.begin(), _children.end(),
            [](const LayoutChild& child)
            {
                return !child._object || child._object->is_destroyed();
            }),
        _children.end()
    );

    if (_children.size() != original_size)
    {
        mark_dirty();
    }
}

void UiLayout::apply_layout()
{
    if (!_layout_dirty)
    {
        return;
    }

    sync_child_sizes();

    const Vector2 available_area_before_auto_size = available_content_area();
    const Vector2 content_size_before_auto_size = content_size(available_area_before_auto_size);
    if (_auto_width || _auto_height)
    {
        Vector2 layout_size = size();
        if (_auto_width)
        {
            layout_size.x = content_size_before_auto_size.x + _padding.left + _padding.right;
        }

        if (_auto_height)
        {
            layout_size.y = content_size_before_auto_size.y + _padding.top + _padding.bottom;
        }

        GameObject::set_size(layout_size);
    }

    const Vector2 available_area = available_content_area();
    const Vector2 layout_content_size = content_size(available_area);
    const Vector2 start = content_origin(layout_content_size);
    const float spacing_x = _spacing * _transform.scale.x;
    const float spacing_y = _spacing * _transform.scale.y;

    float main_axis_cursor = 0.0f;
    for (LayoutChild& child : _children)
    {
        if (!child._object)
        {
            continue;
        }

        const Vector2 child_size = child_layout_size(child, available_area);
        const Vector2 child_outer_size = UiLayout::child_outer_size(child, available_area);
        const UiLayoutMargin& margin = child._options._margin;
        const UiLayoutAlign align = child._options._use_custom_cross_align
            ? child._options._cross_align
            : _cross_align;
        Vector2 child_world_position = start;

        if (_direction == UiLayoutDirection::Horizontal)
        {
            child_world_position.x += main_axis_cursor + margin.left;
            child_world_position.y += cross_axis_offset(
                layout_content_size.y,
                child_outer_size.y,
                align
            ) + margin.top;
            main_axis_cursor += child_outer_size.x + spacing_x;
        }
        else
        {
            child_world_position.x += cross_axis_offset(
                layout_content_size.x,
                child_outer_size.x,
                align
            ) + margin.left;
            child_world_position.y += main_axis_cursor + margin.top;
            main_axis_cursor += child_outer_size.y + spacing_y;
        }

        child._local_position = child_world_position - position();
        child._object->set_world_position(child_world_position);
        child._object->set_size(child_size);
        child._applied_size = child_size;
    }

    _layout_dirty = false;
}

Vector2 UiLayout::available_content_area() const
{
    const Vector2 layout_size = size();
    return {
        std::max(0.0f, layout_size.x - _padding.left - _padding.right),
        std::max(0.0f, layout_size.y - _padding.top - _padding.bottom)
    };
}

Vector2 UiLayout::child_layout_size(
    const LayoutChild& child,
    const Vector2& available_content_area
) const
{
    Vector2 base_size = child._options._use_size_override
        ? child._options._size_override
        : child._base_size;

    Vector2 child_size{
        std::max(0.0f, base_size.x * _transform.scale.x),
        std::max(0.0f, base_size.y * _transform.scale.y)
    };

    const UiLayoutMargin& margin = child._options._margin;
    if (child._options._fill_cross_axis)
    {
        if (_direction == UiLayoutDirection::Horizontal)
        {
            child_size.y = std::max(
                0.0f,
                available_content_area.y - margin.top - margin.bottom
            );
        }
        else
        {
            child_size.x = std::max(
                0.0f,
                available_content_area.x - margin.left - margin.right
            );
        }
    }

    return child_size;
}

Vector2 UiLayout::child_outer_size(
    const LayoutChild& child,
    const Vector2& available_content_area
) const
{
    const Vector2 child_size = child_layout_size(child, available_content_area);
    const UiLayoutMargin& margin = child._options._margin;
    return {
        child_size.x + margin.left + margin.right,
        child_size.y + margin.top + margin.bottom
    };
}

Vector2 UiLayout::content_size(const Vector2& available_content_area) const
{
    if (_children.empty())
    {
        return Vector2::zero();
    }

    float width = 0.0f;
    float height = 0.0f;
    size_t valid_child_count = 0;

    for (const LayoutChild& child : _children)
    {
        if (!child._object)
        {
            continue;
        }

        ++valid_child_count;
        const Vector2 child_size = child_outer_size(child, available_content_area);
        if (_direction == UiLayoutDirection::Horizontal)
        {
            width += child_size.x;
            height = std::max(height, child_size.y);
        }
        else
        {
            width = std::max(width, child_size.x);
            height += child_size.y;
        }
    }

    if (valid_child_count > 1)
    {
        const float gap_count = static_cast<float>(valid_child_count - 1);
        if (_direction == UiLayoutDirection::Horizontal)
        {
            width += gap_count * _spacing * _transform.scale.x;
        }
        else
        {
            height += gap_count * _spacing * _transform.scale.y;
        }
    }

    return { width, height };
}

Vector2 UiLayout::content_origin(const Vector2& content_size) const
{
    const Vector2 layout_position = position();
    const Vector2 layout_size = size();

    const float available_x = layout_position.x + _padding.left + _transform.translation.x;
    const float available_y = layout_position.y + _padding.top + _transform.translation.y;
    const float available_width = std::max(0.0f, layout_size.x - _padding.left - _padding.right);
    const float available_height = std::max(0.0f, layout_size.y - _padding.top - _padding.bottom);

    float x = available_x;
    float y = available_y;

    switch (_anchor)
    {
    case UiLayoutAnchor::TopLeft:
    case UiLayoutAnchor::CenterLeft:
    case UiLayoutAnchor::BottomLeft:
        break;

    case UiLayoutAnchor::TopCenter:
    case UiLayoutAnchor::Center:
    case UiLayoutAnchor::BottomCenter:
        x += (available_width - content_size.x) * 0.5f;
        break;

    case UiLayoutAnchor::TopRight:
    case UiLayoutAnchor::CenterRight:
    case UiLayoutAnchor::BottomRight:
        x += available_width - content_size.x;
        break;
    }

    switch (_anchor)
    {
    case UiLayoutAnchor::TopLeft:
    case UiLayoutAnchor::TopCenter:
    case UiLayoutAnchor::TopRight:
        break;

    case UiLayoutAnchor::CenterLeft:
    case UiLayoutAnchor::Center:
    case UiLayoutAnchor::CenterRight:
        y += (available_height - content_size.y) * 0.5f;
        break;

    case UiLayoutAnchor::BottomLeft:
    case UiLayoutAnchor::BottomCenter:
    case UiLayoutAnchor::BottomRight:
        y += available_height - content_size.y;
        break;
    }

    x -= _content_offset.x;
    y -= _content_offset.y;
    return { x, y };
}

float UiLayout::cross_axis_offset(
    float content_extent,
    float child_extent,
    UiLayoutAlign align
) const
{
    switch (align)
    {
    case UiLayoutAlign::Start:
        return 0.0f;

    case UiLayoutAlign::Center:
        return (content_extent - child_extent) * 0.5f;

    case UiLayoutAlign::End:
        return content_extent - child_extent;
    }

    return 0.0f;
}
