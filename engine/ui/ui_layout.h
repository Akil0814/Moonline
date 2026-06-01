#pragma once

#include "../core/game_object.h"

#include <array>
#include <memory>
#include <vector>

enum class LayoutAnchor
{
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

enum class LayoutDirection
{
    Horizontal,
    Vertical
};

enum class LayoutAlign
{
    Start,
    Center,
    End
};

struct LayoutPadding
{
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
};

struct LayoutMargin
{
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
};

struct LayoutChildOptions
{
    LayoutMargin _margin;
    LayoutAlign _cross_align = LayoutAlign::Start;
    Vector2 _size_override{ 0.0f, 0.0f };

    bool _use_custom_cross_align = false;
    bool _fill_cross_axis = false;
    bool _use_size_override = false;
};

struct UILayoutTransform
{
    Vector2 translation{ 0.0f, 0.0f };
    Vector2 scale{ 1.0f, 1.0f };
};

class UILayout : public GameObject
{
public:
    explicit UILayout(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void set_world_position(const Vector2& position);
    void set_size(const Vector2& size);

    void add_child(const std::shared_ptr<GameObject>& child);
    void add_child(const std::shared_ptr<GameObject>& child, const LayoutChildOptions& options);
    bool remove_child(const GameObject* child);
    void clear_children();
    bool set_child_options(const GameObject* child, const LayoutChildOptions& options);
    bool try_get_child_options(const GameObject* child, LayoutChildOptions& out_options) const;

    [[nodiscard]] size_t child_count() const;

    void set_spacing(float spacing);
    [[nodiscard]] float spacing() const;

    void set_padding(float left, float top, float right, float bottom);
    void set_padding(const LayoutPadding& padding);
    [[nodiscard]] const LayoutPadding& padding() const;

    void set_anchor(LayoutAnchor anchor);
    [[nodiscard]] LayoutAnchor anchor() const;

    void set_direction(LayoutDirection direction);
    [[nodiscard]] LayoutDirection direction() const;

    void set_cross_align(LayoutAlign align);
    [[nodiscard]] LayoutAlign cross_align() const;

    void set_transform(const UILayoutTransform& transform);
    [[nodiscard]] const UILayoutTransform& transform() const;

    void set_transform_translation(const Vector2& translation);
    [[nodiscard]] const Vector2& transform_translation() const;

    void set_transform_scale(const Vector2& scale);
    [[nodiscard]] const Vector2& transform_scale() const;

    void set_content_offset(const Vector2& offset);
    [[nodiscard]] const Vector2& content_offset() const;

    void set_auto_size(bool auto_width, bool auto_height);
    [[nodiscard]] bool auto_sizes_width() const;
    [[nodiscard]] bool auto_sizes_height() const;

    [[nodiscard]] Vector2 content_view_size() const;
    Vector2 measure_content_size();
    bool try_get_child_rect(const GameObject* child, SDL_Rect& out_rect) const;

    void relayout();

    void on_update(double delta) override;
    void on_render(SDL_Renderer* renderer) override;
    void on_input(const InputSnapshot& input) override;
    void on_input_event(const InputEvent& event) override;
    void reset() override;

private:
    struct LayoutChild
    {
        std::shared_ptr<GameObject> _object;
        Vector2 _base_size{ 0.0f, 0.0f };
        Vector2 _applied_size{ -1.0f, -1.0f };
        Vector2 _local_position{ 0.0f, 0.0f };
        LayoutChildOptions _options;
    };

private:
    void mark_dirty();
    void remove_destroyed_children();
    void sync_child_sizes();
    void apply_layout();

    [[nodiscard]] Vector2 available_content_area() const;
    [[nodiscard]] Vector2 child_layout_size(
        const LayoutChild& child,
        const Vector2& available_content_area
    ) const;
    [[nodiscard]] Vector2 child_outer_size(
        const LayoutChild& child,
        const Vector2& available_content_area
    ) const;
    [[nodiscard]] Vector2 content_size(const Vector2& available_content_area) const;
    [[nodiscard]] Vector2 content_origin(const Vector2& content_size) const;
    [[nodiscard]] float cross_axis_offset(
        float content_extent,
        float child_extent,
        LayoutAlign align
    ) const;

private:
    std::vector<LayoutChild> _children;

    float _spacing = 0.0f;
    LayoutPadding _padding;
    LayoutAnchor _anchor = LayoutAnchor::TopLeft;
    LayoutDirection _direction = LayoutDirection::Vertical;
    LayoutAlign _cross_align = LayoutAlign::Start;
    UILayoutTransform _transform;
    Vector2 _content_offset{ 0.0f, 0.0f };

    bool _auto_width = false;
    bool _auto_height = false;
    bool _layout_dirty = true;
};
