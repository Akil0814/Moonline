#define SDL_MAIN_HANDLED

#include "engine/animation/animation_manager.h"
#include "engine/effects/effect_manager.h"
#include "engine/io/loaders/content_registry_loader.h"
#include "engine/io/path/path_manager.h"
#include "engine/loading/content_manifest_pipeline.h"
#include "engine/loading/resource_load_plan.h"
#include "engine/loading/resource_request_assembler.h"
#include "engine/resources/atlas/atlas.h"
#include "engine/resources/atlas/atlas_build_preparer.h"
#include "tests/support/test_assertions.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace
{
using moonline::tests::require;

std::string make_expected_key(
	const std::string& entity,
	const std::string& key_namespace,
	const std::string& logical,
	bool segmented,
	size_t segment)
{
	std::string key = entity;
	if (!key_namespace.empty()) key += "." + key_namespace;
	key += "." + logical;
	if (segmented) key += "." + std::to_string(segment);
	return key;
}

void replace_all(std::string& value, const std::string& marker, const std::string& replacement)
{
	size_t position = 0;
	while ((position = value.find(marker, position)) != std::string::npos)
	{
		value.replace(position, marker.size(), replacement);
		position += replacement.size();
	}
}

std::string make_expected_prefix(
	const elysia::io::EntityAnimationContentEntry& entry,
	const elysia::io::AnimationClipConfig& clip)
{
	std::string prefix = entry.frame_prefix_template;
	replace_all(prefix, "{id}", entry.entity.id);
	replace_all(prefix, "{animation}", clip.animation_name);
	std::string segment_suffix;
	if (clip.is_segment)
	{
		std::ostringstream stream;
		stream << '_' << std::setw(2) << std::setfill('0') << clip.segment_index;
		segment_suffix = stream.str();
	}
	replace_all(prefix, "{segment_suffix}", segment_suffix);
	return prefix;
}

uint64_t fnv1a64(const std::vector<std::string>& records)
{
	uint64_t hash = 14695981039346656037ull;
	for (const std::string& record : records)
		for (const unsigned char value : record)
		{
			hash ^= value;
			hash *= 1099511628211ull;
		}
	return hash;
}

std::string canonical_origin(const elysia::resources::ResourceOrigin& origin)
{
	return origin.config_path.generic_string() + "#" + origin.json_pointer
		+ "|" + std::to_string(static_cast<int>(origin.scope)) + "|" + origin.module
		+ "|" + origin.capability + "|" + origin.entity_id
		+ "|" + origin.logical_name + "|"
		+ (origin.segment_index ? std::to_string(*origin.segment_index) : "-");
}

void verify_repository_mapping_snapshot(const elysia::loading::ResourceLoadPlan& plan)
{
	std::vector<std::string> atlas_records;
	std::vector<std::string> animation_records;
	std::vector<std::string> effect_records;
	const auto root = elysia::io::PathManager::instance()->root();
	for (const auto& request : plan.atlas_build_requests())
	{
		std::ostringstream record;
		record << request.atlas_key << '|'
			<< request.source_path.lexically_relative(root).generic_string() << '|'
			<< request.frame_count << '|' << request.frame_filename_prefix << '|'
			<< static_cast<int>(request.source_type) << '|' << canonical_origin(request.origin) << '\n';
		atlas_records.push_back(record.str());
	}
	for (const auto& request : plan.animation_build_requests())
	{
		std::ostringstream record;
		record << std::setprecision(17) << request.animation_key << '|' << request.atlas_key << '|'
			<< request.fps << '|' << request.loop << '|' << request.segment_index << '|'
			<< canonical_origin(request.origin) << '\n';
		animation_records.push_back(record.str());
	}
	for (const auto& request : plan.animation_effect_build_requests())
	{
		std::ostringstream record;
		record << std::setprecision(17) << request.effect_key << '|' << request.animation_key << '|'
			<< request.default_size.x << '|' << request.default_size.y << '|'
			<< request.default_angle_degrees << '|' << canonical_origin(request.origin) << '\n';
		effect_records.push_back(record.str());
	}
	std::sort(atlas_records.begin(), atlas_records.end());
	std::sort(animation_records.begin(), animation_records.end());
	std::sort(effect_records.begin(), effect_records.end());
	const uint64_t atlas_hash = fnv1a64(atlas_records);
	const uint64_t animation_hash = fnv1a64(animation_records);
	const uint64_t effect_hash = fnv1a64(effect_records);
	constexpr uint64_t expected_atlas_hash = 1782569922306090303ull;
	constexpr uint64_t expected_animation_hash = 5718358840783149386ull;
	constexpr uint64_t expected_effect_hash = 12763037786180384463ull;
	if (atlas_hash != expected_atlas_hash || animation_hash != expected_animation_hash
		|| effect_hash != expected_effect_hash)
		std::cerr << "mapping hashes: atlas=" << atlas_hash << " animation=" << animation_hash
			<< " effect=" << effect_hash << '\n';
	require(atlas_hash == expected_atlas_hash,
		"the complete Atlas key/path/source/frame/prefix/origin mapping must match the reviewed repository snapshot");
	require(animation_hash == expected_animation_hash,
		"the complete Animation key/Atlas/FPS/loop/segment/origin mapping must match the reviewed repository snapshot");
	require(effect_hash == expected_effect_hash,
		"the complete Effect key/animation/defaults/origin mapping must match the reviewed repository snapshot");
}

void verify_complete_module_mapping(
	const elysia::loading::ContentManifestResult& config,
	const elysia::loading::ResourceLoadPlan& plan)
{
	std::unordered_map<std::string, const elysia::resources::AtlasBuildRequest*> atlases;
	std::unordered_map<std::string, const elysia::resources::AnimationBuildRequest*> animations;
	std::unordered_map<std::string, const elysia::resources::AnimationEffectBuildRequest*> effects;
	for (const auto& request : plan.atlas_build_requests())
		require(atlases.emplace(request.atlas_key, &request).second, "atlas requests must have unique keys");
	for (const auto& request : plan.animation_build_requests())
		require(animations.emplace(request.animation_key, &request).second, "animation requests must have unique keys");
	for (const auto& request : plan.animation_effect_build_requests())
		require(effects.emplace(request.effect_key, &request).second, "effect requests must have unique keys");

	std::unordered_set<std::string> expected_atlases{"test.animation"};
	std::unordered_set<std::string> expected_animations{"test.animation"};
	std::unordered_set<std::string> expected_effects{"effect.test"};
	elysia::resources::AtlasBuildPreparer preparer;
	for (const auto& [module_name, module] : config.additional_modules)
	{
		require(module.name == module_name, "module payload name must match its registry name");
		for (const auto& entry : module.animation_entries)
		{
			for (const auto& clip : entry.animation_config.clips)
			{
				const std::string key = make_expected_key(
					entry.entity.id, module.key_namespace, clip.animation_name,
					clip.is_segment, clip.segment_index);
				expected_atlases.insert(key);
				expected_animations.insert(key);
				const auto atlas_position = atlases.find(key);
				const auto animation_position = animations.find(key);
				require(atlas_position != atlases.end(), "every configured module clip must create an Atlas request");
				require(animation_position != animations.end(), "every configured module clip must create an Animation request");
				const auto& atlas = *atlas_position->second;
				const auto& animation = *animation_position->second;
				const bool strip = entry.animation_config.source_type == elysia::io::AnimationSourceType::HorizontalStrip;
				const auto base_path = (entry.texture_root / clip.path).lexically_normal();
				const auto expected_path = strip
					? (base_path / (clip.animation_name + ".png")).lexically_normal()
					: base_path;
				require(atlas.source_path == expected_path
					&& atlas.frame_count == clip.frame_count
					&& atlas.source_type == (strip ? elysia::resources::AtlasSourceType::HorizontalStrip
						: elysia::resources::AtlasSourceType::FrameDirectory),
					"every module Atlas request must preserve source type, path, and frame count");
				require(atlas.frame_filename_prefix == (strip ? std::string{} : make_expected_prefix(entry, clip)),
					"every module frame-directory prefix must use the configured template and two-digit file segment");
				require(animation.atlas_key == key && animation.fps == clip.fps
					&& animation.loop == clip.loop && animation.segment_index == clip.segment_index,
					"every module Animation request must preserve key, FPS, loop, and unpadded segment index");
				require(atlas.origin.module == module_name && atlas.origin.capability == "animations"
					&& atlas.origin.scope == elysia::resources::ResourceOriginScope::AdditionalModule
					&& atlas.origin.entity_id == entry.entity.id
					&& atlas.origin.logical_name == clip.animation_name
					&& atlas.origin.segment_index == clip.origin.segment_index
					&& !atlas.origin.config_path.is_absolute(),
					"every module request must carry its complete project-relative origin");

				std::vector<elysia::resources::AtlasFramePrepareTask> tasks;
				require(preparer.expand_build_request(atlas, tasks),
					"every configured module Atlas request must expand against real assets");
				if (strip)
					require(tasks.size() == 1 && tasks.front().frame_path == expected_path,
						"a horizontal strip must expand to one preparation task");
				else
				{
					require(tasks.size() == clip.frame_count,
						"a frame directory must expand to exactly the configured frame count");
					for (size_t frame = 0; frame < tasks.size(); ++frame)
					{
						std::ostringstream filename;
						filename << make_expected_prefix(entry, clip) << '_'
							<< std::setw(3) << std::setfill('0') << frame << ".png";
						require(tasks[frame].frame_path == base_path / filename.str(),
							"each frame task must use the explicit prefix and three-digit frame index");
					}
				}
			}
		}
		for (const auto& entry : module.effect_entries)
		{
			for (const auto& definition : entry.effect_config.effects)
			{
				const std::string effect_key = make_expected_key(entry.entity.id, module.key_namespace,
					definition.effect_name, definition.is_segment, definition.segment_index);
				const std::string animation_key = make_expected_key(entry.entity.id, module.key_namespace,
					definition.animation_name, definition.is_segment, definition.segment_index);
				expected_effects.insert(effect_key);
				const auto position = effects.find(effect_key);
				require(position != effects.end(), "every expanded effect mapping must create an EffectDefinition request");
				const auto& request = *position->second;
				require(request.animation_key == animation_key
					&& request.default_size.x == definition.default_width
					&& request.default_size.y == definition.default_height
					&& request.default_angle_degrees == definition.default_angle_degrees,
					"every EffectDefinition must bind the exact same-module animation and presentation defaults");
				require(request.origin.module == module_name && request.origin.capability == "effects"
					&& request.origin.scope == elysia::resources::ResourceOriginScope::AdditionalModule
					&& request.origin.entity_id == entry.entity.id
					&& request.origin.logical_name == definition.effect_name
					&& !request.origin.config_path.is_absolute(),
					"every EffectDefinition must carry its full project-relative origin");
			}
		}
	}

	require(atlases.size() == 167 && animations.size() == 167 && effects.size() == 20,
		"the migrated repository must expose the complete 167 Animation and 20 Effect mappings");
	require(atlases.size() == expected_atlases.size() && animations.size() == expected_animations.size()
		&& effects.size() == expected_effects.size(),
		"the request plan must not contain omitted or extra registry keys");
	for (const auto& [key, request] : atlases)
		require(expected_atlases.contains(key), "every Atlas key must correspond to a core or module config entry");
	for (const auto& [key, request] : animations)
		require(expected_animations.contains(key), "every Animation key must correspond to a core or module config entry");
	for (const auto& [key, request] : effects)
		require(expected_effects.contains(key), "every Effect key must correspond to a core or module config entry");
}

void test_runtime_resource_request_assembly()
{
    elysia::io::PathManager* path_manager = elysia::io::PathManager::instance();
    require(path_manager->init(), "path manager must initialize from the project root");

    elysia::loading::ContentManifestResult config_result;
    elysia::loading::ContentManifestPipeline content_manifest_pipeline;
	elysia::io::ContentRegistry content_registry;
	require(elysia::io::ContentRegistryLoader{}.load(path_manager->content_registry(), content_registry),
		"content registry must parse before resource request assembly");
    require(content_manifest_pipeline.load(content_registry, config_result),
        "config pipeline must load content before assembling resource requests");

	elysia::loading::ResourceLoadPlan load_plan;
	elysia::loading::ResourceRequestAssembler request_assembler;
	require(request_assembler.assemble(
			config_result,
			std::array{10,20,30,40,50,60,70},
			load_plan),
		"request assembler must build generic animation requests");
	require(load_plan.font_requests().size() == 35,
		"five project font families at seven resolved sizes must create 35 requests");

	elysia::loading::ResourceLoadPlan no_project_fonts_plan;
	require(request_assembler.assemble(
			config_result,
			std::span<const int>{},
			no_project_fonts_plan)
			&& no_project_fonts_plan.font_requests().empty(),
		"an empty project size set must create no project font requests");

	elysia::loading::ResourceLoadPlan custom_project_fonts_plan;
	require(request_assembler.assemble(
			config_result,
			std::array{24},
			custom_project_fonts_plan)
			&& custom_project_fonts_plan.font_requests().size() == 5,
		"a custom resolved size must create one request per project font family");
	for (const auto& request : custom_project_fonts_plan.font_requests())
		require(request.point_size == 24 && request.key.ends_with(".24"),
			"custom project font requests must use the externally resolved size");

	verify_complete_module_mapping(config_result, load_plan);
	verify_repository_mapping_snapshot(load_plan);

	const auto atlas_request = std::find_if(
		load_plan.atlas_build_requests().begin(),
		load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request)
		{
			return request.atlas_key == "test.animation";
		});
	require(atlas_request != load_plan.atlas_build_requests().end(),
		"test.animation must create an atlas request");
	require(atlas_request->source_path == path_manager->textures() / "test" / "frame_group.png"
		&& atlas_request->source_type == elysia::resources::AtlasSourceType::HorizontalStrip,
		"test.animation atlas must use the configured horizontal strip");
	require(atlas_request->frame_count == 14,
		"test.animation atlas must have fourteen frames");

	const auto animation_request = std::find_if(
		load_plan.animation_build_requests().begin(),
		load_plan.animation_build_requests().end(),
		[](const elysia::resources::AnimationBuildRequest& request)
		{
			return request.animation_key == "test.animation";
		});
	require(animation_request != load_plan.animation_build_requests().end(),
		"test.animation must create an animation request");
	require(animation_request->atlas_key == "test.animation"
		&& animation_request->fps == 10.0 && animation_request->loop,
		"test.animation must preserve its atlas key and playback settings");

	const auto ryougi_start_request = std::find_if(
		load_plan.atlas_build_requests().begin(),
		load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request)
		{
			return request.atlas_key == "RyougiShiki.start";
		});
	require(ryougi_start_request != load_plan.atlas_build_requests().end()
		&& ryougi_start_request->frame_filename_prefix == "RyougiShiki_start",
		"RyougiShiki start must use its inferred frame prefix");

	const auto runtime_slime_attack_request = std::find_if(
		load_plan.atlas_build_requests().begin(),
		load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request)
		{
			return request.atlas_key == "Slime.attack";
		});
	require(runtime_slime_attack_request != load_plan.atlas_build_requests().end()
		&& runtime_slime_attack_request->frame_filename_prefix == "Slime_attack"
		&& runtime_slime_attack_request->source_type == elysia::resources::AtlasSourceType::FrameDirectory,
		"enabled enemies module must contribute enemy atlas requests to the runtime plan");

	const auto flying_demon_idle_request = std::find_if(
		load_plan.atlas_build_requests().begin(),
		load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request)
		{
			return request.atlas_key == "FlyingDemon.idle";
		});
	require(flying_demon_idle_request != load_plan.atlas_build_requests().end()
		&& flying_demon_idle_request->source_type == elysia::resources::AtlasSourceType::HorizontalStrip
		&& flying_demon_idle_request->source_path
			== path_manager->textures() / "enemy" / "normal" / "FlyingDemon" / "idle" / "idle.png"
		&& flying_demon_idle_request->frame_count == 4,
		"FlyingDemon idle must resolve to its entity-wide horizontal strip source");

	const size_t flying_demon_request_count = static_cast<size_t>(std::count_if(
		load_plan.atlas_build_requests().begin(),
		load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request)
		{
			return request.atlas_key.starts_with("FlyingDemon.")
				&& request.source_type == elysia::resources::AtlasSourceType::HorizontalStrip;
		}));
	require(flying_demon_request_count == 5,
		"FlyingDemon must contribute five horizontal strip animation requests");

	const auto ryougi_getup_air_request = std::find_if(
		load_plan.atlas_build_requests().begin(),
		load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request)
		{
			return request.atlas_key == "RyougiShiki.getup_air";
		});
	require(ryougi_getup_air_request != load_plan.atlas_build_requests().end(),
		"RyougiShiki getup_air must create an atlas request");

	const auto ryougi_special_request = std::find_if(
		load_plan.atlas_build_requests().begin(),
		load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request)
		{
			return request.atlas_key.find("RyougiShiki.attack_special") != std::string::npos;
		});
	require(ryougi_special_request == load_plan.atlas_build_requests().end(),
		"RyougiShiki special attacks must not create runtime atlas requests");

	const auto aoko_no_effect_request = std::find_if(
		load_plan.animation_effect_build_requests().begin(),
		load_plan.animation_effect_build_requests().end(),
		[](const elysia::resources::AnimationEffectBuildRequest& request)
		{
			return request.effect_key == "AozakiAoko.effect.attack_air.0";
		});
	require(aoko_no_effect_request == load_plan.animation_effect_build_requests().end(),
		"omitted Aoko melee animations must not create effect requests");
	const size_t character_effect_count = static_cast<size_t>(std::count_if(
		load_plan.animation_effect_build_requests().begin(), load_plan.animation_effect_build_requests().end(),
		[](const elysia::resources::AnimationEffectBuildRequest& request)
		{
			return request.effect_key.starts_with("RyougiShiki.effect.")
				|| request.effect_key.starts_with("AozakiAoko.effect.")
				|| request.effect_key.starts_with("ArcueidBrunestud.effect.");
		}));
	require(character_effect_count == 19,
		"per-character effect info must add all nineteen configured character effects");
	const auto aoko_ranged = std::find_if(load_plan.atlas_build_requests().begin(), load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request) { return request.atlas_key == "AozakiAoko.effect.attack_ranged_ground"; });
	require(aoko_ranged != load_plan.atlas_build_requests().end()
		&& aoko_ranged->frame_count == 31
		&& aoko_ranged->frame_filename_prefix == "AozakiAoko_effects_attack_ranged_ground"
		&& aoko_ranged->source_path == path_manager->textures() / "character" / "AozakiAoko" / "animation" / "effects" / "attack" / "ranged" / "ground",
		"Aoko ranged effect animation must use its per-character frame count, path, and prefix");
	const auto arcueid_segment = std::find_if(load_plan.atlas_build_requests().begin(), load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request) { return request.atlas_key == "ArcueidBrunestud.effect.attack_normal.2"; });
	require(arcueid_segment != load_plan.atlas_build_requests().end()
		&& arcueid_segment->frame_count == 29
		&& arcueid_segment->source_path.filename() == "02"
		&& arcueid_segment->frame_filename_prefix == "ArcueidBrunestud_effects_attack_normal_02",
		"Arcueid segmented effect animation must use the effect naming profile");
	const auto ryougi_ranged = std::find_if(load_plan.atlas_build_requests().begin(), load_plan.atlas_build_requests().end(),
		[](const elysia::resources::AtlasBuildRequest& request) { return request.atlas_key == "RyougiShiki.effect.attack_ranged_ground"; });
	require(ryougi_ranged == load_plan.atlas_build_requests().end(),
		"omitted Ryougi ranged effect animations must produce no requests");
	for (const char* omitted_key : {
		"RyougiShiki.effect.attack_normal.4",
		"RyougiShiki.effect.attack_normal.5",
		"AozakiAoko.effect.attack_normal.0",
		"AozakiAoko.effect.attack_air.0"})
	{
		require(!std::any_of(load_plan.atlas_build_requests().begin(), load_plan.atlas_build_requests().end(),
			[omitted_key](const auto& request) { return request.atlas_key == omitted_key; }),
			"omitted or unregistered melee effect segments must produce no Atlas request");
		require(!std::any_of(load_plan.animation_effect_build_requests().begin(), load_plan.animation_effect_build_requests().end(),
			[omitted_key](const auto& request) { return request.effect_key == omitted_key; }),
			"omitted or unregistered melee effect segments must produce no EffectDefinition request");
	}

	elysia::resources::AtlasBuildPreparer preparer;
	for (const auto& request : load_plan.atlas_build_requests())
	{
		std::vector<elysia::resources::AtlasFramePrepareTask> tasks;
		require(preparer.expand_build_request(request, tasks),
			"every configured atlas must match its explicitly named real frame sequence");
	}
	for (const auto& request : load_plan.animation_effect_build_requests())
	{
		if (request.effect_key.find(".effect.attack_") == std::string::npos) continue;
		const auto matching_animation = std::find_if(load_plan.animation_build_requests().begin(), load_plan.animation_build_requests().end(),
			[&request](const elysia::resources::AnimationBuildRequest& animation) { return animation.animation_key == request.animation_key; });
		require(matching_animation != load_plan.animation_build_requests().end(),
			"every character effect definition must reference an existing animation request");
	}

	elysia::resources::Atlas atlas("test.animation");
	elysia::animation::AnimationManager* animation_manager =
		elysia::animation::AnimationManager::instance();
	require(animation_manager->register_animation(*animation_request, &atlas),
		"test.animation request must register with the animation manager");
	require(animation_manager->create_animation("test.animation") != nullptr,
		"test.animation must create a playback instance after registration");

	const auto effect_request = std::find_if(
		load_plan.animation_effect_build_requests().begin(),
		load_plan.animation_effect_build_requests().end(),
		[](const elysia::resources::AnimationEffectBuildRequest& request)
		{
			return request.effect_key == "effect.test";
		});
	require(effect_request != load_plan.animation_effect_build_requests().end(),
		"effect.test must create an effect request");
	require(effect_request->animation_key == "test.animation",
		"effect.test must reference test.animation");

	elysia::effects::EffectManager* effect_manager = elysia::effects::EffectManager::instance();
	require(effect_manager->register_animation_effect(*effect_request),
		"effect.test request must register with the effect manager");
	elysia::effects::AnimationEffectSpawnRequest effect_spawn_request;
	effect_spawn_request.effect_key = "effect.test";
	require(effect_manager->create_animation_effect(effect_spawn_request) != nullptr,
		"effect.test must create an effect instance after registration");
}
}

int main()
{
    test_runtime_resource_request_assembly();
    std::cout << "resource request assembler tests passed\n";
    return EXIT_SUCCESS;
}
