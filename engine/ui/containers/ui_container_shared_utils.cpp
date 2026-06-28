#include "ui_container_shared_utils.h"

#include <algorithm>

namespace elysia::ui::container_utils
{
namespace
{
[[nodiscard]] float horizontal_margin_offset(UiLayoutAnchor anchor,const UiLayoutMargin& margin) noexcept
{
    switch (anchor)
    {
    case UiLayoutAnchor::TopLeft:
    case UiLayoutAnchor::CenterLeft:
    case UiLayoutAnchor::BottomLeft:
        return margin.left;
    case UiLayoutAnchor::TopCenter:
    case UiLayoutAnchor::Center:
    case UiLayoutAnchor::BottomCenter:
        return margin.left - margin.right;
    case UiLayoutAnchor::TopRight:
    case UiLayoutAnchor::CenterRight:
    case UiLayoutAnchor::BottomRight:
        return -margin.right;
    default:
        return 0.0f;
    }
}

[[nodiscard]] float vertical_margin_offset(UiLayoutAnchor anchor,const UiLayoutMargin& margin) noexcept
{
    switch (anchor)
    {
    case UiLayoutAnchor::TopLeft:
    case UiLayoutAnchor::TopCenter:
    case UiLayoutAnchor::TopRight:
        return margin.top;
    case UiLayoutAnchor::CenterLeft:
    case UiLayoutAnchor::Center:
    case UiLayoutAnchor::CenterRight:
        return margin.top - margin.bottom;
    case UiLayoutAnchor::BottomLeft:
    case UiLayoutAnchor::BottomCenter:
    case UiLayoutAnchor::BottomRight:
        return -margin.bottom;
    default:
        return 0.0f;
    }
}

[[nodiscard]] std::uint8_t multiply_alpha(std::uint8_t a,std::uint8_t b) noexcept
{
    return static_cast<std::uint8_t>((static_cast<unsigned int>(a) * static_cast<unsigned int>(b)) / 255U);
}
}

elysia::core::Vector2 clamp_size(const elysia::core::Vector2& size) noexcept
{
    return elysia::core::Vector2(std::max(0.0f,size.x),std::max(0.0f,size.y));
}

elysia::core::Rect anchored_rect(
    const elysia::core::Rect& bounds,
    const elysia::core::Vector2& size,
    UiLayoutAnchor anchor,
    const UiLayoutMargin& margin
) noexcept
{
    const elysia::core::Vector2 clamped_size = clamp_size(size);
    elysia::core::Rect rect = elysia::core::Rect::from_center(bounds.center(),clamped_size);

    switch (anchor)
    {
    case UiLayoutAnchor::TopLeft:
        rect.set_position(bounds.top_left());
        break;
    case UiLayoutAnchor::TopCenter:
        rect.set_position(elysia::core::Vector2(bounds.center().x - clamped_size.x * 0.5f,bounds.top()));
        break;
    case UiLayoutAnchor::TopRight:
        rect.set_position(elysia::core::Vector2(bounds.right() - clamped_size.x,bounds.top()));
        break;
    case UiLayoutAnchor::CenterLeft:
        rect.set_position(elysia::core::Vector2(bounds.left(),bounds.center().y - clamped_size.y * 0.5f));
        break;
    case UiLayoutAnchor::Center:
        rect.set_center(bounds.center());
        break;
    case UiLayoutAnchor::CenterRight:
        rect.set_position(elysia::core::Vector2(bounds.right() - clamped_size.x,bounds.center().y - clamped_size.y * 0.5f));
        break;
    case UiLayoutAnchor::BottomLeft:
        rect.set_position(elysia::core::Vector2(bounds.left(),bounds.bottom() - clamped_size.y));
        break;
    case UiLayoutAnchor::BottomCenter:
        rect.set_position(elysia::core::Vector2(bounds.center().x - clamped_size.x * 0.5f,bounds.bottom() - clamped_size.y));
        break;
    case UiLayoutAnchor::BottomRight:
        rect.set_position(elysia::core::Vector2(bounds.right() - clamped_size.x,bounds.bottom() - clamped_size.y));
        break;
    }

    rect.set_position(elysia::core::Vector2(
        rect.x() + horizontal_margin_offset(anchor,margin),
        rect.y() + vertical_margin_offset(anchor,margin)
    ));
    return rect;
}

elysia::core::Rect padded_content_rect(const elysia::core::Rect& rect,const UiLayoutPadding& padding) noexcept
{
    const float width = std::max(0.0f,rect.width() - padding.left - padding.right);
    const float height = std::max(0.0f,rect.height() - padding.top - padding.bottom);
    return elysia::core::Rect(rect.x() + padding.left,rect.y() + padding.top,width,height);
}

void apply_opacity_to_range(
    std::vector<elysia::core::UiRenderCommand>& out_commands,
    std::size_t begin,
    std::uint8_t opacity
) noexcept
{
    if (opacity == 255)
        return;

    for (std::size_t index = begin; index < out_commands.size(); ++index)
    {
        elysia::core::UiRenderCommand& command = out_commands[index];
        switch (command.type)
        {
        case elysia::core::UiRenderCommandType::Texture:
            command.alpha = multiply_alpha(command.alpha,opacity);
            break;
        case elysia::core::UiRenderCommandType::FillRect:
        case elysia::core::UiRenderCommandType::DrawRect:
        case elysia::core::UiRenderCommandType::DrawLine:
            command.color.a = multiply_alpha(command.color.a,opacity);
            break;
        default:
            break;
        }
    }
}

void apply_clip_to_range(
    std::vector<elysia::core::UiRenderCommand>& out_commands,
    std::size_t begin,
    const elysia::core::Rect& clip_rect
)
{
    if (clip_rect.is_empty())
    {
        out_commands.erase(out_commands.begin() + static_cast<std::ptrdiff_t>(begin),out_commands.end());
        return;
    }

    for (std::size_t index = out_commands.size(); index > begin; --index)
    {
        elysia::core::UiRenderCommand& command = out_commands[index - 1];
        const elysia::core::Rect final_clip = command.use_clip_rect ? command.clip_rect.intersection(clip_rect) : clip_rect;
        if (final_clip.is_empty())
        {
            out_commands.erase(out_commands.begin() + static_cast<std::ptrdiff_t>(index - 1));
            continue;
        }

        command.use_clip_rect = true;
        command.clip_rect = final_clip;
    }
}

void finalize_child_command_range(
    std::vector<elysia::core::UiRenderCommand>& out_commands,
    std::size_t begin,
    std::uint8_t opacity,
    bool clip_children,
    const elysia::core::Rect& clip_rect
)
{
    if (begin >= out_commands.size())
        return;

    apply_opacity_to_range(out_commands,begin,opacity);
    if (clip_children)
        apply_clip_to_range(out_commands,begin,clip_rect);
}
}
