#define SDL_MAIN_HANDLED

#include "../engine/ui/composites/ui_confirmation_dialog.h"
#include "../engine/ui/containers/ui_button_group.h"
#include "../engine/ui/containers/ui_chrome_container.h"
#include "../engine/ui/containers/ui_list_container.h"
#include "../engine/ui/containers/ui_panel.h"
#include "../engine/ui/containers/ui_radio_group.h"
#include "../engine/ui/containers/ui_scroll_container.h"
#include "../engine/ui/composites/ui_tab_bar.h"
#include "../engine/ui/widgets/ui_button.h"
#include "../engine/ui/composites/ui_dropdown.h"
#include "../engine/ui/composites/ui_labeled_radio_button.h"
#include "../engine/ui/style/ui_theme.h"
#include "../engine/ui/style/ui_theme_manager.h"
#include "../engine/ui/widgets/ui_checkbox.h"
#include "../engine/ui/widgets/ui_drag_handle.h"
#include "../engine/ui/widgets/ui_radio_button.h"
#include "../engine/ui/widgets/ui_slider.h"
#include "../engine/ui/widgets/ui_text_input.h"
#include "../engine/ui/composites/ui_tooltip.h"
#include "../engine/ui/window/ui_window.h"

#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

namespace
{
void require(bool condition,const char* message)
{
    if (condition)
        return;
    std::cerr << "FAILED: " << message << '\n';
    std::exit(EXIT_FAILURE);
}

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

elysia::ui::UiInteractiveColors test_border_colors()
{
    return {
        elysia::core::Color{ 10,20,30,255 },
        elysia::core::Color{ 40,50,60,255 },
        elysia::core::Color{ 70,80,90,255 },
        elysia::core::Color{ 100,110,120,255 }
    };
}

void test_corner_radius_normalization()
{
    using namespace elysia::core;
    const Rect rect{ 0,0,100,40 };
    require(normalize_ui_corner_radius(rect,-4.0f) == 0.0f,"negative radius should normalize to zero");
    require(normalize_ui_corner_radius(rect,0.0f) == 0.0f,"zero radius should stay zero");
    require(normalize_ui_corner_radius(rect,std::numeric_limits<float>::infinity()) == 0.0f,
        "non-finite radius should normalize to zero");
    require(normalize_ui_corner_radius(Rect::zero(),8.0f) == 0.0f,"empty rect radius should normalize to zero");
    require(normalize_ui_corner_radius(rect,80.0f) == 20.0f,"radius should clamp to half the short side");

    const UiRenderCommand square = make_ui_fill_rect_command(rect,Color{},0.0f);
    require(square.type == UiRenderCommandType::FillRect,"zero radius should preserve the fill-rect fast path");
    const UiRenderCommand rounded = make_ui_draw_rect_command(rect,Color{},80.0f);
    require(rounded.type == UiRenderCommandType::DrawRoundedRect,"positive radius should create rounded border command");
    require(rounded.corner_radius == 20.0f,"rounded command should store the normalized radius");
}

void test_chrome_uses_single_rounded_outer_frame()
{
    elysia::ui::UiChromeContainer chrome(elysia::core::Rect{ 0,0,240,160 });
    chrome.set_header_height(48.0f);
    auto style = chrome.style();
    style.corner_radius = 12.0f;
    chrome.set_style(style);

    std::vector<elysia::core::UiRenderCommand> commands;
    chrome.submit_ui_render_commands(commands);
    std::size_t rounded_fills = 0;
    std::size_t rounded_borders = 0;
    for (const auto& command : commands)
    {
        rounded_fills += command.type == elysia::core::UiRenderCommandType::FillRoundedRect ? 1u : 0u;
        rounded_borders += command.type == elysia::core::UiRenderCommandType::DrawRoundedRect ? 1u : 0u;
    }
    require(rounded_fills == 2,"chrome should emit one outer fill and one clipped rounded header cap");
    require(rounded_borders == 1,"chrome should emit exactly one rounded outer border");
}

void test_overlay_lifetime()
{
    elysia::ui::UiConfirmationDialog dialog(elysia::core::Rect{ 0,0,320,180 });
    {
        elysia::ui::UiWindow window(elysia::core::Rect{ 0,0,640,480 });
        dialog.register_as_overlay(window);
        dialog.open();
        require(window.is_overlay_open(dialog),"external dialog should open");
    }
    dialog.open(); // Window destruction must have cleared the borrowed pointer.

    elysia::ui::UiWindow first_window(elysia::core::Rect{ 0,0,640,480 });
    elysia::ui::UiWindow second_window(elysia::core::Rect{ 0,0,640,480 });
    dialog.register_as_overlay(first_window);
    dialog.register_as_overlay(second_window);
    require(!first_window.is_overlay_open(dialog),"moving an overlay must unregister the old window");
    dialog.unregister_as_overlay();
    dialog.open();

    {
        auto short_lived = std::make_unique<elysia::ui::UiConfirmationDialog>(
            elysia::core::Rect{ 0,0,320,180 });
        short_lived->register_as_overlay(first_window);
    }
    first_window.update(0.0);

    elysia::ui::UiWindow window(elysia::core::Rect{ 0,0,640,480 });
    auto* owned = window.create_child<elysia::ui::UiConfirmationDialog>(
        elysia::core::Rect{ 0,0,320,180 });
    require(owned != nullptr,"owned dialog should be created");
    owned->register_as_overlay(window);
    owned->destroy();
    window.update(0.0);
}

void test_transient_popup_lifetime()
{
    elysia::ui::UiDropdown dropdown(elysia::core::Rect{ 0,0,200,40 });
    dropdown.set_options({ { elysia::ui::ui_raw_text("one") } });
    {
        elysia::ui::UiWindow window(elysia::core::Rect{ 0,0,640,480 });
        dropdown.register_as_transient_popup(window);
        dropdown.open();
        require(dropdown.is_expanded(),"registered dropdown should open");
    }
    require(!dropdown.is_expanded(),"window detach should close dropdown");
    dropdown.open();
    require(!dropdown.is_expanded(),"detached dropdown must not reopen");

    elysia::ui::UiWindow first_window(elysia::core::Rect{ 0,0,640,480 });
    elysia::ui::UiWindow second_window(elysia::core::Rect{ 0,0,640,480 });
    dropdown.register_as_transient_popup(first_window);
    dropdown.register_as_transient_popup(first_window);
    dropdown.register_as_transient_popup(second_window);
    dropdown.unregister_as_transient_popup();
    dropdown.open();
    require(!dropdown.is_expanded(),"explicitly unregistered dropdown must remain closed");

    {
        auto short_lived = std::make_unique<elysia::ui::UiDropdown>(
            elysia::core::Rect{ 0,0,200,40 });
        short_lived->set_options({ { elysia::ui::ui_raw_text("one") } });
        short_lived->register_as_transient_popup(first_window);
    }
    first_window.update(0.0);

    auto owned = std::make_unique<elysia::ui::UiDropdown>(elysia::core::Rect{ 0,0,200,40 });
    auto* owned_raw = owned.get();
    owned_raw->set_options({ { elysia::ui::ui_raw_text("one") } });
    owned_raw->register_as_transient_popup(first_window);
    first_window.add_child(std::move(owned));
    owned_raw->destroy();
    first_window.update(0.0);
}

void test_tooltip_lifetime()
{
    elysia::ui::UiTooltip tooltip;
    {
        elysia::ui::UiWindow window(elysia::core::Rect{ 0,0,640,480 });
        tooltip.register_with_window(window);
    }
    tooltip.show();
    require(!tooltip.is_open(),"tooltip without content remains closed after window detach");

    elysia::ui::UiWindow window(elysia::core::Rect{ 0,0,640,480 });
    {
        auto short_lived = std::make_unique<elysia::ui::UiTooltip>();
        short_lived->register_with_window(window);
    }
    window.update(0.0);
}

void test_group_render_defers_callback()
{
    elysia::ui::UiButtonGroup group(elysia::core::Rect{ 0,0,240,40 });
    group.add_button(std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,100,40 }));
    group.add_button(std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,100,40 }));
    int callback_count = 0;
    group.set_on_selection_changed([&](std::optional<std::size_t>) { ++callback_count; });
    require(group.selected_index() == 0,"button group should auto-select first");
    group.child_at(0)->destroy();

    std::vector<elysia::core::UiRenderCommand> commands;
    group.submit_ui_render_commands(commands);
    require(callback_count == 0,"render must not notify button group callback");
    group.update(0.0);
    require(callback_count == 1,"next update should deliver deferred button group callback once");
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
    elysia::ui::UiButtonStyle custom{};
    custom.chrome.draw_background = false;
    custom.chrome.draw_border = false;
    auto button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,100,40 });
    button->set_style(custom);

    elysia::ui::UiButtonGroup group(elysia::core::Rect{ 0,0,120,40 });
    elysia::ui::UiButton* raw = group.add_button(std::move(button));
    require(raw && raw->has_style_override(),"group must retain explicit button style override");
    require(!raw->style().chrome.draw_background && !raw->style().chrome.draw_border,
        "selection role must not overwrite structural button style");

    elysia::ui::UiTabBar tabs(elysia::core::Rect{ 0,0,240,40 });
    elysia::ui::UiButton* tab = tabs.add_tab(elysia::ui::ui_raw_text("tab"));
    require(tab != nullptr,"tab should be created");
    tab->set_style(custom);
    tabs.set_selected_index(0);
    require(!tab->style().chrome.draw_background && !tab->style().chrome.draw_border,
        "tab selection must retain explicit button style override");
}

