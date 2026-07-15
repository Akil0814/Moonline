#pragma once

#include <string>

namespace elysia::loading
{
struct ContentManifestResult;
class ResourceLoadPlan;

class ResourceRequestAssembler
{
public:
	bool assemble(const ContentManifestResult& config_result, ResourceLoadPlan& out_plan) const;
	[[nodiscard]] const std::string& error_message() const { return _error_message; }

private:
	mutable std::string _error_message;
};

}
