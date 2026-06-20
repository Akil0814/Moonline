#pragma once
#include "asset_config_types.h"

#include <filesystem>

class CharacterAudioLayoutLoader
{
public:
	bool load(
		const std::filesystem::path& layout_path,
		CharacterAudioLayout& layout
	) const;
};
