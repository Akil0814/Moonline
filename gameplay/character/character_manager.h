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


	private:
		std::vector<CharacterPrototype> _prototypes;
};
}