#include "path_manager.h"

bool PathManager::init(const std::filesystem::path& start_path)
{
    std::optional<std::filesystem::path> root_path = find_project_root(start_path);

    if (!root_path.has_value())
        return false;

    _root = root_path.value();
    return true;
}

bool PathManager::ensure_runtime_dirs() const
{
    try
    {
        std::filesystem::create_directories(player_data());
        std::filesystem::create_directories(saves());
        std::filesystem::create_directories(logs());
        return true;
    }
    catch (const std::filesystem::filesystem_error&)
    {
        return false;
    }
}


const std::filesystem::path& PathManager::root() const
{
    return _root;
}

std::filesystem::path PathManager::assets() const
{
    return _root / "assets";
}

std::filesystem::path PathManager::configs() const
{
    return assets() / "configs";
}

std::filesystem::path PathManager::fonts() const
{
    return assets() / "fonts";
}

std::filesystem::path PathManager::preload() const
{
    return assets() / "preload";
}

std::filesystem::path PathManager::audio() const
{
    return assets() / "audio";
}

std::filesystem::path PathManager::textures() const
{
    return assets() / "textures";
}

std::filesystem::path PathManager::player_data() const
{
    return _root / "player_data";
}

std::filesystem::path PathManager::saves() const
{
    return player_data() / "saves";
}

std::filesystem::path PathManager::logs() const
{
    return _root / "logs";
}


std::optional<std::filesystem::path> PathManager::find_project_root(const std::filesystem::path& start_path) const
{
    constexpr int MAX_SEARCH_DEPTH = 16;

    try
    {
        std::filesystem::path current = std::filesystem::absolute(start_path);

        for (int depth = 0; depth < MAX_SEARCH_DEPTH; ++depth)
        {
            const std::filesystem::path assets_magic_file_path = current / "assets" / ".moonline_root";
            const std::filesystem::path assets_dir_path = current / "assets";

            const bool has_assets_magic_file = std::filesystem::exists(assets_magic_file_path);
            const bool assets_is_dir = std::filesystem::is_directory(assets_dir_path);

            if (has_assets_magic_file && assets_is_dir)
                return current;

            if (current == current.root_path())
                return std::nullopt;

            current = current.parent_path();
        }
    }
    catch (const std::filesystem::filesystem_error&)
    {
        return std::nullopt;
    }

    return std::nullopt;
}
