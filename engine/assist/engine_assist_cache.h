#pragma once

#include "engine_assist_catalog.h"
#include "../animation/animation.h"
#include "../resources/atlas/atlas.h"
#include "../resources/texture/texture_loader.h"

#include <SDL_ttf.h>

#include <expected>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

struct SDL_Renderer;

namespace elysia::assist
{
struct EngineAssistFontDeleter
{
    void operator()(TTF_Font* font) const noexcept;
};

using EngineAssistFontPtr = std::unique_ptr<TTF_Font, EngineAssistFontDeleter>;
using EngineAssistTranslationTable = std::unordered_map<std::string, std::string>;

struct EngineAssistAnimationDefinition
{
    std::string key;
    const elysia::resources::Atlas* atlas = nullptr;
    double fps = 0.0;
    bool loop = false;
};

class EngineAssistCache
{
public:
    EngineAssistCache() = default;
    ~EngineAssistCache();

    EngineAssistCache(const EngineAssistCache&) = delete;
    EngineAssistCache& operator=(const EngineAssistCache&) = delete;
    EngineAssistCache(EngineAssistCache&&) = delete;
    EngineAssistCache& operator=(EngineAssistCache&&) = delete;

    [[nodiscard]] std::expected<void, std::string> initialize(
        SDL_Renderer* renderer,
        const EngineAssistCatalog& catalog,
        std::span<const int> point_sizes);
    void shutdown() noexcept;

    [[nodiscard]] bool initialized() const noexcept;
    [[nodiscard]] SDL_Texture* find_texture(std::string_view key) const noexcept;
    [[nodiscard]] TTF_Font* find_font(
        std::string_view locale,
        int point_size) const noexcept;
    [[nodiscard]] const std::string* find_translation(
        std::string_view locale,
        std::string_view key) const noexcept;
    [[nodiscard]] const EngineAssistAnimationDefinition* find_animation(
        std::string_view key) const noexcept;
    [[nodiscard]] std::unique_ptr<elysia::animation::Animation> create_animation(
        std::string_view key) const;

    [[nodiscard]] std::size_t texture_count() const noexcept;
    [[nodiscard]] std::size_t font_count() const noexcept;
    [[nodiscard]] std::size_t locale_count() const noexcept;
    [[nodiscard]] std::size_t animation_count() const noexcept;

    [[nodiscard]] static std::string font_key(
        std::string_view locale,
        int point_size);
    [[nodiscard]] static std::string_view map_project_locale(
        std::string_view project_locale) noexcept;

private:
    using TextureMap = std::unordered_map<std::string, elysia::resources::TexturePtr>;
    using FontMap = std::unordered_map<std::string, EngineAssistFontPtr>;
    using TranslationTables = std::unordered_map<std::string, EngineAssistTranslationTable>;
    using AtlasMap = std::unordered_map<std::string, std::unique_ptr<elysia::resources::Atlas>>;
    using AnimationDefinitions = std::unordered_map<std::string, EngineAssistAnimationDefinition>;

    struct PreparedState
    {
        TextureMap textures;
        FontMap fonts;
        TranslationTables translations;
        AtlasMap atlases;
        AnimationDefinitions animations;
    };

    [[nodiscard]] std::expected<PreparedState, std::string> prepare(
        SDL_Renderer* renderer,
        const EngineAssistCatalog& catalog,
        std::span<const int> point_sizes) const;

private:
    TextureMap _textures;
    FontMap _fonts;
    TranslationTables _translations;
    AtlasMap _atlases;
    AnimationDefinitions _animations;
    SDL_Renderer* _renderer = nullptr;
};
}
