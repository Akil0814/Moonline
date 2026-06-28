#pragma once

#include "../containers/ui_container_layout_types.h"
#include "../core/ui_control.h"
#include "../core/ui_element.h"
#include "../input/contracts/ui_input_event_receiver.h"
#include "../input/contracts/ui_input_frame_receiver.h"
#include "../../core/interface/updatable.h"
#include "../../core/render/colors.h"

#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace elysia::ui
{
enum class UiWindowFocusSlot
{
    Custom,
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

struct UiWindowFocusNeighbors
{
    UiControl* up = nullptr;
    UiControl* down = nullptr;
    UiControl* left = nullptr;
    UiControl* right = nullptr;
};

struct UiWindowFocusOptions
{
    UiWindowFocusSlot slot = UiWindowFocusSlot::Custom;
    UiWindowFocusNeighbors neighbors;
};

using UiWindowCancelCallback = std::function<void()>;

class UiWindow : public UiElement, public elysia::core::Updatable, public UiInputFrameReceiver, public UiInputEventReceiver
{
public:
    struct ChildEntry
    {
        std::unique_ptr<UiElement> element;
        UiLayoutChildOptions layout;
    };

public:
    explicit UiWindow(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiWindow(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiWindow(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiWindow() override = default;

    void reset() noexcept override;

    UiElement* add_child(std::unique_ptr<UiElement> child,UiLayoutChildOptions options = {});

    template<class T,class... Args,
        std::enable_if_t<!(sizeof...(Args) > 0
            && std::is_same_v<
                std::decay_t<std::tuple_element_t<0,std::tuple<Args...>>>,
                UiLayoutChildOptions
            >),int> = 0>
    T* create_child(Args&&... args)
    {
        static_assert(std::is_base_of_v<UiElement,T>);
        std::unique_ptr<T> child = std::make_unique<T>(std::forward<Args>(args)...);
        T* child_ptr = child.get();
        return static_cast<T*>(add_child(std::move(child),UiLayoutChildOptions{})) ? child_ptr : nullptr;
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
    void set_draw_background(bool draw_background) noexcept;
    [[nodiscard]] bool draws_background() const noexcept;
    void set_draw_border(bool draw_border) noexcept;
    [[nodiscard]] bool draws_border() const noexcept;
    void set_background_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color background_color() const noexcept;
    void set_border_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color border_color() const noexcept;
    void set_hover_focus_enabled(bool enabled) noexcept;
    [[nodiscard]] bool hover_focus_enabled() const noexcept;
    void set_on_cancel(UiWindowCancelCallback on_cancel);

    void register_focus_target(UiControl& control,const UiWindowFocusOptions& options = {});
    void unregister_focus_target(UiControl& control);
    void set_focus_neighbors(UiControl& control,const UiWindowFocusNeighbors& neighbors);
    void set_focused_target(UiControl* control);
    [[nodiscard]] UiControl* focused_target() const noexcept;
    bool focus_first_available();

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
    struct FocusEntry
    {
        UiControl* control = nullptr;
        UiWindowFocusOptions options;
    };

private:
    void cleanup_destroyed_children();
    void prune_focus_targets();
    void ensure_valid_focus();
    void apply_focus_state();
    void update_child_objects(double delta);
    void dispatch_frame_to_children(const UiInputFrame& input);
    bool dispatch_input_to_children(const UiInputEvent& event);
    [[nodiscard]] UiControl* find_registered_target_at(int mouse_x,int mouse_y) const;
    [[nodiscard]] UiControl* find_neighbor(const UiControl& control,UiAction action) const;
    [[nodiscard]] bool is_registered_focus_target(const UiControl& control) const noexcept;
    [[nodiscard]] bool set_focused_target_internal(UiControl* control) noexcept;
    [[nodiscard]] bool needs_layout_rebuild() const noexcept;
    [[nodiscard]] static bool is_control_usable(const UiControl* control) noexcept;

private:
    std::vector<ChildEntry> _children;
    std::vector<FocusEntry> _focus_entries;
    UiControl* _focused_target = nullptr;
    UiLayoutPadding _padding{};
    bool _clip_children = false;
    bool _layout_dirty = true;
    elysia::core::Rect _last_layout_rect{};
    bool _draw_background = false;
    bool _draw_border = false;
    elysia::core::Color _background_color = elysia::core::colors::cobalt_blue;
    elysia::core::Color _border_color = elysia::core::colors::sky_blue;
    bool _hover_focus_enabled = true;
    UiWindowCancelCallback _on_cancel;
};
}
