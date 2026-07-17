#include "engine_assist_catalog.h"

#include <array>
#include <system_error>
#include <utility>

namespace elysia::assist
{
namespace
{
const std::array<EngineAssistAssetDescriptor, 5> kFontDescriptors = {
    EngineAssistAssetDescriptor{"engine.font.latin", "fonts/NotoSans-Regular.ttf"},
    EngineAssistAssetDescriptor{"engine.font.zh_hans", "fonts/NotoSansSC-Regular.ttf"},
    EngineAssistAssetDescriptor{"engine.font.zh_hant", "fonts/NotoSansTC-Regular.ttf"},
    EngineAssistAssetDescriptor{"engine.font.ja", "fonts/NotoSansJP-Regular.ttf"},
    EngineAssistAssetDescriptor{"engine.font.ko", "fonts/NotoSansKR-Regular.ttf"},
};

const std::array<EngineAssistAssetDescriptor, 5> kTextureDescriptors = {
    EngineAssistAssetDescriptor{"engine.brand.elysia.default", "textures/elysia.png"},
    EngineAssistAssetDescriptor{"engine.brand.elysia.black", "textures/elysia_black.png"},
    EngineAssistAssetDescriptor{
        "engine.brand.elysia.black_alpha_inverse",
        "textures/elysia_black_alpha_inverse.png"
    },
    EngineAssistAssetDescriptor{"engine.brand.elysia.light_edge", "textures/elysia_light_edge.png"},
    EngineAssistAssetDescriptor{"engine.brand.elysia.white", "textures/elysia_white.png"},
};

const std::array<EngineAssistLocaleDescriptor, 5> kLocaleDescriptors = {
    EngineAssistLocaleDescriptor{"en", "i18n/en/engine.json"},
    EngineAssistLocaleDescriptor{"zh-Hans", "i18n/zh-Hans/engine.json"},
    EngineAssistLocaleDescriptor{"zh-Hant", "i18n/zh-Hant/engine.json"},
    EngineAssistLocaleDescriptor{"ja", "i18n/ja/engine.json"},
    EngineAssistLocaleDescriptor{"ko", "i18n/ko/engine.json"},
};

constexpr std::string_view kRequiredMarkerFileName = ".elysia_engine_required";
}

EngineAssistCatalog::EngineAssistCatalog(std::filesystem::path project_root)
    : _root(std::move(project_root) / "assets" / "engine")
{
}

EngineAssistCatalog::EngineAssistCatalog(const elysia::io::PathManager& path_manager)
    : _root(path_manager.assets() / "engine")
{
}

const std::filesystem::path& EngineAssistCatalog::root() const noexcept
{
    return _root;
}

std::filesystem::path EngineAssistCatalog::required_marker_path() const
{
    return _root / kRequiredMarkerFileName;
}

std::span<const EngineAssistAssetDescriptor> EngineAssistCatalog::fonts() const noexcept
{
    return kFontDescriptors;
}

std::span<const EngineAssistAssetDescriptor> EngineAssistCatalog::textures() const noexcept
{
    return kTextureDescriptors;
}

std::span<const EngineAssistLocaleDescriptor> EngineAssistCatalog::locales() const noexcept
{
    return kLocaleDescriptors;
}

std::filesystem::path EngineAssistCatalog::resolve(
    const std::filesystem::path& relative_path) const
{
    return _root / relative_path;
}

std::expected<void, EngineAssistValidationError>
EngineAssistCatalog::validate_required_files() const
{
    std::error_code error;
    if (!std::filesystem::is_directory(_root, error))
    {
        return std::unexpected(EngineAssistValidationError{
            .code = EngineAssistValidationErrorCode::RootMissing,
            .path = _root
        });
    }

    const std::filesystem::path required_marker = required_marker_path();
    if (!std::filesystem::is_regular_file(required_marker, error))
    {
        return std::unexpected(EngineAssistValidationError{
            .code = EngineAssistValidationErrorCode::RequiredMarkerMissing,
            .path = required_marker
        });
    }

    const auto validate_asset = [this](const auto& descriptor)
        -> std::expected<void, EngineAssistValidationError>
    {
        const std::filesystem::path path = resolve(descriptor.relative_path);
        std::error_code error;
        if (!std::filesystem::is_regular_file(path, error))
        {
            return std::unexpected(EngineAssistValidationError{
                .code = EngineAssistValidationErrorCode::RequiredFileMissing,
                .path = path
            });
        }

        return {};
    };

    for (const EngineAssistAssetDescriptor& descriptor : kFontDescriptors)
    {
        if (const auto result = validate_asset(descriptor); !result)
            return result;
    }
    for (const EngineAssistAssetDescriptor& descriptor : kTextureDescriptors)
    {
        if (const auto result = validate_asset(descriptor); !result)
            return result;
    }
    for (const EngineAssistLocaleDescriptor& descriptor : kLocaleDescriptors)
    {
        if (const auto result = validate_asset(descriptor); !result)
            return result;
    }

    return {};
}
}
