#pragma once

#include "../io/path/path_manager.h"

#include <expected>
#include <filesystem>
#include <cstddef>
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

struct EngineAssistAudioDescriptor
{
    std::string_view key;
    std::filesystem::path relative_path;
};

struct EngineAssistAnimationDescriptor
{
    std::string_view key;
    std::string_view texture_key;
    int frame_width = 0;
    int frame_height = 0;
    std::size_t frame_count = 0;
    double fps = 0.0;
    bool loop = false;

    [[nodiscard]] bool has_expected_texture_dimensions(int width, int height) const noexcept
    {
        return frame_width > 0
            && frame_height > 0
            && frame_count > 0
            && width == frame_width * static_cast<int>(frame_count)
            && height == frame_height;
    }
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
    [[nodiscard]] std::span<const EngineAssistAnimationDescriptor> animations() const noexcept;
    [[nodiscard]] std::span<const EngineAssistAudioDescriptor> sounds() const noexcept;
    [[nodiscard]] std::span<const EngineAssistAudioDescriptor> music() const noexcept;
    [[nodiscard]] std::filesystem::path resolve(const std::filesystem::path& relative_path) const;
    [[nodiscard]] std::expected<void, EngineAssistValidationError>validate_required_files() const;

private:
    std::filesystem::path _root;
};
}
