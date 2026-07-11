#pragma once

#include <string>

struct MainMeunEnterPayload
{
	bool play_theme_music = false;
};

struct StageSelectEnterPayload
{
	std::string character_id = {};
};
