#pragma once
#include "../io/file_manager.h"
#include "../io/json_loader.h"

#include "../tools/singleton.h"
#include "../core/error.h"

#include <SDL.h>

#include<vector>
#include <filesystem>

class ResourceBootstrapper : public Singleton<ResourceBootstrapper>
{
    friend Singleton<ResourceBootstrapper>;
public:
    bool bootstrap(const std::filesystem::path& start_path, SDL_Renderer* renderer);
    bool load_prload_resource();
    bool load_assets();

    std::vector<Error> get_error_list();

private:
    FileManager _file_manager;
    JsonLoader _json_loader;

    std::vector<std::filesystem::path> _all_file_path;

    std::vector<Error> _error_list;
};
