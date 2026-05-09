#include "scene_manager.h"

void SceneManager::set_current_scene(Scene* scene)
{
    if (!scene)
        return;

    if (_current_scene)
        _current_scene->on_exit();

    _current_scene = scene;
    _current_scene->on_enter();
}

void SceneManager::switch_to(Scene* scene)
{
    if (!scene)
        return;

    if (_current_scene == scene)
        return;

    if (_current_scene)
        _current_scene->on_exit();

    _current_scene = scene;
    _current_scene->on_enter();
}

void SceneManager::on_update(double delta)
{
    if (_current_scene)
        _current_scene->on_update(delta);
}

void SceneManager::on_render(SDL_Renderer* renderer)
{
    if (_current_scene)
        _current_scene->on_render(renderer);
}

void SceneManager::on_input(const SDL_Event& event)
{
    if (_current_scene)
        _current_scene->on_input(event);
}