#include "setting_scene.h"
namespace arcneco::scene
{
void SettingScene::on_update(double delta)
{
    elysia::scene::Scene::on_update(delta);
}

void SettingScene::on_render(SDL_Renderer* renderer)
{
    (void)renderer;
}

void SettingScene::on_input(
    const elysia::input::RawInputFrame& input,
    const std::vector<elysia::input::RawInputEvent>& events)
{
    (void)input;
    (void)events;
}

void SettingScene::on_enter(const elysia::scene::ScenePayload& payload)
{
    (void)payload;
}

void SettingScene::on_exit()
{
}

void SettingScene::reset()
{
}
}
