#include "json_loader.h"

#include <fstream>

void JsonLoader::add_error_message(
    JsonReadResult& result,
    const std::string& error
)
{
    result.error += error;
    result.error += '\n';
}

JsonReadResult JsonLoader::open_file(
    const std::filesystem::path& path
)
{

    JsonReadResult result;

    if (path.empty())
    {
        add_error_message(result, "JSON path is empty.");
        return result;
    }

    std::ifstream file(path);

    if (!file.is_open())
    {
        add_error_message(result, "JSON open failed: " + path.string());
        return result;
    }

    try
    {
        file >> _root;
    }
    catch (const std::exception& error)
    {
        add_error_message(result, "JSON parse failed: " + path.string());
        add_error_message(result, std::string("Reason: ") + error.what());
        return result;
    }

    return result;
}