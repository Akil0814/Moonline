#include "ui_animation.h"

#include "../../../assist/engine_assist_cache.h"
#include "../../../animation/animation_manager.h"
#include "../../../core/render/render_command.h"

namespace elysia::ui
{
UiAnimation::UiAnimation(const elysia::core::Rect& rect, int order)
    : UiElement(rect, order)
{
}

UiAnimation::UiAnimation(
    std::string_view animation_key,
    const elysia::core::Vector2& position,
    const elysia::core::Vector2& size,
    int order
)
    : UiElement(position, size, order)
{
    set_animation_key(animation_key);
}

UiAnimation::UiAnimation(std::string_view animation_key, const elysia::core::Rect& rect, int order)
    : UiElement(rect, order)
{
    set_animation_key(animation_key);
}

UiAnimation::UiAnimation(
    std::string_view animation_key,
    const elysia::core::Vector2& center,
    const elysia::core::Vector2& size,
    UiFromCenterTag,
    int order
)
    : UiElement(center, size, from_center, order)
{
    set_animation_key(animation_key);
}

bool UiAnimation::set_animation_key(std::string_view animation_key)
{
    std::unique_ptr<elysia::animation::Animation> animation =
        elysia::animation::AnimationManager::instance()->create_animation(animation_key);
    if (!animation)
    {
        _animation_key.clear();
        _animation.reset();
        _default_loop.reset();
        return false;
    }

    if (_loop_override)
        animation->set_loop(*_loop_override);

    _animation_key = animation_key;
    _animation = std::move(animation);
    const auto* definition = elysia::animation::AnimationManager::instance()
        ->find_definition(animation_key);
    _default_loop = definition ? std::optional<bool>(definition->loop) : std::nullopt;
    _animation->reset();
    return true;
}

bool UiAnimation::set_engine_animation(
    const elysia::assist::EngineAssistCache& engine_assist_cache,
    std::string_view animation_key)
{
    std::unique_ptr<elysia::animation::Animation> animation =
        engine_assist_cache.create_animation(animation_key);
    const auto* definition = engine_assist_cache.find_animation(animation_key);
    if (!animation || !definition)
    {
        _animation_key.clear();
        _animation.reset();
        _default_loop.reset();
        return false;
    }

    if (_loop_override)
        animation->set_loop(*_loop_override);

    _animation_key = animation_key;
    _animation = std::move(animation);
    _default_loop = definition->loop;
    _animation->reset();
    return true;
}

const std::string& UiAnimation::animation_key() const noexcept
{
    return _animation_key;
}

void UiAnimation::set_loop(bool loop)
{
    _loop_override = loop;
    if (_animation)
        _animation->set_loop(loop);
}

bool UiAnimation::is_looping() const noexcept
{
    if (_loop_override)
        return *_loop_override;

    return _default_loop.value_or(false);
}

void UiAnimation::play()
{
    if (_animation)
        _animation->reset();
}

void UiAnimation::pause()
{
    if (_animation)
        _animation->pause();
}

void UiAnimation::resume()
{
    if (_animation)
        _animation->resume();
}

void UiAnimation::reset() noexcept
{
    UiElement::reset();
    if (_animation)
        _animation->reset();
}

bool UiAnimation::is_finished() const noexcept
{
    return _animation && _animation->is_finished();
}

bool UiAnimation::is_paused() const noexcept
{
    return !_animation || _animation->is_paused();
}

void UiAnimation::update(double delta)
{
    if (_animation)
        _animation->update(delta);
}

void UiAnimation::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible() || !_animation)
        return;

    const elysia::resources::FrameInfo* frame = _animation->current_frame();
    if (!frame || !frame->_texture)
        return;

	elysia::core::UiRenderCommand command =
		elysia::core::make_ui_texture_command(frame->_texture, screen_rect());
	if (frame->_source_rect.has_value())
	{
		command.use_src_rect = true;
		command.src_rect = *frame->_source_rect;
	}
	apply_opacity(command);
    out_commands.push_back(command);
}
}
