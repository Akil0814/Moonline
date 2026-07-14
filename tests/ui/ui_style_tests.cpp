#define SDL_MAIN_HANDLED

#include "engine/ui/composites/ui_labeled_checkbox.h"
#include "engine/ui/composites/ui_labeled_radio_button.h"
#include "engine/ui/composites/ui_tooltip.h"
#include "engine/ui/containers/ui_chrome_container.h"
#include "engine/ui/containers/ui_panel.h"
#include "engine/ui/style/ui_theme.h"
#include "engine/ui/style/ui_theme_manager.h"
#include "engine/ui/widgets/ui_button.h"
#include "engine/ui/widgets/ui_checkbox.h"
#include "engine/ui/widgets/ui_drag_handle.h"
#include "engine/ui/widgets/ui_radio_button.h"
#include "engine/ui/widgets/ui_slider.h"
#include "engine/ui/widgets/ui_text_input.h"
#include "engine/ui/window/ui_window.h"
#include "tests/support/test_assertions.h"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <type_traits>
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

elysia::ui::UiInteractiveColors test_border_colors()
{
    return {
        elysia::core::Color{ 10,20,30,255 },
        elysia::core::Color{ 40,50,60,255 },
        elysia::core::Color{ 70,80,90,255 },
        elysia::core::Color{ 100,110,120,255 }
    };
}

elysia::ui::UiInteractiveColorsOverrides test_border_color_overrides()
{
    const auto colors = test_border_colors();
    return { colors.idle,colors.focused,colors.active,colors.disabled };
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
    elysia::ui::UiChromeContainerStyleOverrides style{};
    style.corner_radius = 12.0f;
    chrome.set_style_overrides(style);

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

void test_field_level_style_cascade()
{
    elysia::ui::UiButton button(elysia::core::Rect{ 0,0,120,40 });
    elysia::ui::UiButtonStyle base = button.style();
    base.chrome.background.idle = elysia::core::Color{ 1,2,3,255 };
    base.chrome.border.idle = elysia::core::Color{ 4,5,6,255 };
    button.set_base_style(base);

    elysia::ui::UiButtonStyleOverrides overrides{};
    overrides.chrome.corner_radius = 9.0f;
    overrides.chrome.border.focused = elysia::core::Color{ 7,8,9,255 };
    button.set_style_overrides(overrides);

    elysia::ui::UiButtonStyle next_base = base;
    next_base.chrome.background.idle = elysia::core::Color{ 10,11,12,255 };
    next_base.chrome.border.idle = elysia::core::Color{ 13,14,15,255 };
    button.set_base_style(next_base);
    require(button.style().chrome.corner_radius == 9.0f,"written radius leaf should survive base changes");
    require(button.style().chrome.border.focused == elysia::core::Color{ 7,8,9,255 },
        "written nested border leaf should survive base changes");
    require(button.style().chrome.background.idle == next_base.chrome.background.idle,
        "unwritten background leaf should follow the latest base style");
    require(button.style().chrome.border.idle == next_base.chrome.border.idle,
        "sibling border leaf should remain independent");

    elysia::ui::UiButtonStyleOverrides replacement{};
    replacement.chrome.draw_border = false;
    button.set_style_overrides(replacement);
    require(button.style().chrome.corner_radius == next_base.chrome.corner_radius,
        "replacing overrides should restore omitted radius to base");
    require(button.style().chrome.border.focused == next_base.chrome.border.focused,
        "replacing overrides should restore omitted nested leaf to base");
    require(!button.style().chrome.draw_border,"replacement override should apply its written leaf");
    button.clear_style_overrides();
    require(!button.has_style_overrides(),"clearing overrides should leave an empty sparse override tree");

    static_assert(std::is_const_v<std::remove_reference_t<decltype(button.style())>>);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(button.style_overrides())>>);
}

