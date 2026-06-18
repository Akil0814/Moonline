#pragma once

#include "../io/loaders/asset_config_types.h"
#include "../tools/singleton.h"

class ConfigManager : public Singleton<ConfigManager>
{
	friend Singleton<ConfigManager>;

public:
	ConfigManager() = default;
	~ConfigManager() = default;


private:
};
