#include "../../tools/logger.h"
#include "character_audio_layout_loader.h"

#include "../json/json_loader.h"
#include <utility>

namespace elysia::io
{
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
		ELYSIA_LOG_WARN("io","Load character audio layout failed: " << result.error);
		return false;
	}

	if (!loader.root().is_object())
	{
		ELYSIA_LOG_WARN("io","Load character audio layout failed: root is not an object: "
			<< layout_path);
		return false;
	}

	CharacterAudioLayout parsed_layout;
	for (json::const_iterator sound = loader.root().begin();
		sound != loader.root().end();
		++sound)
	{
		if (!sound.value().is_string())
		{
			ELYSIA_LOG_WARN("io","Load character audio layout failed: sound entry is not a string: "
				<< sound.key());
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

}
