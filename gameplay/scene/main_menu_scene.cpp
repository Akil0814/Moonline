#include "main_menu_scene.h"

#include "../../application/scene/scene_payloads.h"
#include "../../engine/audio/audio_service.h"
#include "../../engine/config/config_manager.h"
#include "../../engine/core/render/colors.h"
#include "../../engine/input/raw_input_types.h"
#include "../../engine/localization/localization_manager.h"

#include <algorithm>
#include <iostream>

namespace arcneco::scene
{
namespace
{
constexpr int kMenuCenterX = 640;
constexpr int kMenuStartY = 220;
constexpr int kMenuVerticalSpacing = 70;
constexpr elysia::core::Color kMenuTextColor = elysia::core::colors::white;
constexpr int kMenuTextPointSize = 24;
}

void MainMenuScene::on_enter(const elysia::scene::ScenePayload& payload)
{
	if (payload.has_value())
	{
		const MainMenuEnterPayload& enter_payload =
			elysia::scene::require_scene_payload<MainMenuEnterPayload>(payload);
		(void)enter_payload;
	}

	_paused = false;
	(void)elysia::audio::AudioService::instance()->play_music("scene.main_meun_scene_main");
	rebuild_menu_textures();
}

void MainMenuScene::on_update(double delta)
{
	elysia::scene::Scene::on_update(delta);
}

void MainMenuScene::on_render(SDL_Renderer* renderer)
{
	elysia::scene::Scene::on_render(renderer);

	for (const MenuTextEntry& entry : _menu_text_entries)
	{
		if (!entry.texture)
			continue;

		SDL_RenderCopy(renderer, entry.texture, nullptr, &entry.destination);
	}
}

void MainMenuScene::on_input(const elysia::input::RawInputFrame& input, const std::vector<elysia::input::RawInputEvent>& events)
{
	ApplicationScene::on_input(input, events);
	(void)input;

	for (const elysia::input::RawInputEvent& event : events)
	{
		if (event.type == elysia::input::RawInputEventType::ControlPressed
			&& elysia::input::matches_control(
				elysia::input::RawInputControl::KeyF6,
				event.control))
		{
			cycle_language();
			break;
		}
	}
}


void MainMenuScene::on_exit()
{
	_paused = false;
	clear_menu_textures();
}

void MainMenuScene::reset()
{
	_paused = false;
	clear_menu_textures();
}

void MainMenuScene::rebuild_menu_textures()
{
	clear_menu_textures();

	elysia::localization::LocalizationManager* localization_manager = elysia::localization::LocalizationManager::instance();
	if (!localization_manager)
		return;

	const std::vector<std::string> menu_keys = {
		"menu_scene.start",
		"menu_scene.settings",
		"menu_scene.exit",
		"menu_scene.about"
	};

	int current_y = kMenuStartY;
	for (const std::string& key : menu_keys)
	{
		MenuTextEntry entry;
		entry.key = key;
		entry.style.point_size = kMenuTextPointSize;
		entry.style.color = kMenuTextColor;
		entry.texture = localization_manager->get_text_texture(entry.key, entry.style);
		if (!entry.texture)
		{
			std::cout << "MainMenuScene warning: texture build failed for key "
				<< entry.key << std::endl;
			continue;
		}

		int width = 0;
		int height = 0;
		if (SDL_QueryTexture(entry.texture, nullptr, nullptr, &width, &height) != 0)
		{
			std::cout << "MainMenuScene warning: query texture failed for key "
				<< entry.key << ", error: " << SDL_GetError() << std::endl;
			continue;
		}

		entry.destination = SDL_Rect{
			kMenuCenterX - width / 2,
			current_y,
			width,
			height
		};
		current_y += kMenuVerticalSpacing;
		_menu_text_entries.push_back(std::move(entry));
	}
}

void MainMenuScene::clear_menu_textures()
{
	_menu_text_entries.clear();
}

void MainMenuScene::cycle_language()
{
	elysia::localization::LocalizationManager* localization_manager = elysia::localization::LocalizationManager::instance();
	if (!localization_manager)
		return;

	const std::vector<std::string>& languages = localization_manager->supported_languages();
	if (languages.empty())
		return;

	const auto current_iterator = std::find(
		languages.begin(),
		languages.end(),
		localization_manager->current_language());

	size_t next_index = 0;
	if (current_iterator != languages.end())
	{
		next_index = (
			static_cast<size_t>(std::distance(languages.begin(), current_iterator)) + 1)
			% languages.size();
	}

	if (!localization_manager->set_language(languages[next_index]))
		return;

	rebuild_menu_textures();

	elysia::config::ConfigManager* config_manager = elysia::config::ConfigManager::instance();
	std::string save_error;
	if (!config_manager->set_language(
		localization_manager->current_language(),
		save_error))
	{
		std::cout << "MainMenuScene warning: sync language config failed: "
			<< save_error << std::endl;
	}
	else if (!config_manager->save(save_error))
	{
		std::cout << "MainMenuScene warning: save language failed: "
			<< save_error << std::endl;
	}
}
}
