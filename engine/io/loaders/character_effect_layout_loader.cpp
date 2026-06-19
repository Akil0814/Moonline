#include "character_effect_layout_loader.h"

#include "../json/json_loader.h"

#include <iostream>
#include <utility>

bool CharacterEffectLayoutLoader::load(
	const std::filesystem::path& layout_path,
	CharacterEffectLayout& layout
) const
{
	layout = CharacterEffectLayout{};

	JsonLoader loader;
	JsonReadResult result = loader.open_file(layout_path);
	if (!result)
	{
		std::cout << "Load character effect layout failed: " << result.error;
		return false;
	}

	if (!loader.root().is_object())
	{
		std::cout << "Load character effect layout failed: root is not an object: "
			<< layout_path << std::endl;
		return false;
	}

	if (!loader.root().contains("effects") || !loader.root().at("effects").is_object())
	{
		std::cout << "Load character effect layout failed: effects is missing or not an object: "
			<< layout_path << std::endl;
		return false;
	}

	CharacterEffectLayout parsed_layout;
	const json& effects = loader.root().at("effects");
	for (json::const_iterator effect = effects.begin();
		effect != effects.end();
		++effect)
	{
		if (!effect.value().is_object())
		{
			std::cout << "Load character effect layout failed: effect entry is not an object: "
				<< effect.key() << std::endl;
			return false;
		}

		const json& effect_node = effect.value();
		CharacterEffectLayoutEntry entry;

		if (effect_node.contains("path"))
		{
			if (!effect_node.at("path").is_string())
			{
				std::cout << "Load character effect layout failed: path is not a string: "
					<< effect.key() << std::endl;
				return false;
			}

			entry.path = effect_node.at("path").get<std::string>();
			entry.has_path = true;
		}

		if (effect_node.contains("segment_path"))
		{
			if (!effect_node.at("segment_path").is_string())
			{
				std::cout << "Load character effect layout failed: segment_path is not a string: "
					<< effect.key() << std::endl;
				return false;
			}

			entry.segment_path = effect_node.at("segment_path").get<std::string>();
			entry.has_segment_path = true;
		}

		if (!entry.has_path && !entry.has_segment_path)
		{
			std::cout << "Load character effect layout failed: path or segment_path is missing: "
				<< effect.key() << std::endl;
			return false;
		}

		parsed_layout.effects.emplace(effect.key(), std::move(entry));
	}

	layout = std::move(parsed_layout);
	return true;
}
