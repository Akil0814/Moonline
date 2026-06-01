#include "progress_bar.h"

ProgressBar::ProgressBar(Vector2 position,Vector2 size) :GameObject(DepthLayer::UI)
{
    GameObject::set_world_position(position);
    GameObject::set_size(size);
}


void ProgressBar::on_render(SDL_Renderer* renderer)
{
    _bar.render(renderer, rect());
}

void ProgressBar::reset()
{
    GameObject::reset();
    _bar.reset();
}


void ProgressBar::set_progress(float value)
{
    _bar.set_ratio(value);
}

float ProgressBar::progress() const
{
    return _bar.ratio();
}

Bar& ProgressBar::bar()
{
    return _bar;
}

const Bar& ProgressBar::bar() const
{
    return _bar;
}
