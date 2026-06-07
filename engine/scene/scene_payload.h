#pragma once
#include <string>
#include <variant>

struct EmptyScenePayload
{
};

struct GameplayEnterPayload
{
	int i = 0;
};

struct MainMenuEnterPayload
{
	std::string tmp = {};
};

using ScenePayload = std::variant<EmptyScenePayload,GameplayEnterPayload,MainMenuEnterPayload>;