void test_slider_adjustment_mode()
{
    const auto action = [](elysia::ui::UiAction action,elysia::ui::UiInputEventType type,elysia::input::InputDevice device)
    {
        return elysia::ui::UiInputEvent{ .action=action,.type=type,.device=device };
    };

    for (const auto device : { elysia::input::InputDevice::Keyboard,elysia::input::InputDevice::Gamepad })
    {
        elysia::ui::UiSlider horizontal(elysia::core::Rect{ 0,0,180,40 });
        horizontal.set_range(0.0f,10.0f);
        horizontal.set_step(1.0f);
        horizontal.set_value(5.0f);
        horizontal.set_focused(true);
        require(!horizontal.is_adjusting(),"focused slider should begin in navigation mode");
        require(!horizontal.on_ui_input_event(action(elysia::ui::UiAction::NavigateRight,elysia::ui::UiInputEventType::ActionPressed,device)),
            "focused horizontal slider must yield navigation before confirmation");
        require(horizontal.value() == 5.0f,"navigation mode must not change slider value");

        require(horizontal.on_ui_input_event(action(elysia::ui::UiAction::Confirm,elysia::ui::UiInputEventType::ActionPressed,device)),
            "confirm press should enter slider adjustment mode");
        require(horizontal.is_adjusting(),"confirm should mark slider as adjusting");
        elysia::ui::UiSliderStyleOverrides slider_style{};
        slider_style.chrome.draw_background = false;
        slider_style.chrome.draw_border = true;
        slider_style.chrome.border = test_border_color_overrides();
        horizontal.set_style_overrides(slider_style);
        std::vector<elysia::core::UiRenderCommand> commands;
        horizontal.submit_ui_render_commands(commands);
        require(!commands.empty() && commands.back().type == elysia::core::UiRenderCommandType::DrawRect
                && commands.back().color == test_border_colors().active,
            "adjusting slider should render its active border color");
        require(horizontal.on_ui_input_event(action(elysia::ui::UiAction::NavigateRight,elysia::ui::UiInputEventType::ActionPressed,device)),
            "adjusting horizontal slider should consume its primary axis");
        require(horizontal.value() == 6.0f,"adjusting primary axis should change value");
        require(horizontal.on_ui_input_event(action(elysia::ui::UiAction::End,elysia::ui::UiInputEventType::ActionPressed,device))
                && horizontal.value() == 10.0f,
            "adjusting slider should consume End and reach its maximum");
        require(horizontal.on_ui_input_event(action(elysia::ui::UiAction::NavigateRight,elysia::ui::UiInputEventType::ActionPressed,device))
                && horizontal.value() == 10.0f,
            "adjusting slider must retain its primary axis at the maximum value");
        require(horizontal.on_ui_input_event(action(elysia::ui::UiAction::Home,elysia::ui::UiInputEventType::ActionPressed,device))
                && horizontal.value() == 0.0f,
            "adjusting slider should consume Home and reach its minimum");
        require(!horizontal.on_ui_input_event(action(elysia::ui::UiAction::NavigateDown,elysia::ui::UiInputEventType::ActionPressed,device)),
            "adjusting slider must yield its secondary axis to the parent scope");
        require(horizontal.on_ui_input_event(action(elysia::ui::UiAction::Confirm,elysia::ui::UiInputEventType::ActionReleased,device)),
            "confirm release should be consumed without changing adjustment state");
        require(horizontal.is_adjusting(),"confirm release must not toggle adjustment mode");
        horizontal.on_ui_input_event(action(elysia::ui::UiAction::Confirm,elysia::ui::UiInputEventType::ActionPressed,device));
        require(!horizontal.is_adjusting(),"second confirm press should leave adjustment mode");
        require(!horizontal.on_ui_input_event(action(elysia::ui::UiAction::NavigateRight,elysia::ui::UiInputEventType::ActionPressed,device)),
            "after leaving adjustment mode the primary axis must return to navigation");
        horizontal.set_focused(false);
        require(!horizontal.is_adjusting(),"focus loss must clear adjustment mode");

        elysia::ui::UiSlider vertical(elysia::core::Rect{ 0,0,40,180 });
        vertical.set_orientation(elysia::ui::UiSliderOrientation::Vertical);
        vertical.set_range(0.0f,10.0f);
        vertical.set_step(1.0f);
        vertical.set_value(5.0f);
        vertical.set_focused(true);
        vertical.on_ui_input_event(action(elysia::ui::UiAction::Confirm,elysia::ui::UiInputEventType::ActionPressed,device));
        require(vertical.on_ui_input_event(action(elysia::ui::UiAction::NavigateUp,elysia::ui::UiInputEventType::ActionPressed,device)),
            "adjusting vertical slider should consume up");
        require(vertical.value() == 6.0f,"vertical primary axis should increase value");
        require(!vertical.on_ui_input_event(action(elysia::ui::UiAction::NavigateRight,elysia::ui::UiInputEventType::ActionPressed,device)),
            "vertical slider must yield horizontal navigation even while adjusting");
        vertical.set_enabled(false);
        require(!vertical.is_adjusting(),"disabling a slider must clear adjustment mode");
    }
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
    const auto border = test_border_colors();
    elysia::ui::UiButtonStyleOverrides style{};
    style.chrome.draw_background = false;
    style.chrome.draw_border = true;
    style.chrome.border = test_border_color_overrides();
    elysia::ui::UiButton button(elysia::core::Rect{ 0,0,100,40 });
    button.set_style_overrides(style);

    const auto render = [&button]()
    {
        std::vector<elysia::core::UiRenderCommand> commands;
        button.submit_ui_render_commands(commands);
        return commands;
    };

    require_command_color(render(),elysia::core::UiRenderCommandType::DrawRect,
        border.idle,"button idle border color");
    button.set_focused(true);
    require_command_color(render(),elysia::core::UiRenderCommandType::DrawRect,
        border.focused,"button focused border color");
    button.on_ui_input_event(elysia::ui::UiInputEvent{
        .action = elysia::ui::UiAction::Confirm,
        .type = elysia::ui::UiInputEventType::ActionPressed
    });
    require_command_color(render(),elysia::core::UiRenderCommandType::DrawRect,
        border.active,"button active border color");
    button.set_enabled(false);
    require_command_color(render(),elysia::core::UiRenderCommandType::DrawRect,
        border.disabled,"button disabled border color");
}

