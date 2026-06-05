#include "animation.h"

namespace
{
	SDL_Rect to_sdl_rect(const Rect& rect)
	{
		SDL_Rect sdl_rect{};
		sdl_rect.x = static_cast<int>(rect.x());
		sdl_rect.y = static_cast<int>(rect.y());

		const int width = static_cast<int>(rect.width());
		const int height = static_cast<int>(rect.height());
		sdl_rect.w = width > 0 ? width : 0;
		sdl_rect.h = height > 0 ? height : 0;
		return sdl_rect;
	}
}

Animation::Animation()
{
	_timer.set_one_shot(false);
	_timer.set_wait_time(_interval_seconds);
	_timer.set_on_timeout(
		[this]()
		{
			if (!_atlas || _atlas->empty())
			{
				_frame_index = 0;
				return;
			}

			++_frame_index;
			if (_frame_index < _atlas->size())
				return;

			_frame_index = _is_loop ? 0 : _atlas->size() - 1;
			if (_is_loop)
				return;

			_is_finished = true;
			_timer.pause();
			if (_on_finished)
				_on_finished();
		});
}

void Animation::render(SDL_Renderer* renderer, const Rect& target_rect, double angle_degrees) const
{
	const FrameInfo* frame_info = current_frame();
	if (!frame_info || !frame_info->_texture)
		return;

	SDL_Rect destination_rect = to_sdl_rect(target_rect);
	SDL_Point center_point{ destination_rect.w / 2, destination_rect.h / 2 };
	SDL_RenderCopyEx(renderer, frame_info->_texture, nullptr, &destination_rect, angle_degrees, &center_point, SDL_FLIP_NONE);
}

void Animation::render(SDL_Renderer* renderer, const Vector2& target_position, double angle_degrees) const
{
	const FrameInfo* frame_info = current_frame();
	if (!frame_info || !frame_info->_texture)
		return;

	SDL_Rect destination_rect{};
	destination_rect.x = static_cast<int>(target_position.x);
	destination_rect.y = static_cast<int>(target_position.y);
	destination_rect.w = frame_info->_width;
	destination_rect.h = frame_info->_height;

	SDL_Point center_point{ frame_info->_width / 2, frame_info->_height / 2 };
	SDL_RenderCopyEx(renderer, frame_info->_texture, nullptr, &destination_rect, angle_degrees, &center_point, SDL_FLIP_NONE);
}

bool Animation::build_render_command(
	const Rect& target_rect,
	double angle_degrees,
	RenderCommand& out_command
) const
{
	const FrameInfo* frame_info = current_frame();
	if (!frame_info || !frame_info->_texture)
		return false;

	out_command._texture = frame_info->_texture;
	out_command._world_rect = target_rect;
	out_command._use_src_rect = false;
	out_command._rotation_degrees = angle_degrees;
	out_command._rotation_origin = Vector2(0.5f, 0.5f);
	out_command._flip = SpriteFlip::None;
	return true;
}

void Animation::update(double delta_seconds)
{
	if (!_atlas || _atlas->empty() || _is_finished)
		return;

	_timer.update(delta_seconds);
}

void Animation::set_atlas(const Atlas* atlas)
{
	_atlas = atlas;
	reset();
}

void Animation::set_loop(bool is_loop)
{
	_is_loop = is_loop;
}

void Animation::set_interval_seconds(double interval_seconds)
{
	_interval_seconds = interval_seconds > 0.0 ? interval_seconds : 0.1;
	_timer.set_wait_time(_interval_seconds);
}

void Animation::set_on_finished(PlayCallback on_finished)
{
	_on_finished = std::move(on_finished);
}

void Animation::reset()
{
	_timer.restart();
	_frame_index = 0;
	_is_finished = false;
}

void Animation::pause()
{
	_timer.pause();
}

void Animation::resume()
{
	_timer.resume();
}

bool Animation::is_finished() const
{
	return _is_finished;
}

bool Animation::is_paused() const
{
	return _timer.is_paused();
}

size_t Animation::current_frame_index() const
{
	return _frame_index;
}

const FrameInfo* Animation::current_frame() const
{
	if (!_atlas)
		return nullptr;

	return _atlas->frame_at(_frame_index);
}
