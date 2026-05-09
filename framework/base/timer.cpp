#include "timer.h"

void Timer::restart()
{
	_elapsed_seconds = 0.0;
	_has_shot = false;
	_is_paused = false;
}

void Timer::set_wait_time(double wait_time_seconds)
{
	_wait_time_seconds = wait_time_seconds;
}

void Timer::set_one_shot(bool is_one_shot)
{
	_is_one_shot = is_one_shot;
}

void Timer::set_on_timeout(TimeoutCallback on_timeout)
{
	_on_timeout = std::move(on_timeout);
}

void Timer::pause()
{
	_is_paused = true;
}

void Timer::resume()
{
	_is_paused = false;
}

bool Timer::is_paused() const
{
	return _is_paused;
}

void Timer::update(double delta_seconds)
{
	if (_is_paused || _wait_time_seconds <= 0.0)
		return;

	_elapsed_seconds += delta_seconds;
	if (_elapsed_seconds < _wait_time_seconds)
		return;

	const bool can_shot = !_is_one_shot || !_has_shot;
	_has_shot = true;
	if (can_shot && _on_timeout)
		_on_timeout();

	_elapsed_seconds -= _wait_time_seconds;
}
