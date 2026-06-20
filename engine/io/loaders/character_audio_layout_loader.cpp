#include "character_audio_layout_loader.h"

#include "../json/json_loader.h"

#include <iostream>
#include <utility>

bool CharacterAudioLayoutLoader::load(
	const std::filesystem::path& layout_path,
	CharacterAudioLayout& layout
) const
{
	layout = CharacterAudioLayout{};

	JsonLoader loader;
	JsonReadResult result = loader.open_file(layout_path);
	if (!result)
	{
		std::cout << "Load character audio layout failed: " << result.error;
		return false;
	}

	if (!loader.root().is_object())
	{
		std::cout << "Load character audio layout failed: root is not an object: "
			<< layout_path << std::endl;
		return false;
	}

	CharacterAudioLayout parsed_layout;
	for (json::const_iterator sound = loader.root().begin();
		sound != loader.root().end();
		++sound)
	{
		if (!sound.value().is_string())
		{
			std::cout << "Load character audio layout failed: sound entry is not a string: "
				<< sound.key() << std::endl;
			return false;
		}

		CharacterAudioLayoutEntry entry;
		entry.key = sound.key();
		entry.path = sound.value().get<std::string>();
		parsed_layout.sounds.push_back(std::move(entry));
	}

	layout = std::move(parsed_layout);
	return true;
}
