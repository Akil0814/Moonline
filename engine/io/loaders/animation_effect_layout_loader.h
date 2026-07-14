#pragma once
#include "asset_config_types.h"
#include <filesystem>
namespace elysia::io { class AnimationEffectLayoutLoader { public: bool load(const std::filesystem::path&, AnimationEffectLayout&) const; }; }
