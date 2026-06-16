#pragma once

class ConfigManager;
class ResourceLoadPlan;

class ResourceRequestAssembler
{
public:
	bool assemble(const ConfigManager& config_manager, ResourceLoadPlan& out_plan) const;
};
