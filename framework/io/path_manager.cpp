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
    std::error_code error_c;

    std::filesystem::create_directories(player_data(), error_c);
    if (error_c)
        return false;

    std::filesystem::create_directories(saves(), error_c);
    if (error_c)
        return false;

    std::filesystem::create_directories(logs(), error_c);
    if (error_c)
        return false;

    return true;
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

    std::error_code error_c;

    std::filesystem::path current = std::filesystem::absolute(start_path, error_c);

    if (error_c)
        return std::nullopt;

    for (int depth = 0; depth < MAX_SEARCH_DEPTH; ++depth)
    {
        const std::filesystem::path magic_file_path = current / ".moonline_root";
        const std::filesystem::path assets_dir_path = current / "assets";

        error_c.clear();
        const bool has_magic_file = std::filesystem::exists(magic_file_path, error_c);

        if (error_c)
            return std::nullopt;

        error_c.clear();
        const bool assets_exists = std::filesystem::exists(assets_dir_path, error_c);

        if (error_c)
            return std::nullopt;

        error_c.clear();
        const bool assets_is_dir = std::filesystem::is_directory(assets_dir_path, error_c);

        if (error_c)
            return std::nullopt;

        const bool has_assets_dir = assets_exists && assets_is_dir;

        if (has_magic_file && has_assets_dir)
            return current;

        if (current == current.root_path())
            return std::nullopt;

        current = current.parent_path();
    }

    return std::nullopt;
}
