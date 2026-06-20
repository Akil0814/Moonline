#pragma once
#include "asset_config_types.h"

#include <filesystem>

namespace elysia::io
{
class CharacterTextureLayoutLoader
{
public:
	bool load(
		const std::filesystem::path& layout_path,
		CharacterTextureLayout& layout
	) const;
};

}
