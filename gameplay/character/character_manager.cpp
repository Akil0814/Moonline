#include "character_manager.h"

namespace arcneco::character
{
	bool CharacterManager::init()
	{
		CharacterPrototype tmp;
		tmp.id = "ryougi_shiki";
		_prototypes.push_back(tmp);
		return true;
	}

	const std::vector<CharacterPrototype> CharacterManager::get_character_prototype()
	{
		return _prototypes;
	}

}