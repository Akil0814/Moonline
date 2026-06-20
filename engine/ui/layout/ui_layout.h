/*#pragma once

#include "../core/ui_element.h"

#include <array>
#include <memory>
#include <vector>



class UiLayout : public UiElement
{
public:
    explicit UiLayout(elysia::core::Vector2 position = elysia::core::Vector2::zero(), elysia::core::Vector2 size = elysia::core::Vector2::zero(), int order = 0);

    void set_world_position(const elysia::core::Vector2& position) override;
    void set_size(const elysia::core::Vector2& size) override;

    void add_child(const std::shared_ptr<elysia::core::GameObject>& child);
    void add_child(const std::shared_ptr<elysia::core::GameObject>& child, const UiLayoutChildOptions& options);
    bool remove_child(const elysia::core::GameObject* child);
    void clear_children();
    bool set_child_options(const elysia::core::GameObject* child, const UiLayoutChildOptions& options);
    bool try_get_child_options(const elysia::core::GameObject* child, UiLayoutChildOptions& out_options) const;

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

    void set_transform_translation(const elysia::core::Vector2& translation);
    [[nodiscard]] const elysia::core::Vector2& transform_translation() const;

    void set_transform_scale(const elysia::core::Vector2& scale);
    [[nodiscard]] const elysia::core::Vector2& transform_scale() const;

    void set_content_offset(const elysia::core::Vector2& offset);
    [[nodiscard]] const elysia::core::Vector2& content_offset() const;

    void set_auto_size(bool auto_width, bool auto_height);
    [[nodiscard]] bool auto_sizes_width() const;
    [[nodiscard]] bool auto_sizes_height() const;

    [[nodiscard]] elysia::core::Vector2 content_view_size() const;
    elysia::core::Vector2 measure_content_size();
    bool try_get_child_rect(const elysia::core::GameObject* child, SDL_Rect& out_rect) const;

    void relayout();

    void on_update(double delta) override;
    void on_render(SDL_Renderer* renderer) override;
    void on_input(const elysia::input::InputSnapshot& input) override;
    void on_input_event(const elysia::input::InputEvent& event) override;
    void reset() override;

private:
    struct LayoutChild
    {
        std::shared_ptr<elysia::core::GameObject> _object;
        elysia::core::Vector2 _base_size{ 0.0f, 0.0f };
        elysia::core::Vector2 _applied_size{ -1.0f, -1.0f };
        elysia::core::Vector2 _local_position{ 0.0f, 0.0f };
        UiLayoutChildOptions _options;
    };

private:
    void mark_dirty();
    void remove_destroyed_children();
    void sync_child_sizes();
    void apply_layout();
    [[nodiscard]] std::vector<std::shared_ptr<elysia::core::GameObject>> child_objects() const;

    [[nodiscard]] elysia::core::Vector2 available_content_area() const;
    [[nodiscard]] elysia::core::Vector2 child_layout_size(
        const LayoutChild& child,
        const elysia::core::Vector2& available_content_area
    ) const;
    [[nodiscard]] elysia::core::Vector2 child_outer_size(
        const LayoutChild& child,
        const elysia::core::Vector2& available_content_area
    ) const;
    [[nodiscard]] elysia::core::Vector2 content_size(const elysia::core::Vector2& available_content_area) const;
    [[nodiscard]] elysia::core::Vector2 content_origin(const elysia::core::Vector2& content_size) const;
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
    elysia::core::Vector2 _content_offset{ 0.0f, 0.0f };

    bool _auto_width = false;
    bool _auto_height = false;
    bool _layout_dirty = true;
};*/
namespace elysia::ui
{

}
