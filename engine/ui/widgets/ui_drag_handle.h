#pragma once

#include "../../core/render/colors.h"
#include "../core/ui_control.h"

#include <functional>
#include <optional>

struct SDL_Texture;

namespace elysia::ui
{
enum class UiDragAxis
{
    None,
    Horizontal,
    Vertical,
    Free
};

struct UiDragHandleTextures
{
    SDL_Texture* idle = nullptr;
    SDL_Texture* focused = nullptr;
    SDL_Texture* dragging = nullptr;
    SDL_Texture* disabled = nullptr;
};

struct UiDragHandleConfig
{
    elysia::core::Vector2 size{ 18.0f,18.0f };
    UiDragAxis axis = UiDragAxis::Free;
    std::optional<elysia::core::Rect> drag_bounds = std::nullopt;
    std::optional<UiDragHandleTextures> textures = std::nullopt;
    bool draw_background = true;
    bool draw_border = true;
    elysia::core::Color idle_color = elysia::core::colors::sky_blue;
    elysia::core::Color focused_color = elysia::core::colors::royal_blue;
    elysia::core::Color dragging_color = elysia::core::colors::white;
    elysia::core::Color disabled_background_color = elysia::core::colors::gray_500;
    elysia::core::Color border_color = elysia::core::colors::sky_blue;
    elysia::core::Color disabled_border_color = elysia::core::colors::gray_500;
};

using UiDragHandleDraggedCallback = std::function<void(const elysia::core::Vector2& center)>;
using UiDragHandleDragEndedCallback = std::function<void(const elysia::core::Vector2& center)>;

class UiDragHandle : public UiControl
{
public:
    explicit UiDragHandle(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiDragHandle(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiDragHandle(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    UiDragHandle(const elysia::core::Rect& rect,const UiDragHandleConfig& config,int order = 0) noexcept;
    UiDragHandle(const elysia::core::Vector2& position,const elysia::core::Vector2& size,const UiDragHandleConfig& config,int order = 0) noexcept;
    UiDragHandle(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,const UiDragHandleConfig& config,int order = 0) noexcept;
    ~UiDragHandle() override = default;

    void reset() noexcept override;
    void set_enabled(bool enabled);
    void set_focused(bool focused);

    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

    void set_drag_handle_config(const UiDragHandleConfig& config);
    [[nodiscard]] const UiDragHandleConfig& drag_handle_config() const noexcept;

    void set_drag_axis(UiDragAxis axis) noexcept;
    [[nodiscard]] UiDragAxis drag_axis() const noexcept;

    void set_drag_bounds(const elysia::core::Rect& bounds) noexcept;
    void clear_drag_bounds() noexcept;
    [[nodiscard]] const std::optional<elysia::core::Rect>& drag_bounds() const noexcept;

    void set_on_dragged(UiDragHandleDraggedCallback on_dragged);
    void set_on_drag_ended(UiDragHandleDragEndedCallback on_drag_ended);

    void begin_drag_from_pointer(const elysia::core::Vector2& pointer) noexcept;
    void cancel_drag() noexcept;
    [[nodiscard]] bool is_dragging() const noexcept;

private:
    void apply_drag_handle_config(const UiDragHandleConfig& config);
    [[nodiscard]] bool can_receive_pointer() const noexcept;
    [[nodiscard]] bool contains_pointer(int mouse_x,int mouse_y) const noexcept;
    [[nodiscard]] bool is_primary_pointer_event(const UiInputEvent& event) const noexcept;
    [[nodiscard]] bool drag_to_pointer(int mouse_x,int mouse_y);
    void begin_drag_session(const elysia::core::Vector2& pointer) noexcept;
    [[nodiscard]] elysia::core::Rect clamped_rect(const elysia::core::Rect& rect) const noexcept;
    [[nodiscard]] SDL_Texture* current_state_texture() const noexcept;
    [[nodiscard]] elysia::core::Color current_background_color() const noexcept;
    [[nodiscard]] elysia::core::Color current_border_color() const noexcept;

private:
    UiDragHandleConfig _config{};
    UiDragHandleDraggedCallback _on_dragged;
    UiDragHandleDragEndedCallback _on_drag_ended;
    elysia::core::Vector2 _grab_offset{};
    bool _is_dragging = false;
};
}
