#pragma once

#include "engine_assist_catalog.h"
#include "../resources/texture/texture_loader.h"

#include <SDL_ttf.h>

#include <expected>
#include <memory>
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
        const EngineAssistCatalog& catalog);
    void shutdown() noexcept;

    [[nodiscard]] bool initialized() const noexcept;
    [[nodiscard]] SDL_Texture* find_texture(std::string_view key) const noexcept;
    [[nodiscard]] TTF_Font* find_font(
        std::string_view locale,
        int point_size) const noexcept;
    [[nodiscard]] const std::string* find_translation(
        std::string_view locale,
        std::string_view key) const noexcept;

    [[nodiscard]] std::size_t texture_count() const noexcept;
    [[nodiscard]] std::size_t font_count() const noexcept;
    [[nodiscard]] std::size_t locale_count() const noexcept;

    [[nodiscard]] static std::string font_key(
        std::string_view locale,
        int point_size);
    [[nodiscard]] static std::string_view map_project_locale(
        std::string_view project_locale) noexcept;

private:
    using TextureMap = std::unordered_map<std::string, elysia::resources::TexturePtr>;
    using FontMap = std::unordered_map<std::string, EngineAssistFontPtr>;
    using TranslationTables = std::unordered_map<std::string, EngineAssistTranslationTable>;

    struct PreparedState
    {
        TextureMap textures;
        FontMap fonts;
        TranslationTables translations;
    };

    [[nodiscard]] std::expected<PreparedState, std::string> prepare(
        SDL_Renderer* renderer,
        const EngineAssistCatalog& catalog) const;

private:
    TextureMap _textures;
    FontMap _fonts;
    TranslationTables _translations;
    SDL_Renderer* _renderer = nullptr;
};
}
