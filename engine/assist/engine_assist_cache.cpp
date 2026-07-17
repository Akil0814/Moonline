#include "engine_assist_cache.h"

#include "../io/json/strict_json.h"
#include "../resources/texture/surface_loader.h"
#include "../resources/texture/texture_loader.h"

#include <SDL.h>

#include <array>
#include <exception>
#include <string>
#include <utility>

namespace elysia::assist
{
namespace
{
constexpr std::array<int, 7> kPointSizes{10, 20, 30, 40, 50, 60, 70};

bool flatten_translation_json(
    const elysia::io::json& node,
    const std::string& prefix,
    EngineAssistTranslationTable& destination)
{
    if (node.is_string())
    {
        if (prefix.empty() || node.get_ref<const std::string&>().empty())
            return false;

        return destination.emplace(prefix, node.get<std::string>()).second;
    }

    if (!node.is_object() || node.empty())
        return false;

    for (auto iterator = node.begin(); iterator != node.end(); ++iterator)
    {
        const std::string child_prefix = prefix.empty()
            ? iterator.key()
            : prefix + "." + iterator.key();
        if (!flatten_translation_json(iterator.value(), child_prefix, destination))
            return false;
    }

    return true;
}

std::string make_prepare_error(std::string_view operation, const std::filesystem::path& path)
{
    return std::string(operation) + ": " + path.string();
}

std::string_view font_descriptor_locale(std::string_view locale) noexcept
{
    if (locale == "en" || locale == "latin")
        return "latin";
    if (locale == "zh-Hans" || locale == "zh_hans")
        return "zh_hans";
    if (locale == "zh-Hant" || locale == "zh_hant")
        return "zh_hant";
    if (locale == "ja")
        return "ja";
    if (locale == "ko")
        return "ko";
    return {};
}
}

void EngineAssistFontDeleter::operator()(TTF_Font* font) const noexcept
{
    if (font)
        TTF_CloseFont(font);
}

EngineAssistCache::~EngineAssistCache()
{
    shutdown();
}

std::expected<void, std::string> EngineAssistCache::initialize(
    SDL_Renderer* renderer,
    const EngineAssistCatalog& catalog)
{
    if (!renderer)
        return std::unexpected("Engine assist cache initialization failed: renderer is null.");
    if (_renderer && _renderer != renderer)
    {
        return std::unexpected(
            "Engine assist cache initialization failed: renderer changed; shutdown is required first.");
    }

    try
    {
        auto prepared = prepare(renderer, catalog);
        if (!prepared)
            return std::unexpected(prepared.error());

        _textures = std::move(prepared->textures);
        _fonts = std::move(prepared->fonts);
        _translations = std::move(prepared->translations);
        _atlases = std::move(prepared->atlases);
        _animations = std::move(prepared->animations);
        _renderer = renderer;
        return {};
    }
    catch (const std::exception& error)
    {
        return std::unexpected(
            std::string("Engine assist cache initialization failed: ") + error.what());
    }
}

void EngineAssistCache::shutdown() noexcept
{
    _animations.clear();
    _atlases.clear();
    _translations.clear();
    _fonts.clear();
    _textures.clear();
    _renderer = nullptr;
}

bool EngineAssistCache::initialized() const noexcept
{
    return _renderer != nullptr;
}

SDL_Texture* EngineAssistCache::find_texture(std::string_view key) const noexcept
{
    const auto found = _textures.find(std::string(key));
    return found == _textures.end() ? nullptr : found->second.get();
}

TTF_Font* EngineAssistCache::find_font(
    std::string_view locale,
    int point_size) const noexcept
{
    const std::string_view descriptor_locale = font_descriptor_locale(locale);
    if (descriptor_locale.empty())
        return nullptr;

    const auto found = _fonts.find(font_key(descriptor_locale, point_size));
    return found == _fonts.end() ? nullptr : found->second.get();
}

const std::string* EngineAssistCache::find_translation(
    std::string_view locale,
    std::string_view key) const noexcept
{
    const auto locale_found = _translations.find(std::string(locale));
    if (locale_found == _translations.end())
        return nullptr;

    const auto translation_found = locale_found->second.find(std::string(key));
    return translation_found == locale_found->second.end()
        ? nullptr
        : &translation_found->second;
}

const EngineAssistAnimationDefinition* EngineAssistCache::find_animation(
    std::string_view key) const noexcept
{
    const auto found = _animations.find(std::string(key));
    return found == _animations.end() ? nullptr : &found->second;
}

std::unique_ptr<elysia::animation::Animation> EngineAssistCache::create_animation(
    std::string_view key) const
{
    const EngineAssistAnimationDefinition* definition = find_animation(key);
    if (!definition || !definition->atlas || definition->fps <= 0.0)
        return nullptr;

    auto animation = std::make_unique<elysia::animation::Animation>();
    animation->set_atlas(definition->atlas);
    animation->set_loop(definition->loop);
    animation->set_interval_seconds(1.0 / definition->fps);
    return animation;
}

std::size_t EngineAssistCache::texture_count() const noexcept
{
    return _textures.size();
}

std::size_t EngineAssistCache::font_count() const noexcept
{
    return _fonts.size();
}

std::size_t EngineAssistCache::locale_count() const noexcept
{
    return _translations.size();
}

std::size_t EngineAssistCache::animation_count() const noexcept
{
    return _animations.size();
}

std::string EngineAssistCache::font_key(std::string_view locale, int point_size)
{
    return "engine.font." + std::string(locale) + "." + std::to_string(point_size);
}

std::string_view EngineAssistCache::map_project_locale(
    std::string_view project_locale) noexcept
{
    if (project_locale == "zh_cn" || project_locale == "zh-Hans")
        return "zh-Hans";
    if (project_locale == "zh_hant" || project_locale == "zh-Hant")
        return "zh-Hant";
    if (project_locale == "en" || project_locale == "ja" || project_locale == "ko")
        return project_locale;
    return {};
}

std::expected<EngineAssistCache::PreparedState, std::string> EngineAssistCache::prepare(
    SDL_Renderer* renderer,
    const EngineAssistCatalog& catalog) const
{
    if (const auto validation = catalog.validate_required_files(); !validation)
    {
        return std::unexpected(
            "Engine assist required resource validation failed: " + validation.error().path.string());
    }

    PreparedState prepared;
    elysia::resources::SurfaceLoader surface_loader;
    elysia::resources::TextureLoader texture_loader;

    for (const EngineAssistAssetDescriptor& descriptor : catalog.textures())
    {
        const std::filesystem::path path = catalog.resolve(descriptor.relative_path);
        const elysia::resources::SurfaceLoadResult surface = surface_loader.load_surface({
            ._asset_key = std::string(descriptor.key),
            ._frame_path = path,
            ._frame_index = 0
        });
        if (!surface._success)
            return std::unexpected(make_prepare_error("Engine assist texture surface load failed", path));

        elysia::resources::TextureLoadResult texture = texture_loader.load_texture(renderer, surface);
        if (!texture._success || !texture._texture)
            return std::unexpected(make_prepare_error("Engine assist texture creation failed", path));
        prepared.textures.emplace(std::string(descriptor.key), std::move(texture._texture));
    }

    for (const EngineAssistAssetDescriptor& descriptor : catalog.fonts())
    {
        const std::filesystem::path path = catalog.resolve(descriptor.relative_path);
        const std::string_view locale = std::string_view(descriptor.key).substr(
            std::string_view("engine.font.").size());
        for (const int point_size : kPointSizes)
        {
            TTF_Font* raw_font = TTF_OpenFont(path.string().c_str(), point_size);
            if (!raw_font)
                return std::unexpected(make_prepare_error("Engine assist font load failed", path));
            prepared.fonts.emplace(font_key(locale, point_size), EngineAssistFontPtr(raw_font));
        }
    }

    for (const EngineAssistAnimationDescriptor& descriptor : catalog.animations())
    {
        const auto texture_found = prepared.textures.find(std::string(descriptor.texture_key));
        if (texture_found == prepared.textures.end() || !texture_found->second)
        {
            return std::unexpected(
                "Engine assist animation texture is not registered: " + std::string(descriptor.texture_key));
        }

        int texture_width = 0;
        int texture_height = 0;
        if (SDL_QueryTexture(texture_found->second.get(), nullptr, nullptr,
                &texture_width, &texture_height) != 0
            || !descriptor.has_expected_texture_dimensions(texture_width, texture_height))
        {
            return std::unexpected(
                "Engine assist animation texture dimensions are invalid: " + std::string(descriptor.key));
        }

        auto atlas = std::make_unique<elysia::resources::Atlas>(std::string(descriptor.key));
        for (std::size_t frame_index = 0; frame_index < descriptor.frame_count; ++frame_index)
        {
            const elysia::core::Rect source_rect(
                static_cast<float>(frame_index * static_cast<std::size_t>(descriptor.frame_width)),
                0.0f,
                static_cast<float>(descriptor.frame_width),
                static_cast<float>(descriptor.frame_height));
            if (!atlas->add_frame({}, texture_found->second.get(), source_rect))
            {
                return std::unexpected(
                    "Engine assist animation atlas build failed: " + std::string(descriptor.key));
            }
        }

        const elysia::resources::Atlas* atlas_pointer = atlas.get();
        if (!prepared.atlases.emplace(std::string(descriptor.key), std::move(atlas)).second
            || !prepared.animations.emplace(
                std::string(descriptor.key),
                EngineAssistAnimationDefinition{
                    .key = std::string(descriptor.key),
                    .atlas = atlas_pointer,
                    .fps = descriptor.fps,
                    .loop = descriptor.loop
                }).second)
        {
            return std::unexpected(
                "Engine assist animation key is duplicated: " + std::string(descriptor.key));
        }
    }

    for (const EngineAssistLocaleDescriptor& descriptor : catalog.locales())
    {
        const std::filesystem::path path = catalog.resolve(descriptor.relative_path);
        const auto document = elysia::io::load_strict_json(path);
        if (!document)
            return std::unexpected("Engine assist i18n load failed: " + document.error());
        if (!document->is_object() || document->size() != 1 || !document->contains("engine"))
            return std::unexpected(make_prepare_error("Engine assist i18n root is invalid", path));

        EngineAssistTranslationTable table;
        if (!flatten_translation_json(*document, "", table))
            return std::unexpected(make_prepare_error("Engine assist i18n structure is invalid", path));
        prepared.translations.emplace(std::string(descriptor.locale), std::move(table));
    }

    return prepared;
}
}
