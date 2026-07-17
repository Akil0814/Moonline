#include "engine/assist/engine_assist_catalog.h"
#include "tests/support/test_assertions.h"

#include <chrono>
#include <filesystem>
#include <set>
#include <string>

namespace
{
using moonline::tests::require;
using elysia::assist::EngineAssistCatalog;
using elysia::assist::EngineAssistValidationErrorCode;

std::set<std::string> keys_of(
    const std::span<const elysia::assist::EngineAssistAssetDescriptor> descriptors)
{
    std::set<std::string> keys;
    for (const auto& descriptor : descriptors)
        keys.emplace(descriptor.key);
    return keys;
}

std::set<std::string> locales_of(
    const std::span<const elysia::assist::EngineAssistLocaleDescriptor> descriptors)
{
    std::set<std::string> locales;
    for (const auto& descriptor : descriptors)
        locales.emplace(descriptor.locale);
    return locales;
}

class TemporaryDirectory
{
public:
    explicit TemporaryDirectory(const std::filesystem::path& path)
        : _path(path)
    {
        std::error_code error;
        std::filesystem::remove_all(_path, error);
        require(!error, "temporary directory must be removable");
        std::filesystem::create_directories(_path, error);
        require(!error, "temporary directory must be creatable");
    }

    ~TemporaryDirectory()
    {
        std::error_code error;
        std::filesystem::remove_all(_path, error);
    }

private:
    std::filesystem::path _path;
};
}

int main()
{
    const std::filesystem::path source_root = MOONLINE_SOURCE_DIR;
    EngineAssistCatalog catalog(source_root);

    require(
        catalog.root() == source_root / "assets" / "engine",
        "assist root must be resolved below the project assets root"
    );
    require(catalog.fonts().size() == 5, "assist catalog must describe five font faces");
    require(catalog.textures().size() == 5, "assist catalog must describe five Elysia textures");
    require(catalog.locales().size() == 5, "assist catalog must describe five locales");
    require(
        keys_of(catalog.fonts()) == std::set<std::string>{
            "engine.font.ja",
            "engine.font.ko",
            "engine.font.latin",
            "engine.font.zh_hans",
            "engine.font.zh_hant",
        },
        "assist font keys must be stable"
    );
    require(
        keys_of(catalog.textures()) == std::set<std::string>{
            "engine.brand.elysia.black",
            "engine.brand.elysia.black_alpha_inverse",
            "engine.brand.elysia.default",
            "engine.brand.elysia.light_edge",
            "engine.brand.elysia.white",
        },
        "assist texture keys must be stable"
    );
    require(
        locales_of(catalog.locales()) == std::set<std::string>{
            "en", "ja", "ko", "zh-Hans", "zh-Hant"
        },
        "assist locales must use the Engine BCP-47 contract"
    );
    require(
        catalog.validate_required_files().has_value(),
        "all repository Engine assist files must be present"
    );

    const auto unique_suffix = std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count()
    );
    const std::filesystem::path temporary_root =
        std::filesystem::temp_directory_path() / ("elysia_assist_catalog_" + unique_suffix);
    TemporaryDirectory temporary_directory(temporary_root / "assets" / "engine");

    EngineAssistCatalog missing_file_catalog(temporary_root);
    const auto missing_file = missing_file_catalog.validate_required_files();
    require(!missing_file.has_value(), "catalog must reject a missing required resource");
    require(
        missing_file.error().code == EngineAssistValidationErrorCode::RequiredFileMissing,
        "existing assist root with no resources must report a required file failure"
    );
    require(
        missing_file.error().path == temporary_root / "assets" / "engine" / "fonts" / "NotoSans-Regular.ttf",
        "missing file failure must identify the first missing required asset"
    );

    EngineAssistCatalog missing_root_catalog(temporary_root / "missing");
    const auto missing_root = missing_root_catalog.validate_required_files();
    require(!missing_root.has_value(), "catalog must reject a missing assist root");
    require(
        missing_root.error().code == EngineAssistValidationErrorCode::RootMissing,
        "missing assist root must have a distinct diagnostic"
    );

    return 0;
}
