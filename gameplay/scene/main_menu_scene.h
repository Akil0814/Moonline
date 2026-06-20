#pragma once

#include "../../application/scene/application_scene.h"

#include "../../engine/localization/localized_text_style.h"

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
	struct MenuTextEntry
	{
		std::string key;
		elysia::localization::LocalizedTextStyle style;
		SDL_Texture* texture = nullptr;
		SDL_Rect destination{ 0, 0, 0, 0 };
	};

	void rebuild_menu_textures();
	void clear_menu_textures();
	void cycle_language();

private:
	std::vector<MenuTextEntry> _menu_text_entries;
};
}
