#include "character_texture_layout_loader.h"

#include "../json/json_loader.h"

#include <iostream>
#include <utility>

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
		std::cout << "Load character texture layout failed: " << result.error;
		return false;
	}

	if (!loader.root().is_object())
	{
		std::cout << "Load character texture layout failed: root is not an object: "
			<< layout_path << std::endl;
		return false;
	}

	CharacterTextureLayout parsed_layout;
	for (json::const_iterator texture = loader.root().begin();
		texture != loader.root().end();
		++texture)
	{
		if (!texture.value().is_string())
		{
			std::cout << "Load character texture layout failed: texture entry is not a string: "
				<< texture.key() << std::endl;
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
