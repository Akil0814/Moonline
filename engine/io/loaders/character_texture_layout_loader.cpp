#include "../../tools/logger.h"
#include "character_texture_layout_loader.h"

#include "../json/json_loader.h"
#include <utility>

namespace elysia::io
{
bool CharacterTextureLayoutLoader::load(
	const std::filesystem::path& layout_path,
	CharacterTextureLayout& layout
) const
{
	layout = CharacterTextureLayout{};

	JsonLoader loader;
	JsonReadResult result = loader.open_file(layout_path);
	if (!result)
	{
		ELYSIA_LOG_ERROR("io","Load character texture layout failed: " << result.error);
		return false;
	}

	if (!loader.root().is_object())
	{
		ELYSIA_LOG_ERROR("io","Load character texture layout failed: root is not an object: "
			<< layout_path);
		return false;
	}

	CharacterTextureLayout parsed_layout;
	for (json::const_iterator texture = loader.root().begin();
		texture != loader.root().end();
		++texture)
	{
		if (!texture.value().is_string())
		{
			ELYSIA_LOG_ERROR("io","Load character texture layout failed: texture entry is not a string: "
				<< texture.key());
			return false;
		}

		CharacterTextureLayoutEntry entry;
		entry.key = texture.key();
		entry.path = texture.value().get<std::string>();
		parsed_layout.textures.push_back(std::move(entry));
	}

	layout = std::move(parsed_layout);
	return true;
}

}
