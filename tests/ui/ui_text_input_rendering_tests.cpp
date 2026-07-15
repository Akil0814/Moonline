#define SDL_MAIN_HANDLED

#include "engine/io/path/path_manager.h"
#include "engine/loading/content_manifest_pipeline.h"
#include "engine/localization/localization_manager.h"
#include "engine/resources/pipeline/resource_request_builder.h"
#include "engine/resources/resource_manager.h"
#include "engine/ui/widgets/ui_text_input.h"
#include "tests/support/test_assertions.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
using moonline::tests::require;

const elysia::core::UiRenderCommand* find_command(
    const std::vector<elysia::core::UiRenderCommand>& commands,
    elysia::core::UiRenderCommandType type)
{
    for (const auto& command : commands)
    {
        if (command.type == type)
            return &command;
    }
    return nullptr;
}

void test_text_input_uses_private_editing_texture()
{
    using namespace elysia;

    require(SDL_Init(SDL_INIT_VIDEO) == 0,"text input texture test must initialize SDL video");
    require(TTF_Init() == 0,"text input texture test must initialize SDL_ttf");

    SDL_Surface* target_surface = SDL_CreateRGBSurfaceWithFormat(
        0,256,128,32,SDL_PIXELFORMAT_RGBA32);
    require(target_surface != nullptr,"text input texture test must create a software target surface");
    SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(target_surface);
    require(renderer != nullptr,"text input texture test must create a software renderer");

    auto* path_manager = io::PathManager::instance();
    auto* resource_manager = resources::ResourceManager::instance();
    auto* localization_manager = localization::LocalizationManager::instance();
    require(path_manager->init(),"text input texture test must initialize the project path manager");

    resource_manager->clear();
    require(resource_manager->load_font("ui.latin.20",path_manager->fonts() / "fusion-pixel-10px-proportional-latin.ttf",20)
            && resource_manager->load_font("ui.latin.30",path_manager->fonts() / "fusion-pixel-10px-proportional-latin.ttf",30)
            && resource_manager->load_font("ui.zh_hans.20",path_manager->fonts() / "fusion-pixel-10px-proportional-zh_hans.ttf",20),
        "text input texture test must load the required localized fonts");

    localization_manager->shutdown();
    require(localization_manager->init(
            renderer,
            path_manager->configs() / "manifests" / "i18n_manifest.json",
            "en"),
        "text input texture test must initialize localization");

    {
        ui::UiTextInput input(core::Rect{ 0,0,240,48 });
        input.set_text("draft");

        localization::LocalizedTextStyle input_style;
        input_style.point_size = 30;
        input_style.color = input.style().text.enabled;
        SDL_Texture* shared_texture = localization_manager->get_raw_text_texture("draft",input_style);
        require(shared_texture != nullptr,"text input texture test must create the comparison cache texture");

        std::vector<core::UiRenderCommand> commands;
        input.submit_ui_render_commands(commands);
        const core::UiRenderCommand* first_command = find_command(commands,core::UiRenderCommandType::Texture);
        require(first_command && first_command->texture && first_command->texture != shared_texture,
            "input text must bypass the shared raw-text texture cache");
        SDL_Texture* first_private_texture = first_command->texture;

        commands.clear();
        input.submit_ui_render_commands(commands);
        const core::UiRenderCommand* repeated_command = find_command(commands,core::UiRenderCommandType::Texture);
        require(repeated_command && repeated_command->texture == first_private_texture,
            "unchanged input text must reuse its private texture");

        localization_manager->clear_texture_cache();
        commands.clear();
        input.submit_ui_render_commands(commands);
        const core::UiRenderCommand* after_cache_clear_command = find_command(commands,core::UiRenderCommandType::Texture);
        require(after_cache_clear_command && after_cache_clear_command->texture == first_private_texture,
            "clearing the shared cache must not discard unchanged input text");

        input.set_focused(true);
        require(input.on_ui_input_event(ui::UiInputEvent{
                    .type = ui::UiInputEventType::TextEditing,
                    .composition_start = 0,
                    .composition_length = 1,
                    .text = "x"
                }),
            "focused input text must accept an IME composition update");
        commands.clear();
        input.submit_ui_render_commands(commands);
        const core::UiRenderCommand* composition_command = find_command(commands,core::UiRenderCommandType::Texture);
        require(composition_command && composition_command->texture != first_private_texture,
            "IME composition changes must rebuild the private texture");

        SDL_Texture* composition_texture = composition_command->texture;
        input.set_text("changed");
        commands.clear();
        input.submit_ui_render_commands(commands);
        const core::UiRenderCommand* changed_text_command = find_command(commands,core::UiRenderCommandType::Texture);
        require(changed_text_command && changed_text_command->texture != composition_texture,
            "input text changes must rebuild the private texture");

        SDL_Texture* changed_text_texture = changed_text_command->texture;
        input.set_enabled(false);
        commands.clear();
        input.submit_ui_render_commands(commands);
        const core::UiRenderCommand* disabled_command = find_command(commands,core::UiRenderCommandType::Texture);
        require(disabled_command && disabled_command->texture != changed_text_texture,
            "effective text-color changes must rebuild the private texture");
        input.set_enabled(true);
        commands.clear();
        input.submit_ui_render_commands(commands);
        const core::UiRenderCommand* reenabled_command = find_command(commands,core::UiRenderCommandType::Texture);
        require(reenabled_command && reenabled_command->texture,
            "reenabled input text must render with a private texture");
        SDL_Texture* reenabled_texture = reenabled_command->texture;

        input.set_typography_role(ui::UiTypographyRole::InputPlaceholder);
        commands.clear();
        input.submit_ui_render_commands(commands);
        const core::UiRenderCommand* typography_command = find_command(commands,core::UiRenderCommandType::Texture);
        require(typography_command && typography_command->texture != reenabled_texture,
            "input typography changes must rebuild the private texture");
        SDL_Texture* typography_texture = typography_command->texture;

        require(localization_manager->set_language("zh_cn"),
            "text input texture test must switch to a loaded language");
        commands.clear();
        input.submit_ui_render_commands(commands);
        const core::UiRenderCommand* language_command = find_command(commands,core::UiRenderCommandType::Texture);
        require(language_command && language_command->texture != typography_texture,
            "language changes must rebuild the private texture");
        require(localization_manager->set_language("en"),
            "text input texture test must restore the default language");

        input.clear_text();
        input.set_placeholder_content(ui::ui_raw_text("placeholder"));
        localization::LocalizedTextStyle placeholder_style;
        placeholder_style.point_size = 20;
        placeholder_style.color = input.style().placeholder.enabled;
        SDL_Texture* placeholder_texture = localization_manager->get_raw_text_texture("placeholder",placeholder_style);
        require(placeholder_texture != nullptr,"text input texture test must create the placeholder cache texture");

        commands.clear();
        input.submit_ui_render_commands(commands);
        const core::UiRenderCommand* placeholder_command = find_command(commands,core::UiRenderCommandType::Texture);
        require(placeholder_command && placeholder_command->texture == placeholder_texture,
            "text input placeholders must continue using the shared text cache");

        input.reset();
        commands.clear();
        input.submit_ui_render_commands(commands);
        require(find_command(commands,core::UiRenderCommandType::Texture) == nullptr,
            "reset text input must release its private text texture");
    }

    localization_manager->shutdown();
    resource_manager->clear();
    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(target_surface);
    TTF_Quit();
    SDL_Quit();
}
}

int main()
{
    test_text_input_uses_private_editing_texture();
    std::cout << "ui text input rendering tests passed\n";
    return EXIT_SUCCESS;
}

