#pragma once

#include "ui_element.h"
#include "../layout/ui_layout_types.h"
#include "../../core/interface/updatable.h"
#include "../input/contracts/ui_input_event_receiver.h"
#include "../input/contracts/ui_input_frame_receiver.h"

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace elysia::ui
{
class UiChildHost : public UiElement, public elysia::core::Updatable, public UiInputFrameReceiver, public UiInputEventReceiver
{
    friend class UiElement;

public:
    // Owns a child element together with the layout options used to place it.
    struct ChildEntry
    {
        std::unique_ptr<UiElement> element;
        UiLayoutChildOptions layout;
    };

public:
    explicit UiChildHost(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiChildHost(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiChildHost(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiChildHost() override = default;

    void reset() noexcept override;

    // Adopts a child element and registers it with this host's layout tree.
    virtual UiElement* add_child(std::unique_ptr<UiElement> child,UiLayoutChildOptions options = {});

    template<class T,class... Args,
        std::enable_if_t<!(sizeof...(Args) > 0
            && std::is_same_v<
                std::decay_t<std::tuple_element_t<0,std::tuple<Args...>>>,
                UiLayoutChildOptions
            >),int> = 0>
    T* create_child(Args&&... args)
    {
        static_assert(std::is_base_of_v<UiElement,T>);
        // Creates and adopts a child in one step, returning a non-owning pointer if insertion succeeds.
        std::unique_ptr<T> child = std::make_unique<T>(std::forward<Args>(args)...);
        T* child_ptr = child.get();
        return static_cast<T*>(add_child(std::move(child),UiLayoutChildOptions{})) ? child_ptr : nullptr;
    }

    template<class T,class... Args>
    T* create_child(const UiLayoutChildOptions& options,Args&&... args)
    {
        static_assert(std::is_base_of_v<UiElement,T>);
        // Creates and adopts a child with explicit layout options applied at insertion time.
        std::unique_ptr<T> child = std::make_unique<T>(std::forward<Args>(args)...);
        T* child_ptr = child.get();
        return static_cast<T*>(add_child(std::move(child),options)) ? child_ptr : nullptr;
    }

    // Releases all child ownership and detaches their layout-parent links.
    void clear_children();
    [[nodiscard]] std::size_t child_count() const noexcept;
    [[nodiscard]] UiElement* child_at(std::size_t index) noexcept;
    [[nodiscard]] const UiElement* child_at(std::size_t index) const noexcept;
    void set_child_layout_options(std::size_t index,const UiLayoutChildOptions& options);
    [[nodiscard]] const UiLayoutChildOptions* child_layout_options(std::size_t index) const noexcept;

    void set_padding(const UiLayoutPadding& padding) noexcept;
    [[nodiscard]] const UiLayoutPadding& padding() const noexcept;
    void set_clip_children(bool clip_children) noexcept;
    [[nodiscard]] bool clips_children() const noexcept;

    // Marks child layout as stale so the next update or render can rebuild placement.
    void mark_layout_dirty() noexcept;
    // Rebuilds child layout on demand when bounds, padding, or child options changed.
    void update_layout_if_dirty();

    void update(double delta) override;
    void on_ui_input_frame(const UiInputFrame& input) override;
    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

protected:
    // Repositions children inside the current content rect.
    virtual void rebuild_layout();
    // Bubbles intrinsic size changes up to the parent layout host.
    void invalidate_intrinsic_layout() noexcept;
    // Hook for derived hosts that need to react when a child changes its intrinsic size.
    virtual void on_child_intrinsic_layout_invalidated(UiElement& child) noexcept;

    [[nodiscard]] elysia::core::Rect content_rect() const noexcept;
    [[nodiscard]] std::vector<ChildEntry>& children() noexcept;
    [[nodiscard]] const std::vector<ChildEntry>& children() const noexcept;
    // Inserts a child at a specific z/input order while preserving host ownership.
    UiElement* insert_child(std::unique_ptr<UiElement> child,std::size_t index,UiLayoutChildOptions options = {});

    // Appends child draw commands in child order before host-level clipping or opacity is applied.
    void submit_child_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const;
    // Applies this host's opacity to commands emitted by children after begin.
    void apply_opacity_to_range(std::vector<elysia::core::UiRenderCommand>& out_commands,std::size_t begin) const;
    // Clips child commands to the host-visible region when clipping is enabled.
    void apply_clip_to_range(std::vector<elysia::core::UiRenderCommand>& out_commands,std::size_t begin,const elysia::core::Rect& clip_rect) const;
    // Finalizes a child command range with the host's opacity and clipping policy.
    void finalize_child_command_range(
        std::vector<elysia::core::UiRenderCommand>& out_commands,
        std::size_t begin,
        const elysia::core::Rect& clip_rect
    ) const;
    // Updates child scene objects while preserving host-controlled child ownership.
    void update_child_objects(double delta);
    // Broadcasts the frame-level input snapshot to children before event routing.
    void dispatch_frame_to_children(const UiInputFrame& input);
    // Routes a discrete UI event to children until one of them consumes it.
    bool dispatch_input_to_children(const UiInputEvent& event);
    // Drops destroyed children from the host without disturbing surviving entries.
    void cleanup_destroyed_children();
    [[nodiscard]] bool needs_layout_rebuild() const noexcept;

private:
    // Wires a newly adopted child into the intrinsic-layout invalidation tree.
    void attach_child_to_layout_tree(UiElement& child) noexcept;
    // Removes one child from the invalidation tree without changing ownership elsewhere.
    void detach_child_from_layout_tree(UiElement* child) noexcept;
    // Clears layout-parent links before bulk child removal or host reset.
    void detach_all_children_from_layout_tree() noexcept;

    std::vector<ChildEntry> _children;
    UiLayoutPadding _padding{};
    bool _clip_children = false;
    bool _layout_dirty = true;
    bool _is_rebuilding_layout = false;
    bool _layout_dirty_after_rebuild = false;
    elysia::core::Rect _last_layout_rect{};
};
}
