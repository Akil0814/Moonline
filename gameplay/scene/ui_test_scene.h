#pragma once

#include "../../application/scene/application_scene.h"
#include "../../engine/ui/style/ui_theme_manager.h"

#include <array>
#include <string>
#include <vector>

namespace elysia::ui
{
class UiWindow;
class UiButton;
class UiElement;
class UiLabel;
}

namespace arcneco::scene
{
class UiTestScene final : public ApplicationScene
{
public:
    UiTestScene() = default;
    ~UiTestScene() override = default;

    void on_input(const elysia::input::RawInputFrame& input,const std::vector<elysia::input::RawInputEvent>& events) override;
    void on_enter(const elysia::scene::ScenePayload& payload) override;
    void on_exit() override;
    void reset() override;

private:
    void rebuild_ui();
    void clear_ui();
    void refresh_theme_preview_styles();
    void request_back_to_menu();
    void set_active_theme(elysia::ui::UiBuiltinTheme theme);
    void sync_theme_switch_button_roles() noexcept;
    void set_status(std::string text);

private:
    elysia::ui::UiWindow* _root_window = nullptr;
    elysia::ui::UiThemeManager _theme_manager;
    std::vector<elysia::ui::UiThemeRegistration> _theme_registrations;
    std::array<elysia::ui::UiButton*,7> _theme_buttons{};
    elysia::ui::UiLabel* _status_label = nullptr;
};
}