void test_empty_focus_scopes()
{
    elysia::ui::UiListContainer list;
    elysia::ui::UiPanel panel;
    require(!list.focus_first_available(),"empty list should not acquire focus");
    require(!panel.focus_first_available(),"empty panel should not acquire focus");
}

void test_nested_focus_and_dropdown_navigation()
{
    auto inner = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,200,40 });
    inner->add_back(std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,100,40 }));
    elysia::ui::UiListContainer outer(elysia::core::Rect{ 0,0,240,80 });
    outer.add_back(std::move(inner));
    outer.set_scope_focused(true);
    require(outer.focus_first_available(),"nested list should find delegated button focus");
    require(outer.focused_target() != nullptr,"nested list should expose cached focused target");

    auto scroll_content = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,200,80 });
    scroll_content->add_back(std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,100,40 }));
    elysia::ui::UiScrollContainer scroll(elysia::core::Rect{ 0,0,200,40 });
    scroll.set_content(std::move(scroll_content));
    scroll.set_scope_focused(true);
    require(scroll.focus_first_available(),"scroll container should delegate focus to content");

    elysia::ui::UiWindow window(elysia::core::Rect{ 0,0,640,480 });
    auto dropdown = std::make_unique<elysia::ui::UiDropdown>(
        elysia::core::Rect{ 20,20,200,40 });
    auto* raw_dropdown = dropdown.get();
    raw_dropdown->set_options({
        { elysia::ui::ui_raw_text("one") },
        { elysia::ui::ui_raw_text("two") }
    });
    raw_dropdown->register_as_transient_popup(window);
    window.add_child(std::move(dropdown));
    raw_dropdown->open();
    require(raw_dropdown->is_expanded(),"owned dropdown should open");
    window.on_ui_input_event(elysia::ui::UiInputEvent{
        .action = elysia::ui::UiAction::NavigateDown,
        .type = elysia::ui::UiInputEventType::ActionPressed
    });
    window.on_ui_input_event(elysia::ui::UiInputEvent{
        .action = elysia::ui::UiAction::Confirm,
        .type = elysia::ui::UiInputEventType::ActionPressed
    });
    window.on_ui_input_event(elysia::ui::UiInputEvent{
        .action = elysia::ui::UiAction::Confirm,
        .type = elysia::ui::UiInputEventType::ActionReleased
    });
    require(raw_dropdown->selected_index() == 1,"dropdown navigation should select focused option");
    require(!raw_dropdown->is_expanded(),"dropdown should close after confirmation");
}

