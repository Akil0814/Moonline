#define SDL_MAIN_HANDLED

#include "engine/io/path/path_manager.h"
#include "engine/localization/localization_manager.h"
#include "engine/resources/resource_manager.h"
#include "engine/ui/widgets/number/ui_number.h"
#include "tests/support/test_assertions.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <cstdlib>
#include <vector>

namespace
{
using moonline::tests::require;

std::vector<elysia::core::UiRenderCommand> texture_commands(
    const std::vector<elysia::core::UiRenderCommand>& commands
)
{
    std::vector<elysia::core::UiRenderCommand> textures;
    for (const auto& command : commands)
    {
        if (command.type == elysia::core::UiRenderCommandType::Texture)
            textures.push_back(command);
    }
    return textures;
}

void test_ui_number_uses_shared_localized_glyphs()
{
    using namespace elysia;
    require(SDL_Init(SDL_INIT_VIDEO) == 0,"UI number tests must initialize SDL video");
    require(TTF_Init() == 0,"UI number tests must initialize SDL_ttf");
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0,256,128,32,SDL_PIXELFORMAT_RGBA32);
    require(surface != nullptr,"UI number tests must create a software surface");
    SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(surface);
    require(renderer != nullptr,"UI number tests must create a software renderer");

    auto* paths = io::PathManager::instance();
    auto* resources = resources::ResourceManager::instance();
    auto* localization = localization::LocalizationManager::instance();
    require(paths->init(),"UI number tests must initialize paths");
    resources->clear();
    require(resources->load_font("ui.latin.30",paths->fonts() / "fusion-pixel-10px-proportional-latin.ttf",30)
        && resources->load_font("ui.zh_hans.30",paths->fonts() / "fusion-pixel-10px-proportional-zh_hans.ttf",30),
        "UI number tests must load localized number fonts");
    localization->shutdown();
    require(localization->init(
        renderer,
        paths->configs() / "manifests" / "i18n_manifest.json",
        "en"
    ),"UI number tests must initialize localization");

    {
        ui::UiNumber number(core::Rect{ 10,20,200,60 });
        number.set_value(-12.5);
        number.set_decimal_places(1);
        number.set_trim_trailing_zeros(false);
        number.set_suffix(ui::UiNumberSuffix::Percent);
        number.set_target_height(20.0f);
        number.set_digit_spacing(3.0f);
        number.set_fixed_glyph_advance(12.0f);
        number.set_horizontal_align(ui::TextHorizontalAlign::Center);
        number.set_vertical_align(ui::TextVerticalAlign::Bottom);
        number.set_opacity(128);

        std::vector<core::UiRenderCommand> commands;
        number.submit_ui_render_commands(commands);
        const auto first = texture_commands(commands);
        require(first.size() == 6,"negative decimal percent values must render one command per glyph");
        require(first.front().screen_rect.x() == 66.5f
            && first.front().screen_rect.y() == 60.0f
            && first.front().screen_rect.height() == 20.0f,
            "centered fixed-advance layout must preserve spacing and bottom alignment");
        require(first.front().alpha == 128,"widget opacity must apply to every number glyph");
        require(first.back().screen_rect.x() == 141.5f,"percent glyph must occupy the final fixed-advance column");

        commands.clear();
        number.submit_ui_render_commands(commands);
        const auto repeated = texture_commands(commands);
        require(repeated.size() == first.size(),"unchanged UI numbers must remain renderable");
        for (std::size_t index = 0; index < first.size(); ++index)
            require(repeated[index].texture == first[index].texture,"UI numbers must reuse localized glyph textures");

        localization->clear_texture_cache();
        commands.clear();
        number.submit_ui_render_commands(commands);
        require(texture_commands(commands).size() == 6,"UI numbers must rebuild after shared cache clearing");

        require(localization->set_language("zh_cn"),"UI number tests must switch language");
        commands.clear();
        number.submit_ui_render_commands(commands);
        require(texture_commands(commands).size() == 6,"UI number glyphs must follow the active language font mapping");

        number.set_horizontal_align(ui::TextHorizontalAlign::Right);
        number.set_vertical_align(ui::TextVerticalAlign::Top);
        commands.clear();
        number.submit_ui_render_commands(commands);
        const auto right_aligned = texture_commands(commands);
        require(!right_aligned.empty() && right_aligned.front().screen_rect.x() == 123.0f
            && right_aligned.front().screen_rect.y() == 20.0f,
            "right and top alignment must be applied by the UI widget");
    }

    localization->shutdown();
    resources->clear();
    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
    TTF_Quit();
    SDL_Quit();
}
}

int main()
{
    test_ui_number_uses_shared_localized_glyphs();
    return EXIT_SUCCESS;
}
