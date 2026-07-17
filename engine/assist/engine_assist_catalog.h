#pragma once

#include "../io/path/path_manager.h"

#include <expected>
#include <filesystem>
#include <span>
#include <string_view>

namespace elysia::assist
{
struct EngineAssistAssetDescriptor
{
    std::string_view key;
    std::filesystem::path relative_path;
};

struct EngineAssistLocaleDescriptor
{
    std::string_view locale;
    std::filesystem::path relative_path;
};

enum class EngineAssistValidationErrorCode
{
    RootMissing,
    RequiredMarkerMissing,
    RequiredFileMissing
};

struct EngineAssistValidationError
{
    EngineAssistValidationErrorCode code;
    std::filesystem::path path;
};

class EngineAssistCatalog
{
public:
    explicit EngineAssistCatalog(std::filesystem::path project_root);
    explicit EngineAssistCatalog(const elysia::io::PathManager& path_manager);

    [[nodiscard]] const std::filesystem::path& root() const noexcept;
    [[nodiscard]] std::filesystem::path required_marker_path() const;
    [[nodiscard]] std::span<const EngineAssistAssetDescriptor> fonts() const noexcept;
    [[nodiscard]] std::span<const EngineAssistAssetDescriptor> textures() const noexcept;
    [[nodiscard]] std::span<const EngineAssistLocaleDescriptor> locales() const noexcept;
    [[nodiscard]] std::filesystem::path resolve(
        const std::filesystem::path& relative_path) const;
    [[nodiscard]] std::expected<void, EngineAssistValidationError>
        validate_required_files() const;

private:
    std::filesystem::path _root;
};
}
