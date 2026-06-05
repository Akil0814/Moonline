#pragma once

#include "../core/ui_element.h"

#include <array>
#include <memory>
#include <vector>

enum class UiLayoutAnchor
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

enum class UiLayoutDirection
{
    Horizontal,
    Vertical
};

enum class UiLayoutAlign
{
    Start,
    Center,
    End
};

struct UiLayoutPadding
{
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
};

struct UiLayoutMargin
{
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
};

struct UiLayoutChildOptions
{
    UiLayoutMargin _margin;
    UiLayoutAlign _cross_align = UiLayoutAlign::Start;
    Vector2 _size_override{ 0.0f, 0.0f };

    bool _use_custom_cross_align = false;
    bool _fill_cross_axis = false;
    bool _use_size_override = false;
};

struct UiLayoutTransform
{
    Vector2 translation{ 0.0f, 0.0f };
    Vector2 scale{ 1.0f, 1.0f };
};

class UiLayout : public UiElement
{
public:
    explicit UiLayout(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void set_world_position(const Vector2& position) override;
    void set_size(const Vector2& size) override;

    void add_child(const std::shared_ptr<GameObject>& child);
    void add_child(const std::shared_ptr<GameObject>& child, const UiLayoutChildOptions& options);
    bool remove_child(const GameObject* child);
    void clear_children();
    bool set_child_options(const GameObject* child, const UiLayoutChildOptions& options);
    bool try_get_child_options(const GameObject* child, UiLayoutChildOptions& out_options) const;

    [[nodiscard]] size_t child_count() const;

    void set_spacing(float spacing);
    [[nodiscard]] float spacing() const;

    void set_padding(float left, float top, float right, float bottom);
    void set_padding(const UiLayoutPadding& padding);
    [[nodiscard]] const UiLayoutPadding& padding() const;

    void set_anchor(UiLayoutAnchor anchor);
    [[nodiscard]] UiLayoutAnchor anchor() const;

    void set_direction(UiLayoutDirection direction);
    [[nodiscard]] UiLayoutDirection direction() const;

    void set_cross_align(UiLayoutAlign align);
    [[nodiscard]] UiLayoutAlign cross_align() const;

    void set_transform(const UiLayoutTransform& transform);
    [[nodiscard]] const UiLayoutTransform& transform() const;

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
        UiLayoutChildOptions _options;
    };

private:
    void mark_dirty();
    void remove_destroyed_children();
    void sync_child_sizes();
    void apply_layout();
    [[nodiscard]] std::vector<std::shared_ptr<GameObject>> child_objects() const;

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
        UiLayoutAlign align
    ) const;
    void apply_theme(const UiTheme& theme) override;

private:
    std::vector<LayoutChild> _children;

    float _spacing = 0.0f;
    UiLayoutPadding _padding;
    UiLayoutAnchor _anchor = UiLayoutAnchor::TopLeft;
    UiLayoutDirection _direction = UiLayoutDirection::Vertical;
    UiLayoutAlign _cross_align = UiLayoutAlign::Start;
    UiLayoutTransform _transform;
    Vector2 _content_offset{ 0.0f, 0.0f };

    bool _auto_width = false;
    bool _auto_height = false;
    bool _layout_dirty = true;
};
