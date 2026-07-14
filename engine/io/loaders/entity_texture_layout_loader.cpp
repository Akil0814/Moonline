#include "../../tools/logger.h"
#include "entity_texture_layout_loader.h"
#include "../json/json_loader.h"
namespace elysia::io { bool EntityTextureLayoutLoader::load(const std::filesystem::path& path, EntityTextureLayout& layout) const { layout={}; JsonLoader loader; if(!loader.open_file(path)||!loader.root().is_object()){ ELYSIA_LOG_WARN("io","Load entity texture layout failed: "<<path); return false;} for(auto it=loader.root().begin();it!=loader.root().end();++it){if(!it.value().is_string())return false; layout.textures.push_back({it.key(),it.value().get<std::string>()});} return true; } }
