#define SDL_MAIN_HANDLED

#include "engine/camera/camera_manager.h"
#include "engine/scene/scene.h"
#include "engine/scene/scene_manager.h"
#include "tests/support/test_assertions.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <utility>

namespace
{
using moonline::tests::require;
using elysia::camera::CameraManager;
using elysia::camera::CameraSlot;
using elysia::core::Rect;
using elysia::core::Vector2;

constexpr std::array<CameraSlot, 4> k_camera_slots{
    CameraSlot::Main,
    CameraSlot::Cinematic,
    CameraSlot::Auxiliary1,
    CameraSlot::Auxiliary2
};

using CameraAccess = decltype(
    std::declval<CameraManager&>().camera(CameraSlot::Main)
);
static_assert(std::is_same_v<CameraAccess, const elysia::camera::Camera&>);

void reset_cameras()
{
    auto* manager = CameraManager::instance();
    manager->reset_all();

    for (CameraSlot slot : k_camera_slots)
    {
        manager->set_viewport_size(slot, Vector2::zero());
    }
}

void test_fixed_slots_are_independent()
{
    reset_cameras();
    auto* manager = CameraManager::instance();

    manager->set_center(CameraSlot::Main, Vector2(10.0f, 20.0f));
    manager->set_center(CameraSlot::Cinematic, Vector2(30.0f, 40.0f));
    manager->set_center(CameraSlot::Auxiliary1, Vector2(50.0f, 60.0f));
    manager->set_center(CameraSlot::Auxiliary2, Vector2(70.0f, 80.0f));

    require(manager->camera(CameraSlot::Main).center() == Vector2(10.0f, 20.0f),
        "Main camera must retain its own center");
    require(manager->camera(CameraSlot::Cinematic).center() == Vector2(30.0f, 40.0f),
        "Cinematic camera must retain its own center");
    require(manager->camera(CameraSlot::Auxiliary1).center() == Vector2(50.0f, 60.0f),
        "Auxiliary1 camera must retain its own center");
    require(manager->camera(CameraSlot::Auxiliary2).center() == Vector2(70.0f, 80.0f),
        "Auxiliary2 camera must retain its own center");

    manager->set_focus_rect(CameraSlot::Cinematic, Rect(90.0f, 40.0f, 20.0f, 20.0f));
    manager->set_follow_strategy(
        CameraSlot::Cinematic,
        std::make_unique<elysia::camera::HardFollowStrategy>()
    );
    manager->update(0.0);

    require(manager->camera(CameraSlot::Cinematic).center() == Vector2(100.0f, 50.0f),
        "CameraManager::update must advance configured cameras");
    require(manager->camera(CameraSlot::Main).center() == Vector2(10.0f, 20.0f),
        "updating Cinematic must not change Main");
}

void test_requests_are_fifo_and_targeted()
{
    reset_cameras();
    auto* manager = CameraManager::instance();
    manager->set_center(CameraSlot::Main, Vector2(5.0f, 5.0f));

    const elysia::camera::CameraShakeParams shake{
        .amplitude = Vector2(0.0f, 10.0f),
        .duration_seconds = 1.0,
        .frequency_hz = 0.0
    };

    manager->request_shake(CameraSlot::Main, shake);
    manager->request_clear_effects(CameraSlot::Main);
    manager->update(0.1);
    require(manager->camera(CameraSlot::Main).center() == Vector2(5.0f, 5.0f),
        "a later clear request must cancel an earlier shake before camera update");

    manager->request_clear_effects(CameraSlot::Main);
    manager->request_shake(CameraSlot::Main, shake);
    manager->update(0.1);
    require(manager->camera(CameraSlot::Main).center().nearly_equals(Vector2(5.0f, 14.0f)),
        "a later shake request must remain active after an earlier clear request");
    require(manager->camera(CameraSlot::Auxiliary1).center() == Vector2::zero(),
        "Main requests must not affect Auxiliary1");
}

void test_reset_preserves_viewport_and_other_slots()
{
    reset_cameras();
    auto* manager = CameraManager::instance();

    manager->set_viewport_size(CameraSlot::Main, Vector2(320.0f, 180.0f));
    manager->set_center(CameraSlot::Main, Vector2(25.0f, 30.0f));
    manager->set_center(CameraSlot::Auxiliary1, Vector2(75.0f, 80.0f));
    manager->request_shake(CameraSlot::Main, elysia::camera::CameraShakeParams{});

    manager->reset(CameraSlot::Main);
    manager->update(0.01);

    require(manager->camera(CameraSlot::Main).center() == Vector2::zero(),
        "reset must clear the camera center and pending requests");
    require(manager->camera(CameraSlot::Main).viewport_size() == Vector2(320.0f, 180.0f),
        "reset must preserve the camera viewport");
    require(manager->camera(CameraSlot::Auxiliary1).center() == Vector2(75.0f, 80.0f),
        "resetting Main must not reset Auxiliary1");
}

class CameraScene final : public elysia::scene::Scene
{
public:
    CameraScene()
    {
        last_instance = this;
    }

    void on_enter(const elysia::scene::ScenePayload&) override {}
    void on_exit() override {}
    void reset() override {}

    void select_camera(CameraSlot slot) noexcept
    {
        set_render_camera_slot(slot);
    }

    void request_self_reset()
    {
        request_scene_switch(1, {}, elysia::scene::SceneReloadMode::Reset);
    }

    static inline CameraScene* last_instance = nullptr;
};

void test_scene_defaults_and_main_lifecycle()
{
    reset_cameras();
    auto* cameras = CameraManager::instance();
    cameras->set_viewport_size(CameraSlot::Main, Vector2(640.0f, 360.0f));
    cameras->set_center(CameraSlot::Main, Vector2(10.0f, 20.0f));
    cameras->set_center(CameraSlot::Cinematic, Vector2(70.0f, 80.0f));

    {
        CameraScene scene;
        require(scene.render_camera_slot() == CameraSlot::Main,
            "Scene must default to Main camera");
        require(scene.camera().center() == Vector2(10.0f, 20.0f),
            "Scene camera access must read Main by default");

        scene.select_camera(CameraSlot::Cinematic);
        require(scene.camera().center() == Vector2(70.0f, 80.0f),
            "Scene must read an explicitly selected render camera");
    }

    elysia::scene::SceneManager scene_manager;
    scene_manager.register_scene<CameraScene>(1);
    scene_manager.start(1);

    require(cameras->camera(CameraSlot::Main).center() == Vector2::zero(),
        "entering the first managed scene must reset Main");
    require(cameras->camera(CameraSlot::Main).viewport_size() == Vector2(640.0f, 360.0f),
        "scene transitions must preserve Main viewport");
    require(cameras->camera(CameraSlot::Cinematic).center() == Vector2(70.0f, 80.0f),
        "scene transitions must preserve Cinematic");

    cameras->set_center(CameraSlot::Main, Vector2(35.0f, 45.0f));
    CameraScene::last_instance->request_self_reset();
    scene_manager.on_update(0.0);

    require(cameras->camera(CameraSlot::Main).center() == Vector2::zero(),
        "Reset reload must clear Main before re-entering the scene");
    require(cameras->camera(CameraSlot::Cinematic).center() == Vector2(70.0f, 80.0f),
        "Reset reload must not clear Cinematic");

    scene_manager.shutdown();
}
}

int main()
{
    test_fixed_slots_are_independent();
    test_requests_are_fifo_and_targeted();
    test_reset_preserves_viewport_and_other_slots();
    test_scene_defaults_and_main_lifecycle();
    reset_cameras();
    std::cout << "camera manager tests passed\n";
    return EXIT_SUCCESS;
}
