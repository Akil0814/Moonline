#pragma once

#include "../../application/scene/application_scene.h"

#include "../../engine/ui/widgets/ui_button.h"

#include <SDL.h>

#include <string>
#include <vector>

namespace arcneco::scene
{
class MainMenuScene final : public ApplicationScene
{
public:
	MainMenuScene() = default;
	~MainMenuScene() override = default;

	void on_update(double delta)override;
	void on_render(SDL_Renderer* renderer)override;
	void on_input(const elysia::input::RawInputFrame& input, const std::vector<elysia::input::RawInputEvent>& events)override;

	void on_enter(const elysia::scene::ScenePayload& payload) override;
	void on_exit() override;
	void reset() override;

private:
	struct MenuButtonEntry
	{
		std::string key;
		elysia::ui::UiButton* button = nullptr;
	};

	[[nodiscard]] elysia::input::InputContext input_context() const override;
	void rebuild_menu_buttons();
	void clear_menu_buttons();
	void cycle_language();
	void set_focused_button(size_t index);
	void move_focus(int direction);

private:
	std::vector<MenuButtonEntry> _menu_button_entries;
	size_t _focused_button_index = 0;
};
}
