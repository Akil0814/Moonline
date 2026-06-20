#pragma once

namespace elysia::loading
{
struct ConfigLoadResult;
class ResourceLoadPlan;

class ResourceRequestAssembler
{
public:
	bool assemble(const ConfigLoadResult& config_result, ResourceLoadPlan& out_plan) const;
};

}
