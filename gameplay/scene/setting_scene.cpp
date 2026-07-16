#include "setting_scene.h"

#include "../../application/scene/scene_keys.h"
#include "../../application/scene/scene_payloads.h"

#include "../../engine/config/user_config_service.h"
#include "../../engine/ui/containers/ui_list_container.h"
#include "../../engine/ui/layout/ui_layout_types.h"
#include "../../engine/ui/text/ui_text_content.h"
#include "../../engine/ui/text/ui_typography.h"
#include "../../engine/ui/widgets/label/ui_label.h"
#include "../../engine/ui/widgets/ui_button.h"
#include "../../engine/ui/widgets/ui_checkbox.h"
#include "../../engine/ui/composites/ui_dropdown.h"
#include "../../engine/ui/widgets/ui_slider.h"

#include <memory>
#include <utility>

namespace arcneco::scene
{
namespace
{
constexpr float field_width = 620.0f;
constexpr float label_width = 190.0f;
constexpr float control_width = 410.0f;
constexpr float row_height = 48.0f;

std::unique_ptr<elysia::ui::UiLabel> make_label(
    const char* text,float width = label_width,elysia::ui::UiTypographyRole role = elysia::ui::UiTypographyRole::Label)
{
    auto label = std::make_unique<elysia::ui::UiLabel>(
        elysia::core::Rect{ 0,0,width,row_height },0,elysia::ui::ui_raw_text(text));
    label->set_typography_role(role);
    label->set_horizontal_align(elysia::ui::TextHorizontalAlign::Left);
    label->set_vertical_align(elysia::ui::TextVerticalAlign::Center);
    return label;
}

std::unique_ptr<elysia::ui::UiListContainer> make_field_row(
    const char* label_text,std::unique_ptr<elysia::ui::UiElement> control)
{
    auto row = std::make_unique<elysia::ui::UiListContainer>(
        elysia::core::Rect{ 0,0,field_width,row_height });
    row->set_direction(elysia::ui::UiListDirection::Horizontal);
    row->set_item_spacing(20.0f);
    row->add_back(make_label(label_text));
    row->add_back(std::move(control));
    return row;
}

std::unique_ptr<elysia::ui::UiSlider> make_volume_slider(float value)
{
    elysia::ui::UiSliderConfig config{};
    config.min_value = 0.0f;
    config.max_value = 100.0f;
    config.value = value;
    config.step = 1.0f;
    config.value_display = elysia::ui::UiSliderValueDisplay::Percent;
    return std::make_unique<elysia::ui::UiSlider>(
        elysia::core::Rect{ 0,0,control_width,row_height },config);
}
}

void SettingScene::on_update(double delta)
{
    elysia::scene::Scene::on_update(delta);
}

void SettingScene::on_render(SDL_Renderer* renderer)
{
    elysia::scene::Scene::on_render(renderer);
}

void SettingScene::on_input(
    const elysia::input::RawInputFrame& input,
    const std::vector<elysia::input::RawInputEvent>& events)
{
    elysia::scene::Scene::on_input(input,events);
}

void SettingScene::on_enter(const elysia::scene::ScenePayload& payload)
{
    (void)payload;
    if (!_main_setting_window || _main_setting_window->is_destroyed())
        build_ui();
    restore_ui_state();
}

void SettingScene::on_exit()
{
}

void SettingScene::reset()
{
}

void SettingScene::build_ui()
{
    if (_main_setting_window && !_main_setting_window->is_destroyed())
        return;

    _main_setting_window = Scene::create_and_add_object<elysia::ui::UiWindow>(elysia::core::Rect{ 0,0,1280,720 },10);
    if (!_main_setting_window)
        return;

    auto page = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,700,680 });
    page->set_direction(elysia::ui::UiListDirection::Vertical);
    page->set_item_spacing(6.0f);
    page->set_padding(elysia::ui::UiLayoutPadding{ 40.0f,24.0f,40.0f,24.0f });

    auto title = make_label("Settings",field_width,elysia::ui::UiTypographyRole::Title);
    title->set_horizontal_align(elysia::ui::TextHorizontalAlign::Center);
    title->set_size(elysia::core::Vector2(field_width,64.0f));
    page->add_back(std::move(title));

    page->add_back(make_label("Display",field_width,elysia::ui::UiTypographyRole::Subtitle));

    auto resolution = std::make_unique<elysia::ui::UiDropdown>(
        elysia::core::Rect{ 0,0,control_width,row_height });
    resolution->set_options({
        { elysia::ui::ui_raw_text("1280 x 720") },
        { elysia::ui::ui_raw_text("1600 x 900") },
        { elysia::ui::ui_raw_text("1920 x 1080") }
    });
    resolution->register_with_window(*_main_setting_window);
    page->add_back(make_field_row("Resolution",std::move(resolution)));

    auto fullscreen = std::make_unique<elysia::ui::UiCheckbox>(
        elysia::core::Rect{ 0,0,row_height,row_height });
    fullscreen->set_checked(elysia::config::UserConfigService::instance()->user_config().fullscreen());
    page->add_back(make_field_row("Fullscreen",std::move(fullscreen)));

    page->add_back(make_label("Audio",field_width,elysia::ui::UiTypographyRole::Subtitle));
    const auto& settings = elysia::config::UserConfigService::instance()->user_config();
    page->add_back(make_field_row("Master volume",make_volume_slider(static_cast<float>(settings.master_volume()))));
    page->add_back(make_field_row("Music volume",make_volume_slider(static_cast<float>(settings.music_volume()))));
    page->add_back(make_field_row("Sound volume",make_volume_slider(static_cast<float>(settings.sound_volume()))));

    page->add_back(make_label("General",field_width,elysia::ui::UiTypographyRole::Subtitle));
    auto language = std::make_unique<elysia::ui::UiDropdown>(
        elysia::core::Rect{ 0,0,control_width,row_height });
    language->set_options({
        { elysia::ui::ui_raw_text("English") },
        { elysia::ui::ui_raw_text("简体中文") }
    });
    language->register_with_window(*_main_setting_window);
    page->add_back(make_field_row("Language",std::move(language)));

    auto actions = std::make_unique<elysia::ui::UiListContainer>(
        elysia::core::Rect{ 0,0,field_width,52 });
    actions->set_direction(elysia::ui::UiListDirection::Horizontal);
    actions->set_item_spacing(16.0f);

    auto save = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,160,48 });
    save->set_text_content(elysia::ui::ui_raw_text("Save"));
    save->set_visual_role(elysia::ui::UiButtonVisualRole::Primary);
    save->set_on_click([]()
    {
        (void)elysia::config::UserConfigService::instance()->save_user_config();
    });
    actions->add_back(std::move(save));

    auto back = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,160,48 });
    back->set_text_content(elysia::ui::ui_raw_text("Back"));
    back->set_on_click([this]() { Scene::request_scene_switch(AppSceneKeys::MainMenu); });
    actions->add_back(std::move(back));
    page->add_back(std::move(actions));

    elysia::ui::UiElement* added = _main_setting_window->add_child(
        std::move(page),{ elysia::ui::UiLayoutAnchor::Center });
    if (auto* scope = dynamic_cast<elysia::ui::UiListContainer*>(added))
        _main_setting_window->register_focus_scope(*scope);

    _main_setting_window->set_on_cancel([this]()
    {
            Scene::request_scene_switch(AppSceneKeys::MainMenu, MainMeunEnterPayload{ .replay_theme_music = false });
    });
}

void SettingScene::restore_ui_state()
{
    if (!_main_setting_window || _main_setting_window->is_destroyed())
        return;
    _main_setting_window->set_visible(true);
    _main_setting_window->set_active(true);
    _main_setting_window->focus_first_available_scope();
}
}
