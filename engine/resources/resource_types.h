#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

namespace elysia::resources
{
struct TextureLoadRequest
{
	std::string key;
	std::filesystem::path file_path;
};

struct FontLoadRequest
{
	std::string key;
	std::filesystem::path file_path;
	int point_size = 0;
};

struct SoundLoadRequest
{
	std::string key;
	std::filesystem::path file_path;
};

struct MusicLoadRequest
{
	std::string key;
	std::filesystem::path file_path;
};

struct AtlasBuildRequest
{
	[[nodiscard]] bool is_valid() const
	{
		return !atlas_key.empty()
			&& !directory_path.empty()
			&& frame_count > 0;
	}

	std::string atlas_key;
	std::filesystem::path directory_path;
	size_t frame_count = 0;
};

struct AnimationBuildRequest
{
	std::string animation_key;
	std::string atlas_key;
	double fps = 10.0;
	bool loop = true;
	size_t segment_index = 0;
};

struct EffectBuildRequest
{
	std::string effect_key;
	std::string animation_key;
};

}
