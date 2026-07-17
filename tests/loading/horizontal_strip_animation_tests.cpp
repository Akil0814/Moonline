#define SDL_MAIN_HANDLED

#include "engine/animation/animation_manager.h"
#include "engine/io/loaders/animation_effect_manifest_loader.h"
#include "engine/io/loaders/animation_manifest_loader.h"
#include "engine/io/loaders/entity_manifest_loader.h"
#include "engine/io/loaders/fonts_manifest_loader.h"
#include "engine/io/loaders/i18n_manifest_loader.h"
#include "engine/io/path/path_manager.h"
#include "engine/resources/atlas/atlas_build_preparer.h"
#include "engine/resources/resource_manager.h"
#include "engine/ui/widgets/image/ui_animation.h"
#include "tests/support/test_assertions.h"

#include <SDL.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

namespace
{
using moonline::tests::require;

void test_horizontal_strip_manifest_schema()
{
	const std::filesystem::path test_root =
		std::filesystem::temp_directory_path() / "moonline_horizontal_strip_json_tests";
	std::filesystem::remove_all(test_root);
	std::filesystem::create_directories(test_root);

	const std::filesystem::path animation_manifest_path = test_root / "animations.json";
	std::ofstream(animation_manifest_path)
		<< R"({"animations":[{"key":"bad","path":"bad.png","frame_count":1,"fps":10,"loop":true,"horizontal_strip":"yes"}]})";
	elysia::io::AnimationManifest animation_manifest;
	require(!elysia::io::AnimationManifestLoader{}.load(animation_manifest_path, animation_manifest),
		"animation manifest horizontal_strip must reject non-boolean values");

	const std::filesystem::path directory_without_prefix_path = test_root / "directory_without_prefix.json";
	std::ofstream(directory_without_prefix_path)
		<< R"({"animations":[{"key":"core.idle","path":"idle","frame_count":2,"fps":10,"loop":true}]})";
	require(!elysia::io::AnimationManifestLoader{}.load(directory_without_prefix_path, animation_manifest),
		"core frame-directory animations must require frame_prefix");

	const std::filesystem::path strip_with_prefix_path = test_root / "strip_with_prefix.json";
	std::ofstream(strip_with_prefix_path)
		<< R"({"animations":[{"key":"core.strip","path":"strip.png","frame_count":2,"fps":10,"loop":true,"horizontal_strip":true,"frame_prefix":"strip"}]})";
	require(!elysia::io::AnimationManifestLoader{}.load(strip_with_prefix_path, animation_manifest),
		"core horizontal-strip animations must reject frame_prefix");
	const std::filesystem::path strip_with_empty_prefix_path = test_root / "strip_with_empty_prefix.json";
	std::ofstream(strip_with_empty_prefix_path)
		<< R"({"animations":[{"key":"core.strip","path":"strip.png","frame_count":2,"fps":10,"loop":true,"horizontal_strip":true,"frame_prefix":""}]})";
	require(!elysia::io::AnimationManifestLoader{}.load(strip_with_empty_prefix_path, animation_manifest),
		"core horizontal-strip animations must reject even an explicitly empty frame_prefix field");

	const std::filesystem::path valid_directory_path = test_root / "valid_directory.json";
	std::ofstream(valid_directory_path)
		<< R"({"animations":[{"key":"core.idle","path":"idle","frame_count":2,"fps":10,"loop":true,"frame_prefix":"core_idle"}]})";
	require(elysia::io::AnimationManifestLoader{}.load(valid_directory_path, animation_manifest)
		&& animation_manifest.animations.size() == 1
		&& animation_manifest.animations.front().frame_prefix == "core_idle"
		&& !animation_manifest.animations.front().horizontal_strip,
		"core frame-directory animations must accept an explicit frame_prefix");

	const std::filesystem::path valid_strip_path = test_root / "valid_strip.json";
	std::ofstream(valid_strip_path)
		<< R"({"animations":[{"key":"core.strip","path":"strip.png","frame_count":2,"fps":10,"loop":true,"horizontal_strip":true}]})";
	require(elysia::io::AnimationManifestLoader{}.load(valid_strip_path, animation_manifest)
		&& animation_manifest.animations.size() == 1
		&& animation_manifest.animations.front().frame_prefix.empty()
		&& animation_manifest.animations.front().horizontal_strip,
		"core horizontal-strip animations must load without a frame_prefix");

	const std::filesystem::path entity_manifest_path = test_root / "legacy_asset_key_entity.json";
	std::ofstream(entity_manifest_path)
		<< R"({"entities":[{"id":"Bad","asset_key":"Bad"}]})";
	elysia::io::EntityManifest entity_manifest;
	require(!elysia::io::EntityManifestLoader{}.load(entity_manifest_path, entity_manifest),
		"entity manifests must reject the removed asset_key field");
	const std::filesystem::path removed_horizontal_strip_entity_path = test_root / "horizontal_strip_entity.json";
	std::ofstream(removed_horizontal_strip_entity_path)
		<< R"({"entities":[{"id":"Bad","horizontal_strip":true}]})";
	require(!elysia::io::EntityManifestLoader{}.load(removed_horizontal_strip_entity_path, entity_manifest),
		"entity manifests must reject the removed horizontal_strip field");
	const std::filesystem::path missing_id_entity_path = test_root / "missing_id_entity.json";
	std::ofstream(missing_id_entity_path)
		<< R"({"entities":[{"enabled":true}]})";
	require(!elysia::io::EntityManifestLoader{}.load(missing_id_entity_path, entity_manifest),
		"entity manifests must reject entries without id");
	const std::filesystem::path unknown_entity_field_path = test_root / "unknown_entity_field.json";
	std::ofstream(unknown_entity_field_path)
		<< R"({"entities":[{"id":"Bad","unknown":true}]})";
	require(!elysia::io::EntityManifestLoader{}.load(unknown_entity_field_path, entity_manifest),
		"entity manifests must reject unknown fields");
	const std::filesystem::path disabled_invalid_entity_path = test_root / "disabled_invalid_entity.json";
	std::ofstream(disabled_invalid_entity_path)
		<< R"({"entities":[{"id":"bad-id","animation_layout":1,"enabled":false}]})";
	require(!elysia::io::EntityManifestLoader{}.load(disabled_invalid_entity_path, entity_manifest),
		"disabled entities must still validate all configured fields and key components");
	const std::filesystem::path duplicate_entity_path = test_root / "duplicate_entity.json";
	std::ofstream(duplicate_entity_path)
		<< R"({"entities":[{"id":"duplicate"},{"id":"duplicate"}]})";
	std::ostringstream duplicate_entity_log;
	std::streambuf* previous_log_buffer = std::clog.rdbuf(duplicate_entity_log.rdbuf());
	const bool duplicate_entity_loaded =
		elysia::io::EntityManifestLoader{}.load(duplicate_entity_path, entity_manifest);
	std::clog.rdbuf(previous_log_buffer);
	require(!duplicate_entity_loaded
		&& duplicate_entity_log.str().find("duplicate entity id: duplicate") != std::string::npos
		&& duplicate_entity_log.str().find("first:") != std::string::npos
		&& duplicate_entity_log.str().find("#/entities/0") != std::string::npos
		&& duplicate_entity_log.str().find("second:") != std::string::npos
		&& duplicate_entity_log.str().find("#/entities/1") != std::string::npos,
		"duplicate entity ids must be rejected with both manifest indices in the diagnostic");

	const std::filesystem::path duplicate_animation_property_path = test_root / "duplicate_animation_property.json";
	std::ofstream(duplicate_animation_property_path)
		<< R"({"animations":[{"key":"core.strip","key":"core.other","path":"strip.png","frame_count":2,"fps":10,"loop":true,"horizontal_strip":true}]})";
	require(!elysia::io::AnimationManifestLoader{}.load(duplicate_animation_property_path, animation_manifest),
		"core animation manifests must reject duplicate JSON object properties before parsing");

	const std::filesystem::path duplicate_effect_property_path = test_root / "duplicate_effect_property.json";
	std::ofstream(duplicate_effect_property_path)
		<< R"({"effects":[{"key":"effect.test","animation_key":"core.strip","animation_key":"core.other"}]})";
	elysia::io::AnimationEffectManifest effect_manifest;
	require(!elysia::io::AnimationEffectManifestLoader{}.load(duplicate_effect_property_path, effect_manifest),
		"core effect manifests must reject duplicate JSON object properties before parsing");

	const std::filesystem::path duplicate_font_property_path = test_root / "duplicate_font_property.json";
	std::ofstream(duplicate_font_property_path)
		<< R"({"fonts":[{"key":"ui.test","file":"first.ttf","file":"second.ttf"}]})";
	elysia::io::FontManifest font_manifest;
	require(!elysia::io::FontsManifestLoader{}.load(duplicate_font_property_path, font_manifest),
		"font manifests must reject duplicate JSON object properties before parsing");

	const std::filesystem::path valid_font_manifest_path = test_root / "valid_fonts.json";
	std::ofstream(valid_font_manifest_path)
		<< R"({"fonts":[{"key":"ui.test","file":"test.ttf"}]})";
	require(elysia::io::FontsManifestLoader{}.load(valid_font_manifest_path, font_manifest),
		"font manifests must accept font families without a sizes field");

	const std::filesystem::path legacy_font_sizes_path = test_root / "legacy_font_sizes.json";
	std::ofstream(legacy_font_sizes_path)
		<< R"({"sizes":[10,20,30,40,50,60,70],"fonts":[{"key":"ui.test","file":"test.ttf"}]})";
	require(!elysia::io::FontsManifestLoader{}.load(legacy_font_sizes_path, font_manifest),
		"font manifests must reject the removed sizes field");

	const std::filesystem::path duplicate_i18n_property_path = test_root / "duplicate_i18n_property.json";
	std::ofstream(duplicate_i18n_property_path)
		<< R"({"default_language":"en","default_language":"zh_cn","languages":["en"],"file":["base.json"]})";
	elysia::io::I18nManifest i18n_manifest;
	require(!elysia::io::I18nManifestLoader{}.load(duplicate_i18n_property_path, i18n_manifest),
		"i18n manifests must reject duplicate JSON object properties before parsing");

	std::filesystem::remove_all(test_root);
}

void test_horizontal_strip_build_and_render_commands()
{
	using namespace elysia;

	require(SDL_Init(SDL_INIT_VIDEO) == 0,
		"horizontal strip tests must initialize SDL video");
	SDL_Surface* target_surface = SDL_CreateRGBSurfaceWithFormat(
		0,640,480,32,SDL_PIXELFORMAT_RGBA32);
	require(target_surface != nullptr,
		"horizontal strip tests must create a software target surface");
	SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(target_surface);
	require(renderer != nullptr,
		"horizontal strip tests must create a software renderer");

	io::PathManager* path_manager = io::PathManager::instance();
	require(path_manager->init(),
		"horizontal strip tests must initialize the project path manager");
	resources::ResourceManager* resource_manager = resources::ResourceManager::instance();
	resource_manager->clear();

	resources::AtlasBuildRequest request;
	request.atlas_key = "horizontal_strip.test";
	request.source_path = path_manager->textures() / "test" / "frame_group.png";
	request.frame_count = 14;
	request.source_type = resources::AtlasSourceType::HorizontalStrip;

	resources::AtlasBuildPreparer preparer;
	std::vector<resources::AtlasFramePrepareTask> tasks;
	require(preparer.expand_build_request(request, tasks) && tasks.size() == 1,
		"horizontal strip must expand into exactly one preparation task");
	resources::AtlasFramePreparedResult prepared = preparer.prepare_frame(tasks.front());
	require(prepared.surface_result._success && prepared.surface_result._surface,
		"horizontal strip source image must decode successfully");
	require(resource_manager->begin_atlas_build(request),
		"horizontal strip atlas build must initialize");
	require(resource_manager->commit_prepared_atlas_frame(renderer, prepared),
		"horizontal strip atlas must commit from one prepared image");

	const resources::Atlas* atlas = resource_manager->find_atlas(request.atlas_key);
	require(atlas && atlas->size() == 14,
		"horizontal strip atlas must expose fourteen logical frames");
	const resources::FrameInfo* first = atlas->frame_at(0);
	const resources::FrameInfo* second = atlas->frame_at(1);
	const resources::FrameInfo* last = atlas->frame_at(13);
	require(first && second && last
		&& first->_texture == second->_texture && second->_texture == last->_texture,
		"horizontal strip frames must share one texture");
	require(resource_manager->texture_manager().resource_count() == 1,
		"horizontal strip atlas must store exactly one owned texture");
	require(first->_width == 146 && first->_height == 146
		&& first->_source_rect.has_value()
		&& first->_source_rect->nearly_equals(core::Rect{ 0,0,146,146 })
		&& second->_source_rect.has_value()
		&& second->_source_rect->nearly_equals(core::Rect{ 146,0,146,146 })
		&& last->_source_rect.has_value()
		&& last->_source_rect->nearly_equals(core::Rect{ 1898,0,146,146 }),
		"horizontal strip frames must expose contiguous 146 by 146 source rectangles");

	resources::AnimationBuildRequest animation_request;
	animation_request.animation_key = "horizontal_strip.test.animation";
	animation_request.atlas_key = request.atlas_key;
	animation_request.fps = 10.0;
	animation_request.loop = true;
	require(animation::AnimationManager::instance()->register_animation(animation_request, atlas),
		"horizontal strip animation must register");
	std::unique_ptr<animation::Animation> animation =
		animation::AnimationManager::instance()->create_animation(animation_request.animation_key);
	require(animation != nullptr,
		"horizontal strip animation must create a playback instance");
	core::RenderCommand render_command;
	require(animation->build_render_command(
		core::Rect{ 0,0,292,292 },0.0,core::SpriteFlip::None,render_command)
		&& render_command.use_src_rect
		&& render_command.src_rect.nearly_equals(core::Rect{ 0,0,146,146 }),
		"world animation render commands must preserve the frame source rectangle");
	animation->update(0.11);
	require(animation->build_render_command(
		core::Rect{ 0,0,292,292 },0.0,core::SpriteFlip::None,render_command)
		&& render_command.src_rect.nearly_equals(core::Rect{ 146,0,146,146 }),
		"horizontal strip playback must advance to the next logical source rectangle");

	ui::UiAnimation ui_animation(
		animation_request.animation_key,
		core::Rect{ 0,0,292,292 }
	);
	std::vector<core::UiRenderCommand> ui_commands;
	ui_animation.submit_ui_render_commands(ui_commands);
	require(ui_commands.size() == 1
		&& ui_commands.front().use_src_rect
		&& ui_commands.front().src_rect.nearly_equals(core::Rect{ 0,0,146,146 }),
		"UI animation render commands must preserve the frame source rectangle");
	ui_animation.update(0.11);
	ui_commands.clear();
	ui_animation.submit_ui_render_commands(ui_commands);
	require(ui_commands.size() == 1
		&& ui_commands.front().src_rect.nearly_equals(core::Rect{ 146,0,146,146 }),
		"UI horizontal strip playback must advance its source rectangle");

	resource_manager->clear();
	SDL_DestroyRenderer(renderer);
	SDL_FreeSurface(target_surface);
	SDL_Quit();
}

void test_horizontal_strip_rejects_non_divisible_width()
{
	using namespace elysia;

	require(SDL_Init(SDL_INIT_VIDEO) == 0,
		"invalid horizontal strip test must initialize SDL video");
	SDL_Surface* target_surface = SDL_CreateRGBSurfaceWithFormat(
		0,64,64,32,SDL_PIXELFORMAT_RGBA32);
	SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(target_surface);
	require(target_surface && renderer,
		"invalid horizontal strip test must create a software renderer");

	resources::ResourceManager* resource_manager = resources::ResourceManager::instance();
	resource_manager->clear();
	resources::AtlasBuildRequest request;
	request.atlas_key = "horizontal_strip.invalid";
	request.source_path = "invalid.png";
	request.frame_count = 3;
	request.source_type = resources::AtlasSourceType::HorizontalStrip;
	require(resource_manager->begin_atlas_build(request),
		"invalid strip build must initialize before dimension validation");

	resources::AtlasFramePreparedResult prepared;
	prepared.task.atlas_key = request.atlas_key;
	prepared.task.frame_path = request.source_path;
	prepared.task.expected_frame_count = request.frame_count;
	prepared.task.source_type = request.source_type;
	prepared.surface_result._success = true;
	prepared.surface_result._asset_key = request.atlas_key;
	prepared.surface_result._frame_path = request.source_path;
	prepared.surface_result._surface.reset(SDL_CreateRGBSurfaceWithFormat(
		0,10,4,32,SDL_PIXELFORMAT_RGBA32));
	require(prepared.surface_result._surface != nullptr,
		"invalid strip test must create its source surface");
	require(!resource_manager->commit_prepared_atlas_frame(renderer, prepared),
		"horizontal strip width not divisible by frame count must fail");

	resource_manager->clear();
	SDL_DestroyRenderer(renderer);
	SDL_FreeSurface(target_surface);
	SDL_Quit();
}
}

int main()
{
	test_horizontal_strip_manifest_schema();
	test_horizontal_strip_build_and_render_commands();
	test_horizontal_strip_rejects_non_divisible_width();
	std::cout << "horizontal strip animation tests passed\n";
	return EXIT_SUCCESS;
}
