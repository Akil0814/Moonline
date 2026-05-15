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
		AnimationConfig& config
	) const;

private:
	bool append_clip(
		const std::string& animation_name,
		bool is_segment,
		size_t segment_index,
		const json& clip_node,
		AnimationConfig& config
	) const;
};
