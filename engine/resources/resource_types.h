#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <utility>

struct TextureLoadRequest
{
	std::string key;
	std::filesystem::path file_path;
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

class AtlasLoadRequest
{
public:
	AtlasLoadRequest(
		std::string atlas_key,
		std::filesystem::path directory_path,
		size_t frame_count
	)
		: _atlas_key(std::move(atlas_key))
		, _directory_path(std::move(directory_path))
		, _frame_count(frame_count)
	{
	}

	[[nodiscard]] const std::string& atlas_key() const
	{
		return _atlas_key;
	}

	[[nodiscard]] const std::filesystem::path& directory_path() const
	{
		return _directory_path;
	}

	[[nodiscard]] size_t frame_count() const
	{
		return _frame_count;
	}

	[[nodiscard]] bool is_valid() const
	{
		return !_atlas_key.empty()
			&& !_directory_path.empty()
			&& _frame_count > 0;
	}

private:
	std::string _atlas_key;
	std::filesystem::path _directory_path;
	size_t _frame_count = 0;
};

struct AnimationBuildRequest
{
	std::string _animation_key;
	std::string _atlas_key;
	double _fps = 10.0;
	bool _loop = true;
	size_t _segment_index = 0;
};

struct EffectBuildRequest
{
	std::string _effect_key;
	std::string _animation_key;
};
