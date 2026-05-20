#include "character_animation_layout_loader.h"

#include "json_loader.h"

#include <iostream>
#include <utility>

bool CharacterAnimationLayoutLoader::load(
	const std::filesystem::path& layout_path,
	CharacterAnimationLayout& layout
) const
{
	layout = CharacterAnimationLayout{};

	JsonLoader loader;
	JsonReadResult result = loader.open_file(layout_path);
	if (!result)
	{
		std::cout << "Load character animation layout failed: " << result.error;
		return false;
	}

	if (!loader.root().is_object())
	{
		std::cout << "Load character animation layout failed: root is not an object: "
			<< layout_path << std::endl;
		return false;
	}

	if (!loader.root().contains("animations") || !loader.root().at("animations").is_object())
	{
		std::cout << "Load character animation layout failed: animations is missing or not an object: "
			<< layout_path << std::endl;
		return false;
	}

	CharacterAnimationLayout parsed_layout;
	const json& animations = loader.root().at("animations");
	for (json::const_iterator animation = animations.begin();
		animation != animations.end();
		++animation)
	{
		if (!animation.value().is_object())
		{
			std::cout << "Load character animation layout failed: animation entry is not an object: "
				<< animation.key() << std::endl;
			return false;
		}

		const json& animation_node = animation.value();
		CharacterAnimationLayoutEntry entry;

		if (animation_node.contains("path"))
		{
			if (!animation_node.at("path").is_string())
			{
				std::cout << "Load character animation layout failed: path is not a string: "
					<< animation.key() << std::endl;
				return false;
			}

			entry._path = animation_node.at("path").get<std::string>();
			entry._has_path = true;
		}

		if (animation_node.contains("segment_path"))
		{
			if (!animation_node.at("segment_path").is_string())
			{
				std::cout << "Load character animation layout failed: segment_path is not a string: "
					<< animation.key() << std::endl;
				return false;
			}

			entry._segment_path = animation_node.at("segment_path").get<std::string>();
			entry._has_segment_path = true;
		}

		if (!entry._has_path && !entry._has_segment_path)
		{
			std::cout << "Load character animation layout failed: path or segment_path is missing: "
				<< animation.key() << std::endl;
			return false;
		}

		parsed_layout._animations.emplace(animation.key(), std::move(entry));
	}

	layout = std::move(parsed_layout);
	return true;
}
