#pragma once

#include <string>

namespace elysia::loading
{
struct ConfigLoadResult;
class ResourceLoadPlan;

class ResourceRequestAssembler
{
public:
	bool assemble(const ConfigLoadResult& config_result, ResourceLoadPlan& out_plan) const;
	[[nodiscard]] const std::string& error_message() const { return _error_message; }

private:
	mutable std::string _error_message;
};

}
