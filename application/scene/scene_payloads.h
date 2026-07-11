#pragma once

#include <string>

struct MainMeunEnterPayload
{
	bool replay_theme_music = false;
};

struct StageSelectEnterPayload
{
	std::string character_id = {};
};
