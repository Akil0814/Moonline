#define SDL_MAIN_HANDLED

#include "engine/ui/composites/ui_labeled_radio_button.h"
#include "engine/ui/composites/ui_tab_bar.h"
#include "engine/ui/containers/ui_button_group.h"
#include "engine/ui/containers/ui_radio_group.h"
#include "engine/ui/presets/settings_panel.h"
#include "engine/ui/widgets/ui_button.h"
#include "engine/ui/widgets/ui_radio_button.h"
#include "tests/support/test_assertions.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace
{
using moonline::tests::require;

void test_group_repairs_selection_without_group_callback()
{
    elysia::ui::UiButtonGroup group(elysia::core::Rect{ 0,0,240,40 });
    group.add_button(std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,100,40 }));
    group.add_button(std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,100,40 }));
    require(group.selected_index() == 0,"button group should auto-select first");
    group.child_at(0)->destroy();

    std::vector<elysia::core::UiRenderCommand> commands;
    group.submit_ui_render_commands(commands);
    group.update(0.0);
    require(group.selected_index() == 0,"selection should repair after the selected child is removed");
}

void test_radio_render_defers_callback()
{
    elysia::ui::UiRadioGroup group(elysia::core::Rect{ 0,0,240,40 });
    group.add_back(std::make_unique<elysia::ui::UiLabeledRadioButton>(elysia::core::Rect{ 0,0,100,40 }));
    group.add_back(std::make_unique<elysia::ui::UiRadioButton>(elysia::core::Rect{ 0,0,100,40 }));
    group.update(0.0);
    int callback_count = 0;
    group.set_on_selection_changed([&](std::optional<std::size_t>) { ++callback_count; });
    group.child_at(0)->destroy();

    std::vector<elysia::core::UiRenderCommand> commands;
    group.submit_ui_render_commands(commands);
    require(callback_count == 0,"render must not notify radio group callback");
    group.update(0.0);
    require(callback_count == 1,"next update should deliver deferred radio callback once");
}

void test_group_preserves_button_override()
{
    elysia::ui::UiButtonStyleOverrides custom{};
    custom.chrome.draw_background = false;
    custom.chrome.draw_border = false;
    auto button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,100,40 });
    button->set_style_overrides(custom);

    elysia::ui::UiButtonGroup group(elysia::core::Rect{ 0,0,120,40 });
    elysia::ui::UiButton* raw = group.add_button(std::move(button));
    require(raw && raw->has_style_overrides(),"group must retain explicit button style overrides");
    require(!raw->style().chrome.draw_background && !raw->style().chrome.draw_border,
        "selection role must not overwrite structural button style");

    elysia::ui::UiTabBar tabs(elysia::core::Rect{ 0,0,240,40 });
    elysia::ui::UiButton* tab = tabs.add_tab(elysia::ui::ui_raw_text("tab"));
    require(tab != nullptr,"tab should be created");
    tab->set_style_overrides(custom);
    tabs.set_selected_index(0);
    require(!tab->style().chrome.draw_background && !tab->style().chrome.draw_border,
        "tab selection must retain explicit button style override");
}

void test_button_group_preserves_button_callback_after_selection()
{
    using namespace elysia;
    ui::UiButtonGroup group(core::Rect{ 0,0,240,40 });
    auto first = std::make_unique<ui::UiButton>(core::Rect{ 0,0,100,40 });
    auto second = std::make_unique<ui::UiButton>(core::Rect{ 0,0,100,40 });
    int callback_count = 0;
    second->set_on_click([&]()
    {
        ++callback_count;
        require(group.selected_index() == 1,"button callback must observe the new group selection");
    });
    group.add_button(std::move(first));
    ui::UiButton* second_raw = group.add_button(std::move(second));
    require(second_raw != nullptr,"group should adopt second button");
    second_raw->set_focused(true);
    (void)second_raw->on_ui_input_event({ .action=ui::UiAction::Confirm,.type=ui::UiInputEventType::ActionPressed });
    (void)second_raw->on_ui_input_event({ .action=ui::UiAction::Confirm,.type=ui::UiInputEventType::ActionReleased });
    require(callback_count == 1,"group decoration must preserve the original button callback");
}

void test_settings_panel_keeps_draft_local_and_normalizes_options()
{
    using namespace elysia;
    ui::SettingsPanel panel(core::Rect{ 0,0,700,680 });
    const ui::SettingsPanelDraft draft{
        .resolution = { 1366,768 },
        .fullscreen = true,
        .master_volume = 80,
        .music_volume = 70,
        .sound_volume = 60,
        .language = "en"
    };
    panel.set_draft(draft);
    panel.set_options({
        .resolutions = {
            { 1920,1080 },
            { 1280,720 },
            { 1920,1080 },
            { 0,0 }
        },
        .languages = { "en","zh_cn","en","" }
    });

    require(panel.draft() == draft,
        "settings panel option refresh must not apply or replace its local draft");
    require(panel.options().resolutions.size() == 3
        && panel.options().resolutions[0] == ui::SettingsResolution{ 1280,720 }
        && panel.options().resolutions[1] == ui::SettingsResolution{ 1366,768 }
        && panel.options().resolutions[2] == ui::SettingsResolution{ 1920,1080 },
        "settings panel must normalize resolutions and retain the active resolution");
    require(panel.options().languages == std::vector<std::string>{ "en","zh_cn" },
        "settings panel must deduplicate language identifiers from LocalizationManager");

    int save_count = 0;
    int back_count = 0;
    ui::SettingsPanelDraft saved_draft;
    panel.set_on_save([&](const ui::SettingsPanelDraft& value)
    {
        ++save_count;
        saved_draft = value;
    });
    panel.set_on_back([&]() { ++back_count; });
    panel.set_scope_focused(true);
    require(panel.focus_first_available(),
        "settings panel must expose its first field to keyboard focus");

    const auto action = [&panel](ui::UiAction value,ui::UiInputEventType type)
    {
        return panel.on_ui_input_event(ui::UiInputEvent{
            .action = value,
            .type = type,
            .device = input::InputDevice::Keyboard
        });
    };
    (void)action(ui::UiAction::NavigateDown,ui::UiInputEventType::ActionPressed);
    (void)action(ui::UiAction::Confirm,ui::UiInputEventType::ActionPressed);
    (void)action(ui::UiAction::Confirm,ui::UiInputEventType::ActionReleased);
    require(!panel.draft().fullscreen,
        "editing a settings control must update only the panel draft");

    for (int index = 0; index < 5; ++index)
        (void)action(ui::UiAction::NavigateDown,ui::UiInputEventType::ActionPressed);
    (void)action(ui::UiAction::Confirm,ui::UiInputEventType::ActionPressed);
    (void)action(ui::UiAction::Confirm,ui::UiInputEventType::ActionReleased);
    require(save_count == 1 && saved_draft == panel.draft(),
        "the Save button must emit the complete local draft exactly once");

    (void)action(ui::UiAction::NavigateRight,ui::UiInputEventType::ActionPressed);
    (void)action(ui::UiAction::Confirm,ui::UiInputEventType::ActionPressed);
    (void)action(ui::UiAction::Confirm,ui::UiInputEventType::ActionReleased);
    require(back_count == 1,
        "the Back button must invoke its navigation callback exactly once");
}
}

int main()
{
    test_group_repairs_selection_without_group_callback();
    test_radio_render_defers_callback();
    test_group_preserves_button_override();
    test_button_group_preserves_button_callback_after_selection();
    test_settings_panel_keeps_draft_local_and_normalizes_options();
    std::cout << "ui selection control tests passed\n";
    return EXIT_SUCCESS;
}

