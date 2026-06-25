#pragma once

#include "../../application/scene/application_scene.h"

#include "../../engine/ui/widgets/ui_button.h"
#include "../../engine/ui/widgets/ui_slider.h"
#include "../../engine/ui/widgets/label/ui_blink_label.h"
#include "../../engine/ui/widgets/label/ui_label.h"
#include "../../engine/ui/widgets/label/ui_pulse_label.h"

#include <SDL.h>

#include <cstdint>
#include <string>
#include <vector>

namespace arcneco::scene
{
class MainMenuScene final : public ApplicationScene
{
public:
    MainMenuScene() = default;
    ~MainMenuScene() override = default;

    void on_update(double delta) override;
    void on_render(SDL_Renderer* renderer) override;
    void on_input(const elysia::input::RawInputFrame& input, const std::vector<elysia::input::RawInputEvent>& events) override;

    void on_enter(const elysia::scene::ScenePayload& payload) override;
    void on_exit() override;
    void reset() override;

private:
    struct MenuButtonEntry
    {
        std::string key;
        elysia::ui::UiButton* button = nullptr;
    };

    void rebuild_menu_buttons();
    void clear_menu_buttons();
    void rebuild_ui_demo();
    void clear_ui_demo();
    void apply_demo_opacity(std::uint8_t opacity) noexcept;
    void cycle_language();
    void set_focused_button(size_t index);
    void move_focus(int direction);

private:
    std::vector<MenuButtonEntry> _menu_button_entries;
    elysia::ui::UiLabel* _demo_title_label = nullptr;
    elysia::ui::UiSlider* _demo_slider = nullptr;
    elysia::ui::UiLabel* _demo_preview_label = nullptr;
    elysia::ui::UiButton* _demo_preview_button = nullptr;
    elysia::ui::UiBlinkLabel* _demo_blink_label = nullptr;
    elysia::ui::UiPulseLabel* _demo_pulse_label = nullptr;
    elysia::ui::UiLabel* _demo_hint_label = nullptr;
    size_t _focused_button_index = 0;
};
}
