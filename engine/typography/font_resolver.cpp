#include "font_resolver.h"

#include "../assist/engine_assist_cache.h"
#include "../resources/resource_manager.h"

#include <unordered_set>

namespace elysia::typography
{
namespace
{
[[nodiscard]] FontResolveError error(
    FontResolveErrorCode code,
    std::string message)
{
    return FontResolveError{ .code = code,.message = std::move(message) };
}

[[nodiscard]] std::string project_font_key(
    std::string_view language,
    int point_size)
{
    std::string prefix;
    if (language == "en")
        prefix = "ui.latin";
    else if (language == "zh_cn" || language == "zh-Hans")
        prefix = "ui.zh_hans";
    else if (language == "zh_hant" || language == "zh-Hant")
        prefix = "ui.zh_hant";
    else if (language == "ja")
        prefix = "ui.ja";
    else if (language == "ko")
        prefix = "ui.ko";
    else
        return {};

    return prefix + "." + std::to_string(point_size);
}
}

std::expected<void,FontResolveError> FontResolver::configure(
    const ResolvedFontSettings& settings,
    const elysia::assist::EngineAssistCache& engine_assist_cache,
    const elysia::resources::ResourceManager& resource_manager,
    std::span<const std::string> supported_languages)
{
    shutdown();

    if (supported_languages.empty())
    {
        return std::unexpected(error(
            FontResolveErrorCode::UnsupportedLanguage,
            "FontResolver requires at least one supported language."));
    }

    _settings = settings;
    _engine_assist_cache = &engine_assist_cache;
    _resource_manager = &resource_manager;
    _supported_languages.assign(
        supported_languages.begin(),
        supported_languages.end());

    if (const auto validation = validate_engine_fonts();!validation)
    {
        shutdown();
        return validation;
    }

    _configured = true;
    _generation = 1;
    return {};
}

void FontResolver::shutdown() noexcept
{
    _settings.reset();
    _engine_assist_cache = nullptr;
    _resource_manager = nullptr;
    _supported_languages.clear();
    _project_fonts_active = false;
    _configured = false;
    ++_generation;
}

std::expected<ResolvedFont,FontResolveError> FontResolver::resolve_ui(
    UiTypographyRole role,
    std::string_view language) const
{
    if (!_configured || !_settings)
    {
        return std::unexpected(error(
            FontResolveErrorCode::NotConfigured,
            "FontResolver is not configured."));
    }

    const std::size_t role_index = static_cast<std::size_t>(role);
    if (role_index >= UiTypographyProfile::RoleCount)
    {
        return std::unexpected(error(
            FontResolveErrorCode::InvalidRole,
            "FontResolver received an invalid UI typography role."));
    }

    return resolve(
        _settings->ui_source(),
        language,
        _settings->ui_typography().point_size(role));
}

std::expected<ResolvedFont,FontResolveError> FontResolver::resolve_effect(
    EffectTypographyRole role) const
{
    if (!_configured || !_settings)
    {
        return std::unexpected(error(
            FontResolveErrorCode::NotConfigured,
            "FontResolver is not configured."));
    }

    if (role != EffectTypographyRole::FloatingNumber)
    {
        return std::unexpected(error(
            FontResolveErrorCode::InvalidRole,
            "FontResolver received an invalid effect typography role."));
    }

    return resolve(
        _settings->floating_number_source(),
        "en",
        _settings->floating_number_point_size());
}

std::expected<void,FontResolveError> FontResolver::activate_project_fonts()
{
    if (!_configured)
    {
        return std::unexpected(error(
            FontResolveErrorCode::NotConfigured,
            "FontResolver is not configured."));
    }

    const bool project_fonts_requested =
        _settings->ui_source() == FontSource::Project
        || _settings->floating_number_source() == FontSource::Project;
    if (!project_fonts_requested || _project_fonts_active)
        return {};

    if (const auto validation = validate_project_fonts();!validation)
        return validation;

    _project_fonts_active = true;
    ++_generation;
    return {};
}

void FontResolver::deactivate_project_fonts() noexcept
{
    if (!_project_fonts_active)
        return;

    _project_fonts_active = false;
    ++_generation;
}

bool FontResolver::configured() const noexcept
{
    return _configured;
}

bool FontResolver::project_fonts_active() const noexcept
{
    return _project_fonts_active;
}

std::uint64_t FontResolver::generation() const noexcept
{
    return _generation;
}

std::span<const int> FontResolver::project_point_sizes() const noexcept
{
    return _settings
        ? _settings->project_point_sizes()
        : std::span<const int>{};
}

std::expected<ResolvedFont,FontResolveError> FontResolver::resolve(
    FontSource configured_source,
    std::string_view language,
    int point_size) const
{
    if (!_configured)
    {
        return std::unexpected(error(
            FontResolveErrorCode::NotConfigured,
            "FontResolver is not configured."));
    }
    if (point_size <= 0)
    {
        return std::unexpected(error(
            FontResolveErrorCode::InvalidPointSize,
            "FontResolver point size must be positive."));
    }

    const FontSource active_source =
        configured_source == FontSource::Project
            && _project_fonts_active
        ? FontSource::Project
        : FontSource::EngineBuiltIn;
    const auto font = find_font(active_source,language,point_size);
    if (!font)
        return std::unexpected(font.error());

    return ResolvedFont{
        .font = *font,
        .point_size = point_size,
        .source = active_source,
        .generation = _generation
    };
}

std::expected<TTF_Font*,FontResolveError> FontResolver::find_font(
    FontSource source,
    std::string_view language,
    int point_size) const
{
    if (source == FontSource::EngineBuiltIn)
    {
        const std::string_view engine_locale =
            elysia::assist::EngineAssistCache::map_project_locale(language);
        if (engine_locale.empty())
        {
            return std::unexpected(error(
                FontResolveErrorCode::UnsupportedLanguage,
                "FontResolver has no Engine locale mapping for language: "
                    + std::string(language)));
        }

        TTF_Font* font = _engine_assist_cache
            ? _engine_assist_cache->find_font(engine_locale,point_size)
            : nullptr;
        if (font)
            return font;
    }
    else if (source == FontSource::Project)
    {
        const std::string key = project_font_key(language,point_size);
        if (key.empty())
        {
            return std::unexpected(error(
                FontResolveErrorCode::UnsupportedLanguage,
                "FontResolver has no project font mapping for language: "
                    + std::string(language)));
        }

        TTF_Font* font = _resource_manager
            ? _resource_manager->find_font(key)
            : nullptr;
        if (font)
            return font;
    }

    return std::unexpected(error(
        FontResolveErrorCode::FontUnavailable,
        "FontResolver could not find a "
            + std::string(source == FontSource::Project
                ? "project" : "Engine")
            + " font for language " + std::string(language)
            + " at " + std::to_string(point_size) + "pt."));
}

std::expected<void,FontResolveError> FontResolver::validate_engine_fonts() const
{
    std::unordered_set<int> ui_sizes(
        _settings->ui_typography().point_sizes().begin(),
        _settings->ui_typography().point_sizes().end());
    for (const std::string& language : _supported_languages)
    {
        for (const int point_size : ui_sizes)
        {
            if (const auto font = find_font(
                    FontSource::EngineBuiltIn,
                    language,
                    point_size);
                !font)
            {
                return std::unexpected(font.error());
            }
        }
    }

    if (const auto font = find_font(
            FontSource::EngineBuiltIn,
            "en",
            _settings->floating_number_point_size());
        !font)
    {
        return std::unexpected(font.error());
    }

    return {};
}

std::expected<void,FontResolveError> FontResolver::validate_project_fonts() const
{
    if (_settings->ui_source() == FontSource::Project)
    {
        std::unordered_set<int> ui_sizes(
            _settings->ui_typography().point_sizes().begin(),
            _settings->ui_typography().point_sizes().end());
        for (const std::string& language : _supported_languages)
        {
            for (const int point_size : ui_sizes)
            {
                if (const auto font = find_font(
                        FontSource::Project,
                        language,
                        point_size);
                    !font)
                {
                    return std::unexpected(font.error());
                }
            }
        }
    }

    if (_settings->floating_number_source() == FontSource::Project)
    {
        if (const auto font = find_font(
            FontSource::Project,
            "en",
            _settings->floating_number_point_size());
            !font)
        {
            return std::unexpected(font.error());
        }
    }

    return {};
}
}
