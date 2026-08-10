#include "character_manager.h"

namespace arcneco::character
{
	bool CharacterManager::init()
	{
		_prototypes.emplace_back("RyougiShiki");
		_prototypes.emplace_back("AozakiAoko");
		_prototypes.emplace_back("ArcueidBrunestud");
		return true;
	}

	const std::vector<CharacterPrototype> CharacterManager::get_character_prototype()
	{
		return _prototypes;
	}

}
