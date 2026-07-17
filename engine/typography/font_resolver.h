#pragma once

#include "../application/application_presentation_settings.h"

#include <SDL_ttf.h>

#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace elysia::assist
{
class EngineAssistCache;
}

namespace elysia::resources
{
class ResourceManager;
}

namespace elysia::typography
{
enum class EffectTypographyRole
{
    FloatingNumber,
    Count
};

enum class FontResolveErrorCode
{
    NotConfigured,
    InvalidRole,
    InvalidPointSize,
    InvalidSource,
    UnsupportedLanguage,
    FontUnavailable
};

struct FontResolveError
{
    FontResolveErrorCode code = FontResolveErrorCode::FontUnavailable;
    std::string message;
};

struct ResolvedFont
{
    TTF_Font* font = nullptr;
    int point_size = 0;
    elysia::application::ApplicationFontSource source =
        elysia::application::ApplicationFontSource::EngineBuiltIn;
    std::uint64_t generation = 0;
};

class FontResolver
{
public:
    FontResolver() = default;
    ~FontResolver() = default;

    FontResolver(const FontResolver&) = delete;
    FontResolver& operator=(const FontResolver&) = delete;
    FontResolver(FontResolver&&) = delete;
    FontResolver& operator=(FontResolver&&) = delete;

    [[nodiscard]] std::expected<void,FontResolveError> configure(
        const elysia::application::ApplicationFontSettings& settings,
        const elysia::assist::EngineAssistCache& engine_assist_cache,
        const elysia::resources::ResourceManager& resource_manager,
        std::span<const std::string> supported_languages);
    void shutdown() noexcept;

    [[nodiscard]] std::expected<ResolvedFont,FontResolveError> resolve_ui(
        elysia::ui::UiTypographyRole role,
        std::string_view language) const;
    [[nodiscard]] std::expected<ResolvedFont,FontResolveError> resolve_effect(
        EffectTypographyRole role) const;

    [[nodiscard]] std::expected<void,FontResolveError>
        activate_project_fonts();
    void deactivate_project_fonts() noexcept;

    [[nodiscard]] bool configured() const noexcept;
    [[nodiscard]] bool project_fonts_active() const noexcept;
    [[nodiscard]] std::uint64_t generation() const noexcept;

private:
    [[nodiscard]] std::expected<ResolvedFont,FontResolveError> resolve(
        elysia::application::ApplicationFontSource configured_source,
        std::string_view language,
        int point_size) const;
    [[nodiscard]] std::expected<TTF_Font*,FontResolveError> find_font(
        elysia::application::ApplicationFontSource source,
        std::string_view language,
        int point_size) const;
    [[nodiscard]] std::expected<void,FontResolveError>
        validate_engine_fonts() const;
    [[nodiscard]] std::expected<void,FontResolveError>
        validate_project_fonts() const;

private:
    elysia::application::ApplicationFontSettings _settings;
    const elysia::assist::EngineAssistCache* _engine_assist_cache = nullptr;
    const elysia::resources::ResourceManager* _resource_manager = nullptr;
    std::vector<std::string> _supported_languages;
    std::uint64_t _generation = 0;
    bool _configured = false;
    bool _project_fonts_active = false;
};
}