void require_command_color(
    const std::vector<elysia::core::UiRenderCommand>& commands,
    elysia::core::UiRenderCommandType type,
    elysia::core::Color expected,
    const char* message)
{
    const auto* command = find_command(commands,type);
    require(command != nullptr,message);
    require(command->color == expected,message);
}

void test_button_interactive_border_colors()
{
    elysia::ui::UiButtonStyle style{};
    style.chrome.draw_background = false;
    style.chrome.draw_border = true;
    style.chrome.border = test_border_colors();
    elysia::ui::UiButton button(elysia::core::Rect{ 0,0,100,40 });
    button.set_style(style);

    const auto render = [&button]()
    {
        std::vector<elysia::core::UiRenderCommand> commands;
        button.submit_ui_render_commands(commands);
        return commands;
    };

    require_command_color(render(),elysia::core::UiRenderCommandType::DrawRect,
        style.chrome.border.idle,"button idle border color");
    button.set_focused(true);
    require_command_color(render(),elysia::core::UiRenderCommandType::DrawRect,
        style.chrome.border.focused,"button focused border color");
    button.on_ui_input_event(elysia::ui::UiInputEvent{
        .action = elysia::ui::UiAction::Confirm,
        .type = elysia::ui::UiInputEventType::ActionPressed
    });
    require_command_color(render(),elysia::core::UiRenderCommandType::DrawRect,
        style.chrome.border.active,"button active border color");
    button.set_enabled(false);
    require_command_color(render(),elysia::core::UiRenderCommandType::DrawRect,
        style.chrome.border.disabled,"button disabled border color");
}

