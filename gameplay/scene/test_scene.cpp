#include "test_scene.h"

#include "../../application/scene/scene_keys.h"
#include "../../engine/input/raw_input_types.h"

namespace arcneco::scene
{
void TestScene::on_input(
    const elysia::input::RawInputFrame& input,
    const std::vector<elysia::input::RawInputEvent>& events)
{
    ApplicationScene::on_input(input,events);

    if (input.state.is_just_pressed(elysia::input::RawInputControl::KeyEscape))
        request_scene_switch(AppSceneKeys::MainMenu);
}

void TestScene::on_enter(const elysia::scene::ScenePayload& payload)
{
    (void)payload;
}

void TestScene::on_exit()
{
}

void TestScene::reset()
{
}
}
