#pragma once

#include "../../application/scene/application_scene.h"
#include "../../engine/ui/style/ui_theme_manager.h"

#include <vector>

namespace elysia::ui
{
class UiWindow;
class UiButton;
class UiElement;
}

namespace arcneco::scene
{
class UiContainerTestScene final : public ApplicationScene
{
public:
    UiContainerTestScene() = default;
    ~UiContainerTestScene() override = default;

    void on_input(const elysia::input::RawInputFrame& input,const std::vector<elysia::input::RawInputEvent>& events) override;
    void on_enter(const elysia::scene::ScenePayload& payload) override;
    void on_exit() override;
    void reset() override;

private:
    void rebuild_ui();
    void clear_ui();
    void register_theme_element(elysia::ui::UiElement& element);
    void refresh_theme_preview_styles();
    void request_back_to_menu();
    void set_active_theme(elysia::ui::UiBuiltinTheme theme);
    void sync_theme_switch_button_roles() noexcept;

private:
    elysia::ui::UiWindow* _root_window = nullptr;
    elysia::ui::UiThemeManager _theme_manager;
    std::vector<elysia::ui::UiThemeRegistration> _theme_registrations;
    elysia::ui::UiButton* _blue_glass_moon_theme_button = nullptr;
    elysia::ui::UiButton* _elysia_light_theme_button = nullptr;
    elysia::ui::UiButton* _elysia_dark_theme_button = nullptr;
    elysia::ui::UiButton* _evangelion_unit_00_theme_button = nullptr;
    elysia::ui::UiButton* _evangelion_unit_01_theme_button = nullptr;
    elysia::ui::UiButton* _evangelion_unit_02_theme_button = nullptr;
    elysia::ui::UiButton* _quiet_slate_theme_button = nullptr;
};
}