void test_textured_button_border()
{
    elysia::ui::UiButtonStyle style{};
    style.chrome.draw_border = true;
    style.chrome.border = test_border_colors();
    elysia::ui::UiButton button(elysia::core::Rect{ 0,0,100,40 });
    button.set_style(style);
    auto* texture = reinterpret_cast<SDL_Texture*>(static_cast<std::uintptr_t>(1));
    button.set_state_textures(elysia::ui::UiButtonTextures{ .idle = texture });

    std::vector<elysia::core::UiRenderCommand> commands;
    button.submit_ui_render_commands(commands);
    require(commands.size() == 2,"textured button should emit texture and border");
    require(commands[0].type == elysia::core::UiRenderCommandType::Texture,
        "textured button texture should render first");
    require(commands[1].type == elysia::core::UiRenderCommandType::DrawRect,
        "textured button border should render after texture");

    style.chrome.draw_border = false;
    button.set_style(style);
    commands.clear();
    button.submit_ui_render_commands(commands);
    require(commands.size() == 1 && commands[0].type == elysia::core::UiRenderCommandType::Texture,
        "borderless textured button should emit only texture");
}

void test_other_chrome_active_borders()
{
    const auto border = test_border_colors();
    const elysia::ui::UiInputEvent confirm_pressed{
        .action = elysia::ui::UiAction::Confirm,
        .type = elysia::ui::UiInputEventType::ActionPressed
    };
    std::vector<elysia::core::UiRenderCommand> commands;

    elysia::ui::UiCheckboxStyle checkbox_style{};
    checkbox_style.chrome.border = border;
    elysia::ui::UiCheckbox checkbox(elysia::core::Rect{ 0,0,40,40 });
    checkbox.set_style(checkbox_style);
    checkbox.set_focused(true);
    checkbox.on_ui_input_event(confirm_pressed);
    checkbox.submit_ui_render_commands(commands);
    require_command_color(commands,elysia::core::UiRenderCommandType::DrawRect,
        border.active,"checkbox active border color");

    elysia::ui::UiRadioButtonStyle radio_style{};
    radio_style.chrome.border = border;
    elysia::ui::UiRadioButton radio(elysia::core::Rect{ 0,0,40,40 });
    radio.set_style(radio_style);
    radio.set_focused(true);
    radio.on_ui_input_event(confirm_pressed);
    commands.clear();
    radio.submit_ui_render_commands(commands);
    require_command_color(commands,elysia::core::UiRenderCommandType::DrawCircle,
        border.active,"radio active border color");

    elysia::ui::UiTextInputStyle input_style{};
    input_style.chrome.border = border;
    elysia::ui::UiTextInput input(elysia::core::Rect{ 0,0,160,40 });
    input.set_style(input_style);
    input.set_focused(true);
    input.on_ui_input_event(confirm_pressed);
    commands.clear();
    input.submit_ui_render_commands(commands);
    require_command_color(commands,elysia::core::UiRenderCommandType::DrawRect,
        border.active,"text input active border color");

    const elysia::ui::UiInputEvent pointer_pressed{
        .type = elysia::ui::UiInputEventType::PointerPressed,
        .device = elysia::input::InputDevice::Mouse,
        .control = elysia::input::RawInputControl::MouseLeft,
        .mouse_x = 9,
        .mouse_y = 9
    };
    elysia::ui::UiDragHandleStyle handle_style{};
    handle_style.chrome.border = border;
    elysia::ui::UiDragHandle handle(elysia::core::Rect{ 0,0,40,40 });
    handle.set_style(handle_style);
    handle.on_ui_input_event(pointer_pressed);
    commands.clear();
    handle.submit_ui_render_commands(commands);
    require_command_color(commands,elysia::core::UiRenderCommandType::DrawRect,
        border.active,"drag handle active border color");

    elysia::ui::UiSliderStyle slider_style{};
    slider_style.chrome.border = border;
    elysia::ui::UiSlider slider(elysia::core::Rect{ 0,0,180,40 });
    slider.set_style(slider_style);
    slider.on_ui_input_event(elysia::ui::UiInputEvent{
        .type = elysia::ui::UiInputEventType::PointerPressed,
        .device = elysia::input::InputDevice::Mouse,
        .control = elysia::input::RawInputControl::MouseLeft,
        .mouse_x = 9,
        .mouse_y = 20
    });
    commands.clear();
    slider.submit_ui_render_commands(commands);
    require(!commands.empty() && commands.back().type == elysia::core::UiRenderCommandType::DrawRect,
        "slider should emit its outer border last");
    require(commands.back().color == border.active,"slider active border color");
}