void test_textured_button_border()
{
    elysia::ui::UiButtonStyleOverrides style{};
    style.chrome.draw_border = true;
    style.chrome.border = test_border_color_overrides();
    elysia::ui::UiButton button(elysia::core::Rect{ 0,0,100,40 });
    button.set_style_overrides(style);
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
    button.set_style_overrides(style);
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

    elysia::ui::UiCheckboxStyleOverrides checkbox_style{};
    checkbox_style.chrome.border = test_border_color_overrides();
    elysia::ui::UiCheckbox checkbox(elysia::core::Rect{ 0,0,40,40 });
    checkbox.set_style_overrides(checkbox_style);
    checkbox.set_focused(true);
    checkbox.on_ui_input_event(confirm_pressed);
    checkbox.submit_ui_render_commands(commands);
    require_command_color(commands,elysia::core::UiRenderCommandType::DrawRect,
        border.active,"checkbox active border color");

    elysia::ui::UiRadioButtonStyleOverrides radio_style{};
    radio_style.chrome.border = test_border_color_overrides();
    elysia::ui::UiRadioButton radio(elysia::core::Rect{ 0,0,40,40 });
    radio.set_style_overrides(radio_style);
    radio.set_focused(true);
    radio.on_ui_input_event(confirm_pressed);
    commands.clear();
    radio.submit_ui_render_commands(commands);
    require_command_color(commands,elysia::core::UiRenderCommandType::DrawCircle,
        border.active,"radio active border color");

    elysia::ui::UiTextInputStyleOverrides input_style{};
    input_style.chrome.border = test_border_color_overrides();
    elysia::ui::UiTextInput input(elysia::core::Rect{ 0,0,160,40 });
    input.set_style_overrides(input_style);
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
    elysia::ui::UiDragHandleStyleOverrides handle_style{};
    handle_style.chrome.border = test_border_color_overrides();
    elysia::ui::UiDragHandle handle(elysia::core::Rect{ 0,0,40,40 });
    handle.set_style_overrides(handle_style);
    handle.on_ui_input_event(pointer_pressed);
    commands.clear();
    handle.submit_ui_render_commands(commands);
    require_command_color(commands,elysia::core::UiRenderCommandType::DrawRect,
        border.active,"drag handle active border color");

    elysia::ui::UiSliderStyleOverrides slider_style{};
    slider_style.chrome.border = test_border_color_overrides();
    elysia::ui::UiSlider slider(elysia::core::Rect{ 0,0,180,40 });
    slider.set_style_overrides(slider_style);
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

    elysia::ui::UiButtonStyleOverrides custom{};
    custom.chrome.draw_background = false;
    custom.chrome.corner_radius = 11.0f;
    dynamic_raw->set_style_overrides(custom);
    manager.set_theme(elysia::ui::UiBuiltinTheme::ElysiaLight);
    require(!dynamic_raw->style().chrome.draw_background,"manual overrides must survive theme changes");
    require(dynamic_raw->style().chrome.corner_radius == 11.0f,"corner radius override must survive theme changes");
    require(dynamic_raw->style().chrome.background.idle
            == manager.current_theme().button(elysia::ui::UiButtonVisualRole::Default).chrome.background.idle,
        "unwritten color fields must follow theme changes");
    dynamic_raw->clear_style_overrides();
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

void test_labeled_control_text_follows_theme()
{
    elysia::ui::UiThemeManager manager;
    auto window = std::make_unique<elysia::ui::UiWindow>(elysia::core::Rect{ 0,0,640,360 });
    auto checkbox = std::make_unique<elysia::ui::UiLabeledCheckbox>(elysia::core::Rect{ 0,0,180,40 });
    auto radio = std::make_unique<elysia::ui::UiLabeledRadioButton>(elysia::core::Rect{ 0,50,180,40 });
    auto* checkbox_raw = checkbox.get();
    auto* radio_raw = radio.get();
    window->add_child(std::move(checkbox));
    window->add_child(std::move(radio));
    auto registration = manager.register_root(*window);

    manager.set_theme(elysia::ui::UiBuiltinTheme::ElysiaLight);
    require(checkbox_raw->resolved_text_color()
            == manager.current_theme().label(elysia::ui::UiLabelVisualRole::Default).text,
        "labeled checkbox enabled text should follow the current theme");
    require(radio_raw->resolved_text_color()
            == manager.current_theme().label(elysia::ui::UiLabelVisualRole::Default).text,
        "labeled radio enabled text should follow the current theme");

    manager.set_theme(elysia::ui::UiBuiltinTheme::EvangelionUnit01);
    require(checkbox_raw->resolved_text_color()
            == manager.current_theme().label(elysia::ui::UiLabelVisualRole::Default).text,
        "labeled checkbox text should update after theme switching");
    require(radio_raw->resolved_text_color()
            == manager.current_theme().label(elysia::ui::UiLabelVisualRole::Default).text,
        "labeled radio text should update after theme switching");

    checkbox_raw->set_enabled(false);
    radio_raw->set_enabled(false);
    const auto disabled = manager.current_theme().button(elysia::ui::UiButtonVisualRole::Default).text.disabled;
    require(checkbox_raw->resolved_text_color() == disabled,
        "disabled labeled checkbox text should use the current theme disabled color");
    require(radio_raw->resolved_text_color() == disabled,
        "disabled labeled radio text should use the current theme disabled color");
}

void test_labeled_controls_preserve_label_base_style()
{
    const elysia::core::Color label_background{ 17,31,47,255 };
    const elysia::ui::UiEnabledDisabledColors text_colors{
        elysia::core::Color{ 200,210,220,255 },elysia::core::Color{ 90,100,110,255 }
    };
    elysia::ui::UiLabelStyle label_style{};
    label_style.corner_radius = 6.0f;
    label_style.background = label_background;
    label_style.draw_background = true;

    elysia::ui::UiCheckboxStyle checkbox_style{};
    checkbox_style.chrome.draw_background = false;
    checkbox_style.chrome.draw_border = false;
    elysia::ui::UiLabeledCheckbox checkbox(elysia::core::Rect{ 0,0,180,40 });
    checkbox.set_base_styles(checkbox_style,label_style,text_colors);

    elysia::ui::UiRadioButtonStyle radio_style{};
    radio_style.chrome.draw_background = false;
    radio_style.chrome.draw_border = false;
    elysia::ui::UiLabeledRadioButton radio(elysia::core::Rect{ 0,50,180,40 });
    radio.set_base_styles(radio_style,label_style,text_colors);

    const auto has_label_background = [&label_background](const std::vector<elysia::core::UiRenderCommand>& commands)
    {
        for (const auto& command : commands)
        {
            if (command.type == elysia::core::UiRenderCommandType::FillRoundedRect
                && command.color == label_background
                && command.corner_radius == 6.0f)
                return true;
        }
        return false;
    };

    std::vector<elysia::core::UiRenderCommand> commands;
    checkbox.submit_ui_render_commands(commands);
    require(has_label_background(commands),"labeled checkbox must retain themed label background and corner radius");

    commands.clear();
    radio.submit_ui_render_commands(commands);
    require(has_label_background(commands),"labeled radio must retain themed label background and corner radius");
}
}

int main()
{
    test_corner_radius_normalization();
    test_chrome_uses_single_rounded_outer_frame();
    test_field_level_style_cascade();
    test_slider_adjustment_mode();
    test_button_interactive_border_colors();
    test_textured_button_border();
    test_other_chrome_active_borders();
    test_builtin_theme_border_states();
    test_container_driven_theme_tree();
    test_labeled_control_text_follows_theme();
    test_labeled_controls_preserve_label_base_style();
    std::cout << "ui style tests passed\n";
    return EXIT_SUCCESS;
}
