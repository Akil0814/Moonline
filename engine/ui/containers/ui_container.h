#pragma once

#include "../core/ui_element.h"
#include "ui_container_layout_types.h"
#include "../../core/interface/updatable.h"
#include "../input/contracts/ui_input_event_receiver.h"
#include "../input/contracts/ui_input_frame_receiver.h"

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace elysia::ui
{
class UiContainer : public UiElement, public elysia::core::Updatable, public UiInputFrameReceiver, public UiInputEventReceiver
{
public:
    struct ChildEntry
    {
        std::unique_ptr<UiElement> element;
        UiLayoutChildOptions layout;
    };

public:
    explicit UiContainer(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiContainer(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiContainer(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiContainer() override = default;

    void reset() noexcept override;

    UiElement* add_child(std::unique_ptr<UiElement> child,UiLayoutChildOptions options = {});

    template<class T,class... Args>
    T* create_child(Args&&... args)
    {
        static_assert(std::is_base_of_v<UiElement,T>);
        return create_child<T>(UiLayoutChildOptions{},std::forward<Args>(args)...);
    }

    template<class T,class... Args>
    T* create_child(const UiLayoutChildOptions& options,Args&&... args)
    {
        static_assert(std::is_base_of_v<UiElement,T>);
        std::unique_ptr<T> child = std::make_unique<T>(std::forward<Args>(args)...);
        T* child_ptr = child.get();
        return static_cast<T*>(add_child(std::move(child),options)) ? child_ptr : nullptr;
    }

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

    void mark_layout_dirty() noexcept;
    void update_layout_if_dirty();

    void update(double delta) override;
    void on_ui_input_frame(const UiInputFrame& input) override;
    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

protected:
    virtual void rebuild_layout();

    [[nodiscard]] elysia::core::Rect content_rect() const noexcept;
    [[nodiscard]] std::vector<ChildEntry>& children() noexcept;
    [[nodiscard]] const std::vector<ChildEntry>& children() const noexcept;

    void submit_child_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const;
    void apply_opacity_to_range(std::vector<elysia::core::UiRenderCommand>& out_commands,std::size_t begin) const;
    void apply_clip_to_range(std::vector<elysia::core::UiRenderCommand>& out_commands,std::size_t begin,const elysia::core::Rect& clip_rect) const;
    void finalize_child_command_range(
        std::vector<elysia::core::UiRenderCommand>& out_commands,
        std::size_t begin,
        const elysia::core::Rect& clip_rect
    ) const;

private:
    void cleanup_destroyed_children();
    [[nodiscard]] bool needs_layout_rebuild() const noexcept;
    static std::uint8_t multiply_alpha(std::uint8_t a,std::uint8_t b) noexcept;

private:
    std::vector<ChildEntry> _children;
    UiLayoutPadding _padding{};
    bool _clip_children = false;
    bool _layout_dirty = true;
    elysia::core::Rect _last_layout_rect{};
};
}