void test_builtin_theme_border_states()
{
    constexpr elysia::ui::UiBuiltinTheme themes[]{
        elysia::ui::UiBuiltinTheme::BlueGlassMoon,
        elysia::ui::UiBuiltinTheme::ElysiaLight,
        elysia::ui::UiBuiltinTheme::ElysiaDark,
        elysia::ui::UiBuiltinTheme::EvangelionUnit00,
        elysia::ui::UiBuiltinTheme::EvangelionUnit01,
        elysia::ui::UiBuiltinTheme::EvangelionUnit02,
        elysia::ui::UiBuiltinTheme::QuietSlate
    };
    for (const auto theme_id : themes)
    {
        const auto theme = elysia::ui::make_builtin_theme(theme_id);
        const auto& default_border = theme.button(elysia::ui::UiButtonVisualRole::Default).chrome.border;
        const auto& primary_border = theme.button(elysia::ui::UiButtonVisualRole::Primary).chrome.border;
        require(default_border.idle != default_border.focused,
            "default theme border should distinguish idle and focused");
        require(default_border.focused != default_border.active,
            "default theme border should distinguish focused and active");
        require(primary_border.idle != primary_border.focused,
            "primary border should distinguish selected idle and focused");
    }
}

void test_container_driven_theme_tree()
{
    elysia::ui::UiThemeManager manager;
    auto window = std::make_unique<elysia::ui::UiWindow>(elysia::core::Rect{ 0,0,640,360 });
    auto button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,120,40 });
    elysia::ui::UiButton* raw = button.get();
    window->add_child(std::move(button));
    auto registration = manager.register_root(*window);

    manager.set_theme(elysia::ui::UiBuiltinTheme::ElysiaDark);
    require(raw->style().chrome.background.idle
            == manager.current_theme().button(elysia::ui::UiButtonVisualRole::Default).chrome.background.idle,
        "root registration should style existing atomic descendants");

    auto dynamic_button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,120,40 });
    elysia::ui::UiButton* dynamic_raw = dynamic_button.get();
    window->add_child(std::move(dynamic_button));
    require(dynamic_raw->style().chrome.background.idle
            == manager.current_theme().button(elysia::ui::UiButtonVisualRole::Default).chrome.background.idle,
        "new descendants should receive the current theme immediately");

    elysia::ui::UiButtonStyle custom = dynamic_raw->style();
    custom.chrome.draw_background = false;
    dynamic_raw->set_style(custom);
    manager.set_theme(elysia::ui::UiBuiltinTheme::ElysiaLight);
    require(!dynamic_raw->style().chrome.draw_background,"manual overrides must survive theme changes");
    dynamic_raw->clear_style_override();
    require(dynamic_raw->style().chrome.background.idle
            == manager.current_theme().button(elysia::ui::UiButtonVisualRole::Default).chrome.background.idle,
        "clearing an override should expose the latest base style");

    auto* tooltip = window->create_child<elysia::ui::UiTooltip>(0);
    auto tooltip_panel = std::make_unique<elysia::ui::UiPanel>(elysia::core::Rect{ 0,0,180,60 });
    elysia::ui::UiPanel* tooltip_panel_raw = tooltip_panel.get();
    tooltip_panel_raw->set_visual_role(elysia::ui::UiPanelVisualRole::Dialog);
    tooltip->set_content(std::move(tooltip_panel));
    tooltip->register_with_window(*window);
    require(tooltip_panel_raw->style().background
            == manager.current_theme().panel(elysia::ui::UiPanelVisualRole::Dialog).background,
        "tooltip content outside the ownership tree should receive the window theme");

    manager.set_theme(elysia::ui::UiBuiltinTheme::QuietSlate);
    require(tooltip_panel_raw->style().background
            == manager.current_theme().panel(elysia::ui::UiPanelVisualRole::Dialog).background,
        "tooltip content should follow later theme changes");

    window.reset();
    require(!registration.registered(),"destroying a registered root must invalidate its handle");
}
}

int main()
{
    test_corner_radius_normalization();
    test_chrome_uses_single_rounded_outer_frame();
    test_overlay_lifetime();
    test_transient_popup_lifetime();
    test_tooltip_lifetime();
    test_group_render_defers_callback();
    test_radio_render_defers_callback();
    test_group_preserves_button_override();
    test_empty_focus_scopes();
    test_nested_focus_and_dropdown_navigation();
    test_button_interactive_border_colors();
    test_textured_button_border();
    test_other_chrome_active_borders();
    test_builtin_theme_border_states();
    test_container_driven_theme_tree();
    std::cout << "ui lifecycle tests passed\n";
    return EXIT_SUCCESS;
}
