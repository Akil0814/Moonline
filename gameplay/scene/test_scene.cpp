#include "test_scene.h"

#include "moonline_scene_keys.h"
#include "../../engine/input/raw_input_types.h"
#include "../../engine/ui/widgets/image/ui_animation.h"

namespace arcneco::scene
{
void TestScene::on_input(
    const elysia::input::RawInputFrame& input,
    const std::vector<elysia::input::RawInputEvent>& events)
{
    elysia::scene::Scene::on_input(input,events);

    if (input.state.is_just_pressed(elysia::input::RawInputControl::KeyEscape))
        request_scene_switch(MoonlineSceneKeys::MainMenu);
}

void TestScene::on_enter(const elysia::scene::ScenePayload& payload)
{
    (void)payload;

    if (!_test_animation)
    {
        _test_animation = create_and_add_object<elysia::ui::UiAnimation>(
            "test.animation",
            elysia::core::Rect{ 160.0f,200.0f,292.0f,292.0f }
        );
    }
    else
    {
        _test_animation->play();
    }

    if (!_flying_demon_idle)
    {
        _flying_demon_idle = create_and_add_object<elysia::ui::UiAnimation>(
			"FlyingDemon.idle",
            elysia::core::Rect{ 760.0f,204.0f,324.0f,284.0f }
        );
    }
    else
    {
        _flying_demon_idle->play();
    }
}

void TestScene::on_exit()
{
    if (_test_animation)
        _test_animation->pause();
    if (_flying_demon_idle)
        _flying_demon_idle->pause();
}

void TestScene::reset()
{
    if (_test_animation)
        _test_animation->play();
    if (_flying_demon_idle)
        _flying_demon_idle->play();
}
}
