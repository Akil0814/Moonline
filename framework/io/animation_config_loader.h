#pragma once
#include "asset_config_types.h"
#include "json_loader.h"

#include <cstddef>
#include <filesystem>
#include <string>

class AnimationConfigLoader
{
public:
	bool load(
		const std::filesystem::path& animation_config_path,
		const CharacterAnimationLayout& layout,
		AnimationConfig& config
	) const;

private:
	std::filesystem::path resolve_clip_path(
		const std::string& animation_name,
		bool is_segment,
		size_t segment_index,
		const json& clip_node,
		const CharacterAnimationLayout& layout
	) const;

	std::filesystem::path resolve_segment_path(
		const std::filesystem::path& segment_path,
		size_t segment_index
	) const;

	bool append_clip(
		const std::string& animation_name,
		bool is_segment,
		size_t segment_index,
		const json& clip_node,
		const CharacterAnimationLayout& layout,
		AnimationConfig& config
	) const;
};
