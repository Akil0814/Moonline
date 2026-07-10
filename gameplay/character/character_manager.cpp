#include "character_manager.h"

namespace arcneco::character
{
	bool CharacterManager::init()
	{
		_prototypes.emplace_back("ryougi_shiki");
		_prototypes.emplace_back("aozaki_aoko");
		_prototypes.emplace_back("arcueid_brunestud");
		return true;
	}

	const std::vector<CharacterPrototype> CharacterManager::get_character_prototype()
	{
		return _prototypes;
	}

}