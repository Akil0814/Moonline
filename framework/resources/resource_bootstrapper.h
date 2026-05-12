#pragma once
#include "../io/file_manager.h"
#include "../io/path_manager.h"

#include "../base/singleton.h"
#include "../core/error.h"

#include<vector>
#include <filesystem>

class ResourceBootstrapper : public Singleton<ResourceBootstrapper>
{
    friend Singleton<ResourceBootstrapper>;
public:
    bool bootstrap(const std::filesystem::path& start_path);
    bool load_prload_resource();
    bool load_assets();

    std::vector<Error> get_error_list();

private:
    PathManager _path_manager;
    FileManager _file_manager;

    std::vector<Error> _error_list;
};