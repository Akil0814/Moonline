#include "../../engine/tools/singleton.h"

#include <string_view>
#include <vector>

namespace arcneco::character
{
	struct CharacterPrototype
	{
		std::string_view id;
	}; 

	class CharacterManager : public elysia::tools::Singleton<CharacterManager>
	{
		friend elysia::tools::Singleton<CharacterManager>;
	public:
		bool init();

		const std::vector<CharacterPrototype> get_character_prototype();



	private:
		std::vector<CharacterPrototype> _prototypes;
};
}