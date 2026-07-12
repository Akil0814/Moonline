#define SDL_MAIN_HANDLED

#include "../engine/ui/composites/ui_confirmation_dialog.h"
#include "../engine/ui/containers/ui_button_group.h"
#include "../engine/ui/containers/ui_chrome_container.h"
#include "../engine/ui/containers/ui_grid_container.h"
#include "../engine/ui/containers/ui_list_container.h"
#include "../engine/ui/containers/ui_panel.h"
#include "../engine/ui/containers/ui_radio_group.h"
#include "../engine/ui/containers/ui_scroll_container.h"
#include "../engine/ui/composites/ui_tab_bar.h"
#include "../engine/ui/composites/ui_tab_container.h"
#include "../engine/ui/widgets/ui_button.h"
#include "../engine/ui/composites/ui_dropdown.h"
#include "../engine/ui/composites/ui_labeled_checkbox.h"
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
#include "../engine/ui/core/ui_render_command_range_utils.h"
#include "../engine/ui/input/ui_gamepad_scroll_synthesizer.h"
#include "../engine/scene/scene.h"
#include "../engine/io/path/path_manager.h"
#include "../engine/tools/logger.h"
#include "../application/application_event_boundary.h"

#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
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

elysia::ui::UiInteractiveColorsOverrides test_border_color_overrides()
{
    const auto colors = test_border_colors();
    return { colors.idle,colors.focused,colors.active,colors.disabled };
}

class WidthAwareDesiredElement final : public elysia::ui::UiElement
{
public:
    explicit WidthAwareDesiredElement(const elysia::core::Rect& rect)
        : UiElement(rect) {}

    [[nodiscard]] elysia::core::Vector2 content_extent() const noexcept override
    {
        return { 400.0f,size().x * 0.5f };
    }
};

class AllocationSensitiveContent final : public elysia::ui::UiElement
{
public:
    explicit AllocationSensitiveContent(const elysia::core::Rect& rect)
        : UiElement(rect) {}

    [[nodiscard]] elysia::core::Vector2 content_extent() const noexcept override
    {
        return { size().x,std::max(400.0f,size().y + 36.0f) };
    }
};

void test_scroll_offset_does_not_remeasure_allocated_content_as_growth()
{
    using namespace elysia;
    ui::UiScrollContainer scroll(core::Rect{ 0,0,200,100 });
    scroll.set_scroll_axis(ui::UiScrollAxis::Vertical);
    scroll.set_content(std::make_unique<AllocationSensitiveContent>(core::Rect{ 0,0,200,0 }));

    const float initial_height = scroll.content_size().y;
    require(initial_height == 400.0f,"scroll container should measure the initial intrinsic content height");

    for (int offset = 20; offset <= 200; offset += 20)
    {
        scroll.set_scroll_offset_y(static_cast<float>(offset));
        scroll.update_layout_if_dirty();
    }

    require(scroll.content_size().y == initial_height,
        "scrolling must only reposition content, not repeatedly expand its measured height");
}

class TestScene final : public elysia::scene::Scene
{
public:
    void on_enter(const elysia::scene::ScenePayload&) override {}
    void on_exit() override {}
    void reset() override {}
};

class ClearingChild final : public elysia::ui::UiElement,
    public elysia::core::Updatable,
    public elysia::ui::UiInputFrameReceiver,
    public elysia::ui::UiInputEventReceiver
{
public:
    enum class Trigger { Update,Frame,Event };

    ClearingChild(elysia::ui::UiChildHost& host,Trigger trigger,int& calls)
        : _host(host),_trigger(trigger),_calls(calls) {}

    void update(double) override { if (_trigger == Trigger::Update) { ++_calls; _host.clear_children(); } }
    void on_ui_input_frame(const elysia::ui::UiInputFrame&) override
    { if (_trigger == Trigger::Frame) { ++_calls; _host.clear_children(); } }
    bool on_ui_input_event(const elysia::ui::UiInputEvent&) override
    {
        if (_trigger == Trigger::Event) { ++_calls; _host.clear_children(); }
        return false;
    }

private:
    elysia::ui::UiChildHost& _host;
    Trigger _trigger;
    int& _calls;
};

class CountingChild final : public elysia::ui::UiElement,
    public elysia::core::Updatable,
    public elysia::ui::UiInputFrameReceiver,
    public elysia::ui::UiInputEventReceiver
{
public:
    explicit CountingChild(int& calls) : _calls(calls) {}
    void update(double) override { ++_calls; }
    void on_ui_input_frame(const elysia::ui::UiInputFrame&) override { ++_calls; }
    bool on_ui_input_event(const elysia::ui::UiInputEvent&) override { ++_calls; return false; }

private:
    int& _calls;
};

void test_list_consumes_desired_extent_and_cross_alignment()
{
    using namespace elysia;
    ui::UiListContainer list(core::Rect{ 0,0,300,400 });
    list.set_padding(ui::UiLayoutPadding{ 10,10,10,10 });

    auto centered = std::make_unique<WidthAwareDesiredElement>(core::Rect{ 0,0,1,1 });
    WidthAwareDesiredElement* centered_raw = centered.get();
    list.add_back(std::move(centered));
    list.update_layout_if_dirty();
    require(centered_raw->screen_rect().width() == 280.0f,"list should constrain desired width to its content width");
    require(centered_raw->screen_rect().height() == 140.0f,"width-constrained desired height should be remeasured");
    require(centered_raw->screen_rect().x() == 10.0f,"oversized child should fill the constrained cross axis");

    list.set_size(core::Vector2{ 200,400 });
    list.update_layout_if_dirty();
    require(centered_raw->screen_rect().width() == 180.0f,"parent width changes should relayout desired width");
    require(centered_raw->screen_rect().height() == 90.0f,"parent width changes should remeasure desired height");

    auto narrow = std::make_unique<ui::UiButton>(core::Rect{ 0,0,60,30 });
    ui::UiButton* narrow_raw = narrow.get();
    list.add_back(std::move(narrow));
    list.update_layout_if_dirty();
    require(narrow_raw->screen_rect().x() == 70.0f,"default list cross alignment should remain centered");

    list.set_cross_align(ui::UiLayoutAlign::Start);
    list.update_layout_if_dirty();
    require(narrow_raw->screen_rect().x() == 10.0f,"start cross alignment should left-align narrow children");

    ui::UiLayoutChildOptions fixed_options{};
    fixed_options._size_override = core::Vector2{ 70,25 };
    fixed_options._use_size_override = true;
    auto fixed = std::make_unique<WidthAwareDesiredElement>(core::Rect{ 0,0,1,1 });
    WidthAwareDesiredElement* fixed_raw = fixed.get();
    list.add_child(std::move(fixed),fixed_options);
    list.update_layout_if_dirty();
    require(fixed_raw->screen_rect().size().nearly_equals(core::Vector2{ 70,25 }),
        "explicit layout size override should take precedence over desired extent");
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

void test_overlay_lifetime()
{
    elysia::ui::UiConfirmationDialog dialog(elysia::core::Rect{ 0,0,320,180 });
    elysia::ui::UiWindow unowned_window(elysia::core::Rect{ 0,0,640,480 });
    require(!dialog.register_with_window(unowned_window),"unowned dialogs must not register as overlays");

    auto nested_host = std::make_unique<elysia::ui::UiPanel>(elysia::core::Rect{ 0,0,640,480 });
    auto nested_dialog = std::make_unique<elysia::ui::UiConfirmationDialog>(elysia::core::Rect{ 0,0,320,180 });
    auto* nested_dialog_raw = nested_dialog.get();
    nested_host->add_child(std::move(nested_dialog));
    unowned_window.add_child(std::move(nested_host));
    require(!nested_dialog_raw->register_with_window(unowned_window),"nested dialogs must not register as overlays");

    elysia::ui::UiWindow window(elysia::core::Rect{ 0,0,640,480 });
    auto* owned = window.create_child<elysia::ui::UiConfirmationDialog>(
        elysia::core::Rect{ 0,0,320,180 });
    require(owned != nullptr,"owned dialog should be created");
    require(owned->register_with_window(window),"direct window children should register as overlays");
    owned->open();
    require(window.is_overlay_open(*owned),"registered direct child should open");
    owned->destroy();
    window.update(0.0);
}

void test_transient_popup_lifetime()
{
    elysia::ui::UiDropdown dropdown(elysia::core::Rect{ 0,0,200,40 });
    dropdown.set_options({ { elysia::ui::ui_raw_text("one") } });
    {
        elysia::ui::UiWindow window(elysia::core::Rect{ 0,0,640,480 });
        dropdown.register_with_window(window);
        dropdown.open();
        require(dropdown.is_open(),"registered dropdown should open");
    }
    require(!dropdown.is_open(),"window detach should close dropdown");
    dropdown.open();
    require(!dropdown.is_open(),"detached dropdown must not reopen");

    elysia::ui::UiWindow first_window(elysia::core::Rect{ 0,0,640,480 });
    elysia::ui::UiWindow second_window(elysia::core::Rect{ 0,0,640,480 });
    dropdown.register_with_window(first_window);
    dropdown.register_with_window(first_window);
    dropdown.register_with_window(second_window);
    dropdown.unregister_from_window();
    dropdown.open();
    require(!dropdown.is_open(),"explicitly unregistered dropdown must remain closed");

    {
        auto short_lived = std::make_unique<elysia::ui::UiDropdown>(
            elysia::core::Rect{ 0,0,200,40 });
        short_lived->set_options({ { elysia::ui::ui_raw_text("one") } });
        short_lived->register_with_window(first_window);
    }
    first_window.update(0.0);

    auto owned = std::make_unique<elysia::ui::UiDropdown>(elysia::core::Rect{ 0,0,200,40 });
    auto* owned_raw = owned.get();
    owned_raw->set_options({ { elysia::ui::ui_raw_text("one") } });
    owned_raw->register_with_window(first_window);
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
    tooltip.open();
    require(!tooltip.is_open(),"tooltip without content remains closed after window detach");

    elysia::ui::UiWindow window(elysia::core::Rect{ 0,0,640,480 });
    {
        auto short_lived = std::make_unique<elysia::ui::UiTooltip>();
        short_lived->register_with_window(window);
    }
    window.update(0.0);
}

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
    raw_dropdown->register_with_window(window);
    window.add_child(std::move(dropdown));
    raw_dropdown->open();
    require(raw_dropdown->is_open(),"owned dropdown should open");
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
    require(!raw_dropdown->is_open(),"dropdown should close after confirmation");
}

elysia::ui::UiInputEvent navigation_event(elysia::ui::UiAction action)
{
    return elysia::ui::UiInputEvent{
        .action = action,
        .type = elysia::ui::UiInputEventType::ActionPressed,
        .device = elysia::input::InputDevice::Gamepad
    };
}

void test_deep_nested_focus_propagation()
{
    elysia::ui::UiWindow window(elysia::core::Rect{ 0,0,640,480 });
    auto scroll = std::make_unique<elysia::ui::UiScrollContainer>(elysia::core::Rect{ 0,0,320,240 });
    auto* scroll_raw = scroll.get();
    auto list = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,300,220 });
    auto* list_raw = list.get();
    auto panel = std::make_unique<elysia::ui::UiPanel>(elysia::core::Rect{ 0,0,280,180 });
    auto* panel_raw = panel.get();
    auto grid = std::make_unique<elysia::ui::UiGridContainer>(elysia::core::Rect{ 0,0,260,160 });
    auto* grid_raw = grid.get();
    grid->set_column_count(2);
    auto first = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,100,40 });
    auto second = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,100,40 });
    auto* first_raw = first.get();
    auto* second_raw = second.get();
    require(grid->add_child(std::move(first)) == first_raw,"grid add_child should return the adopted child");
    require(grid->add_child(std::move(second)) == second_raw,"grid add_child should return the adopted child");
    require(panel->add_child(std::move(grid),elysia::ui::UiPanelInsertDirection::Down) == grid_raw,
        "panel add_child should return the adopted child");
    list->add_back(std::move(panel));
    scroll->set_content(std::move(list));
    window.add_child(std::move(scroll));
    window.register_focus_scope(*scroll_raw);

    require(window.focus_first_available_scope(),"window should focus a deeply nested scope");
    require(window.focused_scope() == scroll_raw,"window should retain the registered outer scope");
    require(scroll_raw->is_scope_focused(),"scroll scope should receive window focus");
    require(list_raw->is_scope_focused(),"nested list should receive delegated focus");
    require(panel_raw->is_scope_focused(),"nested panel should receive delegated focus");
    require(grid_raw->is_scope_focused(),"nested grid should receive delegated focus");
    require(scroll_raw->focused_target() == first_raw,"outer scope should expose the focused leaf control");
    require(list_raw->focused_target() == first_raw,"list should expose the same focused leaf control");
    require(panel_raw->focused_target() == first_raw,"panel should expose the same focused leaf control");
    require(grid_raw->focused_target() == first_raw,"grid should initially focus its first button");
    require(first_raw->is_focused() && !second_raw->is_focused(),"only one leaf control should own focus");

    window.on_ui_input_event(navigation_event(elysia::ui::UiAction::NavigateRight));
    require(grid_raw->focused_target() == second_raw,"grid navigation should move focus to the next leaf");
    require(scroll_raw->focused_target() == second_raw,"outer delegated target should follow inner navigation");
    require(!first_raw->is_focused() && second_raw->is_focused(),"leaf focus visuals should remain exclusive");
}

void test_nested_focus_boundary_navigation()
{
    elysia::ui::UiWindow window(elysia::core::Rect{ 0,0,640,240 });
    auto left = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,240,120 });
    auto right = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 300,0,240,120 });
    auto* left_raw = left.get();
    auto* right_raw = right.get();
    auto left_button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,120,40 });
    auto right_button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,120,40 });
    auto* left_button_raw = left_button.get();
    auto* right_button_raw = right_button.get();
    left->add_back(std::move(left_button));
    right->add_back(std::move(right_button));
    window.add_child(std::move(left));
    window.add_child(std::move(right));
    window.register_focus_scope(*left_raw,elysia::ui::UiFocusScopeNeighbors{ nullptr,nullptr,nullptr,right_raw });
    window.register_focus_scope(*right_raw,elysia::ui::UiFocusScopeNeighbors{ nullptr,nullptr,left_raw,nullptr });
    window.set_focused_scope(left_raw);

    require(window.focused_scope() == left_raw && left_button_raw->is_focused(),
        "left scope should own the initial leaf focus");
    window.on_ui_input_event(navigation_event(elysia::ui::UiAction::NavigateRight));
    require(window.focused_scope() == right_raw,"unconsumed boundary navigation should move to the neighbor scope");
    require(!left_raw->is_scope_focused() && right_raw->is_scope_focused(),
        "scope focus should transfer exclusively at the boundary");
    require(!left_button_raw->is_focused() && right_button_raw->is_focused(),
        "leaf focus should transfer with its owning scope");
    window.on_ui_input_event(navigation_event(elysia::ui::UiAction::NavigateLeft));
    require(window.focused_scope() == left_raw && left_button_raw->is_focused(),
        "reverse boundary navigation should restore the previous scope");
}

void test_nested_focus_repair_after_visibility_and_removal()
{
    elysia::ui::UiListContainer list(elysia::core::Rect{ 0,0,240,160 });
    auto first = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,120,40 });
    auto second = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,120,40 });
    auto third = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,120,40 });
    auto* first_raw = first.get();
    auto* second_raw = second.get();
    auto* third_raw = third.get();
    list.add_back(std::move(first));
    list.add_back(std::move(second));
    list.add_back(std::move(third));
    list.set_scope_focused(true);
    require(list.focus_first_available() && list.focused_target() == first_raw,
        "repair test should begin on the first control");

    first_raw->destroy();
    list.update(0.0);
    require(list.focused_target() == second_raw && second_raw->is_focused(),
        "destroying the focused control should repair focus to the next live control");
    second_raw->set_visible(false);
    list.update(0.0);
    require(list.focused_target() == third_raw && third_raw->is_focused(),
        "hiding the focused control should repair focus to the next visible control");
    third_raw->set_enabled(false);
    list.update(0.0);
    require(list.focused_target() == nullptr && !third_raw->is_focused(),
        "disabling the final focusable control should clear stale focus");

    third_raw->set_enabled(true);
    require(list.focus_first_available() && list.focused_target() == third_raw,
        "focus should recover when a valid control becomes available again");
    list.clear_children();
    list.update(0.0);
    require(list.focused_target() == nullptr && !list.has_focusable_target(),
        "clearing the focused subtree should clear every cached target");
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

void test_presentation_translation_animation()
{
    elysia::ui::UiButton button(elysia::core::Rect{ 10,20,100,40 });
    button.bind_translation_animation("linear",{
        .from = elysia::core::Vector2(-40.0f,0.0f),
        .to = elysia::core::Vector2(0.0f,0.0f),
        .duration_seconds = 2.0,
        .easing = elysia::ui::UiTranslationAnimationEasing::Linear
    });
    require(button.play_translation_animation("linear"),"bound translation animation should play");
    require(button.presentation_translation().nearly_equals({ -40.0f,0.0f }),"play should apply animation start translation");
    require(button.screen_rect().nearly_equals(elysia::core::Rect{ 10,20,100,40 }),"presentation animation must not change layout rect");

    button.update_presentation_animations(1.0);
    require(button.presentation_translation().nearly_equals({ -20.0f,0.0f }),"linear animation should interpolate translation");
    require(button.presentation_screen_rect().nearly_equals(elysia::core::Rect{ -10,20,100,40 }),"presentation rect should include local translation");

    button.bind_translation_animation("instant",{
        .from = elysia::core::Vector2(1.0f,2.0f),
        .to = elysia::core::Vector2(3.0f,4.0f),
        .duration_seconds = 0.0
    });
    require(button.play_translation_animation("instant"),"second animation should replace current track");
    require(button.presentation_translation().nearly_equals({ 3.0f,4.0f }),"zero duration animation should complete at destination");
    require(!button.is_translation_animation_playing(),"zero duration animation should not remain playing");
    require(!button.play_translation_animation("missing"),"unknown animation must not play");
    button.reset();
    require(button.presentation_translation().is_zero(),"reset should clear presentation translation");
    require(!button.active_translation_animation().has_value(),"reset should clear animation definitions and active state");
}

void test_presentation_translation_subtree_render_and_hit_test()
{
    elysia::ui::UiChildHost root(elysia::core::Rect{ 0,0,300,200 });
    root.set_presentation_translation({ 10.0f,20.0f });
    auto button = std::make_unique<elysia::ui::UiButton>(elysia::core::Rect{ 0,0,80,30 });
    elysia::ui::UiButton* raw = button.get();
    raw->set_presentation_translation({ 5.0f,7.0f });
    int clicks = 0;
    raw->set_on_click([&clicks] { ++clicks; });
    root.add_child(std::move(button));

    require(raw->accumulated_presentation_translation().nearly_equals({ 15.0f,27.0f }),
        "child presentation translation should accumulate ancestors");
    require(raw->presentation_screen_rect().nearly_equals(elysia::core::Rect{ 15,27,80,30 }),
        "child presentation rect should include ancestor translation");

    root.on_ui_input_event({
        .type = elysia::ui::UiInputEventType::PointerPressed,
        .device = elysia::input::InputDevice::Mouse,
        .control = elysia::input::RawInputControl::MouseLeft,
        .mouse_x = 20,
        .mouse_y = 30
    });
    root.on_ui_input_event({
        .type = elysia::ui::UiInputEventType::PointerReleased,
        .device = elysia::input::InputDevice::Mouse,
        .control = elysia::input::RawInputControl::MouseLeft,
        .mouse_x = 20,
        .mouse_y = 30
    });
    require(clicks == 1,"pointer hit test should follow presentation translation");

    std::vector<elysia::core::UiRenderCommand> commands;
    root.submit_ui_render_commands(commands);
    elysia::ui::render_command_range_utils::apply_translation_to_range(commands,0,root.presentation_translation());
    const auto* fill = find_command(commands,elysia::core::UiRenderCommandType::FillRect);
    require(fill && fill->screen_rect.x() == 15.0f && fill->screen_rect.y() == 27.0f,
        "root and child translations should each apply exactly once to render commands");
}

void test_render_command_range_translation()
{
    std::vector<elysia::core::UiRenderCommand> commands;
    auto rect = elysia::core::make_ui_fill_rect_command(
        elysia::core::Rect{ 1,2,3,4 },elysia::core::Color{});
    elysia::core::set_ui_command_clip_rect(rect,elysia::core::Rect{ 5,6,7,8 });
    commands.push_back(rect);
    commands.push_back(elysia::core::make_ui_draw_line_command({ 1,2 },{ 3,4 },elysia::core::Color{}));
    commands.push_back(elysia::core::make_ui_fill_circle_command({ 5,6 },4.0f,elysia::core::Color{}));
    elysia::ui::render_command_range_utils::apply_translation_to_range(commands,0,{ 10.0f,-2.0f });

    require(commands[0].screen_rect.nearly_equals({ 11,0,3,4 }) && commands[0].clip_rect.nearly_equals({ 15,4,7,8 }),
        "range translation should move rect and clip geometry");
    require(commands[1].line_start.nearly_equals({ 11,0 }) && commands[1].line_end.nearly_equals({ 13,2 }),
        "range translation should move lines");
    require(commands[2].circle_center.nearly_equals({ 15,4 }),"range translation should move circles");
}

void test_gamepad_scroll_synthesizer_axes()
{
    elysia::ui::UiGamepadScrollSynthesizer synthesizer;
    elysia::input::RawInputFrame frame{};
    frame.active_device = elysia::input::InputDevice::Gamepad;
    frame.state.set_axis(elysia::input::RawInputAxis::GamepadLeftX,1.0f);
    frame.state.set_axis(elysia::input::RawInputAxis::GamepadLeftY,1.0f);
    require(!synthesizer.synthesize(frame).has_value(),"first diagonal stick sample should accumulate independently");
    const auto diagonal = synthesizer.synthesize(frame);
    require(diagonal && diagonal->wheel_x == -1 && diagonal->wheel_y == -1,
        "diagonal stick input should synthesize independent horizontal and vertical wheel steps");

    frame.state.set_axis(elysia::input::RawInputAxis::GamepadLeftX,0.1f);
    frame.state.set_axis(elysia::input::RawInputAxis::GamepadLeftY,0.0f);
    require(!synthesizer.synthesize(frame).has_value(),"deadzone input should reset both idle axis accumulators");

    frame.state.set_axis(elysia::input::RawInputAxis::GamepadLeftX,-1.0f);
    require(!synthesizer.synthesize(frame).has_value(),"fresh horizontal input should not inherit pre-deadzone accumulation");
    const auto horizontal = synthesizer.synthesize(frame);
    require(horizontal && horizontal->wheel_x == 1 && horizontal->wheel_y == 0,
        "horizontal stick input should emit only horizontal wheel steps");

    frame.device_switched_this_frame = true;
    require(!synthesizer.synthesize(frame).has_value(),"device switches should clear pending scroll accumulation");
    frame.device_switched_this_frame = false;
    require(!synthesizer.synthesize(frame).has_value(),"post-switch input should restart accumulation from zero");
}

void test_callback_exceptions_reach_window_scene_and_boundary()
{
    using namespace elysia;
    const ui::UiInputEvent pressed{ .action=ui::UiAction::Confirm,.type=ui::UiInputEventType::ActionPressed };
    const ui::UiInputEvent released{ .action=ui::UiAction::Confirm,.type=ui::UiInputEventType::ActionReleased };

    ui::UiWindow window(core::Rect{ 0,0,320,120 });
    auto panel = std::make_unique<ui::UiPanel>(core::Rect{ 0,0,320,120 });
    ui::UiPanel* panel_raw = panel.get();
    auto checkbox = std::make_unique<ui::UiCheckbox>(core::Rect{ 0,0,40,40 });
    checkbox->set_on_toggled([](ui::UiCheckboxState) { throw std::runtime_error("checkbox callback"); });
    panel_raw->add_child(std::move(checkbox));
    window.add_child(std::move(panel));
    window.register_focus_scope(*panel_raw);
    require(window.focus_first_available_scope(),"window should focus the checkbox panel");
    require(window.on_ui_input_event(pressed),"checkbox confirm press should route through window");
    bool window_threw = false;
    try { (void)window.on_ui_input_event(released); }
    catch (const std::runtime_error&) { window_threw = true; }
    require(window_threw,"callback exceptions must escape the window input path");

    TestScene scene;
    auto scene_window = std::make_unique<ui::UiWindow>(core::Rect{ 0,0,320,120 });
    auto scene_panel = std::make_unique<ui::UiPanel>(core::Rect{ 0,0,320,120 });
    ui::UiPanel* scene_panel_raw = scene_panel.get();
    auto scene_checkbox = std::make_unique<ui::UiCheckbox>(core::Rect{ 0,0,40,40 });
    scene_checkbox->set_on_toggled([](ui::UiCheckboxState) { throw std::runtime_error("scene callback"); });
    scene_panel_raw->add_child(std::move(scene_checkbox));
    scene_window->add_child(std::move(scene_panel));
    scene_window->register_focus_scope(*scene_panel_raw);
    require(scene_window->focus_first_available_scope(),"scene window should focus the checkbox panel");
    scene.add_object(std::move(scene_window));
    const input::RawInputEvent raw_press{ .control=input::RawInputControl::KeyEnter,.type=input::RawInputEventType::ControlPressed,.device=input::InputDevice::Keyboard };
    const input::RawInputEvent raw_release{ .control=input::RawInputControl::KeyEnter,.type=input::RawInputEventType::ControlReleased,.device=input::InputDevice::Keyboard };
    bool scene_threw = false;
    try { scene.on_input({}, { raw_press,raw_release }); }
    catch (const std::runtime_error&) { scene_threw = true; }
    require(scene_threw,"callback exceptions must escape the full Scene UI input route");

    bool continued = false;
    const bool completed = moonline::application::run_event_boundary("test",[&]()
    {
        throw std::runtime_error("boundary callback");
        continued = true;
    });
    require(!completed && !continued,"application boundary must catch callback exceptions and stop the phase");
}

void test_child_host_tolerates_callback_tree_mutation()
{
    using namespace elysia;
    for (const auto trigger : { ClearingChild::Trigger::Update,ClearingChild::Trigger::Frame,ClearingChild::Trigger::Event })
    {
        ui::UiChildHost host;
        int clear_calls = 0;
        int sibling_calls = 0;
        if (trigger == ClearingChild::Trigger::Event)
        {
            host.add_child(std::make_unique<CountingChild>(sibling_calls));
            host.add_child(std::make_unique<ClearingChild>(host,trigger,clear_calls));
        }
        else
        {
            host.add_child(std::make_unique<ClearingChild>(host,trigger,clear_calls));
            host.add_child(std::make_unique<CountingChild>(sibling_calls));
        }
        if (trigger == ClearingChild::Trigger::Update)
            host.update(0.0);
        else if (trigger == ClearingChild::Trigger::Frame)
            host.on_ui_input_frame({});
        else
            (void)host.on_ui_input_event({});
        require(clear_calls == 1 && sibling_calls == 0 && host.child_count() == 0,
            "host traversal must tolerate a child clearing the tree");
    }
}

void test_text_input_callback_can_remove_its_owner()
{
    using namespace elysia;
    ui::UiChildHost host;
    auto input = std::make_unique<ui::UiTextInput>(core::Rect{ 0,0,160,40 });
    ui::UiTextInput* raw_input = input.get();
    raw_input->set_on_text_changed([&](std::string_view) { host.clear_children(); });
    host.add_child(std::move(input));
    raw_input->set_text("new text");
    require(host.child_count() == 0,"text callback should be able to remove its owning subtree");
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

std::string read_text_file(const std::filesystem::path& path)
{
    std::ifstream input(path);
    return { std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>() };
}

std::size_t count_occurrences(std::string_view text,std::string_view needle)
{
    std::size_t count = 0;
    std::size_t position = 0;
    while ((position = text.find(needle,position)) != std::string_view::npos)
    {
        ++count;
        position += needle.size();
    }
    return count;
}

void remove_test_path(const std::filesystem::path& path)
{
    std::error_code error;
    std::filesystem::remove_all(path,error);
    require(!error,"logger test cleanup must succeed");
}

struct CapturedSdlLogs
{
    std::vector<std::string> messages;
};

void SDLCALL capture_sdl_log(void* userdata,int,SDL_LogPriority,const char* message)
{
    auto* captured = static_cast<CapturedSdlLogs*>(userdata);
    captured->messages.emplace_back(message ? message : "");
}

void test_logger_console_sink()
{
    using namespace elysia;
    auto* path_manager = io::PathManager::instance();
    require(path_manager->init() && path_manager->ensure_runtime_dirs(),
        "console logger test must initialize runtime paths");
    auto* logger = tools::Logger::instance();
    logger->shutdown();

    SDL_LogOutputFunction previous_callback = nullptr;
    void* previous_userdata = nullptr;
    SDL_LogGetOutputFunction(&previous_callback,&previous_userdata);
    CapturedSdlLogs captured;
    captured.messages.reserve(8);
    SDL_LogSetOutputFunction(capture_sdl_log,&captured);

    tools::LoggerConfig console_config;
    console_config.file_mode = tools::LogFileMode::Disabled;
    require(logger->configure(console_config),"logger must accept console configuration");
    logger->initialize();
    const unsigned int console_call_line = __LINE__ + 1;
    logger->info("console-test","console marker");
    require(captured.messages.size() == 1,"enabled console sink must emit exactly once");
    require(captured.messages.front().find("[INFO]") != std::string::npos
            && captured.messages.front().find("[console-test]") != std::string::npos
            && captured.messages.front().find("console marker") != std::string::npos
            && captured.messages.front().find("ui_lifecycle_tests.cpp:" + std::to_string(console_call_line)) != std::string::npos,
        "console sink must emit the formatted file-log line with the original call site");
    logger->shutdown();

    captured.messages.clear();
    const std::filesystem::path dual_sink_path = path_manager->logs() / "ui-lifecycle-console-dual.log";
    remove_test_path(dual_sink_path);
    tools::LoggerConfig dual_sink_config;
    dual_sink_config.file_mode = tools::LogFileMode::Append;
    dual_sink_config.append_file_name = dual_sink_path.filename().string();
    require(logger->configure(dual_sink_config),"logger must accept dual-sink configuration");
    logger->initialize();
    logger->warn("console-test","dual sink marker");
    logger->shutdown();
    require(captured.messages.size() == 1 && captured.messages.front().find("dual sink marker") != std::string::npos,
        "enabled console sink must receive one entry when the file sink succeeds");
    require(count_occurrences(read_text_file(dual_sink_path),"dual sink marker") == 1,
        "enabled file sink must receive the same entry exactly once");
    remove_test_path(dual_sink_path);

    captured.messages.clear();
    const std::filesystem::path silent_path = path_manager->logs() / "ui-lifecycle-console-silent.log";
    remove_test_path(silent_path);
    tools::LoggerConfig silent_config;
    silent_config.file_mode = tools::LogFileMode::Append;
    silent_config.append_file_name = silent_path.filename().string();
    silent_config.console_enabled = false;
    require(logger->configure(silent_config),"logger must accept silent console configuration");
    logger->info("console-test","preinit silent marker");
    logger->initialize();
    logger->warn("console-test","initialized silent marker");
    require(captured.messages.empty(),"disabled console sink must suppress preinit and initialized output");
    logger->shutdown();
    require(read_text_file(silent_path).find("initialized silent marker") != std::string::npos,
        "disabled console sink must not disable the configured file sink");
    remove_test_path(silent_path);

    const std::filesystem::path blocked_path = path_manager->logs() / "ui-lifecycle-console-blocked";
    remove_test_path(blocked_path);
    std::error_code directory_error;
    std::filesystem::create_directory(blocked_path,directory_error);
    require(!directory_error,"console logger test must create a blocking directory");
    captured.messages.clear();
    tools::LoggerConfig fallback_config;
    fallback_config.file_mode = tools::LogFileMode::Append;
    fallback_config.append_file_name = blocked_path.filename().string();
    fallback_config.console_enabled = true;
    require(logger->configure(fallback_config),"logger must accept file-fallback configuration");
    logger->initialize();
    logger->error("console-test","file fallback marker");
    require(captured.messages.size() == 1
            && captured.messages.front().find("file fallback marker") != std::string::npos,
        "file failure must still emit once through the enabled console sink");
    logger->shutdown();
    remove_test_path(blocked_path);

    SDL_LogSetOutputFunction(previous_callback,previous_userdata);
}

void test_logger_file_modes_and_noexcept()
{
    using namespace elysia;
    auto* path_manager = io::PathManager::instance();
    require(path_manager->init(),"logger test must initialize PathManager");
    require(path_manager->ensure_runtime_dirs(),"logger test must create runtime directories");

    auto* logger = tools::Logger::instance();
    logger->shutdown();

    tools::LoggerConfig disabled_config;
    disabled_config.file_mode = tools::LogFileMode::Disabled;
    require(logger->configure(disabled_config),"logger must accept configuration before initialization");
    logger->initialize();
    logger->debug("logger-test","disabled marker");
    require(!logger->active_file_path().has_value(),"disabled logger must not open a file");
    require(!logger->configure(disabled_config),"logger configuration must be fixed after initialization");
    logger->shutdown();

    const std::filesystem::path append_path = path_manager->logs() / "ui-lifecycle-logger-append.log";
    remove_test_path(append_path);
    tools::LoggerConfig append_config;
    append_config.file_mode = tools::LogFileMode::Append;
    append_config.append_file_name = append_path.filename().string();
    require(logger->configure(append_config),"logger must accept append configuration");
    logger->initialize();
    const unsigned int append_call_line = __LINE__ + 1;
    logger->info("logger-test","append first marker");
    const auto active_append_path = logger->active_file_path();
    require(active_append_path.has_value() && *active_append_path == append_path,
        "append logger must expose its active file path");
    logger->shutdown();
    require(read_text_file(append_path).find("append first marker") != std::string::npos,
        "append logger must create and write its configured file");
    require(read_text_file(append_path).find("ui_lifecycle_tests.cpp:" + std::to_string(append_call_line)) != std::string::npos,
        "logger must retain the call-site source location");

    require(logger->configure(append_config),"logger must allow reconfiguration after shutdown");
    logger->initialize();
    logger->warn("logger-test","append second marker");
    logger->shutdown();
    const std::string append_contents = read_text_file(append_path);
    require(append_contents.find("append first marker") != std::string::npos
            && append_contents.find("append second marker") != std::string::npos,
        "append logger must preserve earlier content");
    remove_test_path(append_path);

    const std::filesystem::path filtered_path = path_manager->logs() / "ui-lifecycle-logger-filtered.log";
    remove_test_path(filtered_path);
    tools::LoggerConfig filtered_config;
    filtered_config.minimum_level = tools::LogLevel::Warn;
    filtered_config.file_mode = tools::LogFileMode::Append;
    filtered_config.append_file_name = filtered_path.filename().string();
    require(logger->configure(filtered_config),"logger must accept level filtering configuration");
    logger->initialize();
    logger->debug("logger-test","filtered debug marker");
    logger->warn("logger-test","retained warn marker");
    logger->shutdown();
    const std::string filtered_contents = read_text_file(filtered_path);
    require(filtered_contents.find("filtered debug marker") == std::string::npos
            && filtered_contents.find("retained warn marker") != std::string::npos,
        "logger must filter entries below its configured minimum level");
    remove_test_path(filtered_path);

    tools::LoggerConfig new_run_config;
    new_run_config.file_mode = tools::LogFileMode::NewRunFile;
    require(logger->configure(new_run_config),"logger must accept new-run configuration");
    logger->initialize();
    const auto first_run_path = logger->active_file_path();
    require(first_run_path.has_value() && first_run_path->filename().string().starts_with("Elysia-"),
        "new-run logger must use a timestamped filename");
    logger->error("logger-test","new run marker");
    logger->shutdown();
    require(logger->configure(new_run_config),"logger must allow a second new-run configuration");
    logger->initialize();
    const auto second_run_path = logger->active_file_path();
    require(second_run_path.has_value() && *second_run_path != *first_run_path,
        "new-run logger must avoid reusing an existing run file");
    logger->shutdown();
    require(read_text_file(*first_run_path).find("new run marker") != std::string::npos,
        "new-run logger must write its active file");
    remove_test_path(*first_run_path);
    remove_test_path(*second_run_path);

    const std::filesystem::path blocked_path = path_manager->logs() / "ui-lifecycle-logger-blocked";
    remove_test_path(blocked_path);
    std::error_code directory_error;
    std::filesystem::create_directory(blocked_path,directory_error);
    require(!directory_error,"logger test must create a blocking directory");
    tools::LoggerConfig blocked_config;
    blocked_config.file_mode = tools::LogFileMode::Append;
    blocked_config.append_file_name = blocked_path.filename().string();
    require(logger->configure(blocked_config),"logger must accept blocking-file configuration");
    logger->initialize();
    require(!logger->active_file_path().has_value(),"failed file open must disable the file sink");
    logger->log(tools::LogLevel::Debug,"logger-test","file failure log marker");
    logger->debug("logger-test","file failure debug marker");
    logger->info("logger-test","file failure info marker");
    logger->warn("logger-test","file failure warn marker");
    logger->error("logger-test","file failure error marker");
    logger->shutdown();
    remove_test_path(blocked_path);
}
}

int main()
{
    test_list_consumes_desired_extent_and_cross_alignment();
    test_scroll_offset_does_not_remeasure_allocated_content_as_growth();
    test_corner_radius_normalization();
    test_chrome_uses_single_rounded_outer_frame();
    test_field_level_style_cascade();
    test_overlay_lifetime();
    test_transient_popup_lifetime();
    test_tooltip_lifetime();
    test_group_repairs_selection_without_group_callback();
    test_radio_render_defers_callback();
    test_group_preserves_button_override();
    test_empty_focus_scopes();
    test_nested_focus_and_dropdown_navigation();
    test_deep_nested_focus_propagation();
    test_nested_focus_boundary_navigation();
    test_nested_focus_repair_after_visibility_and_removal();
    test_slider_adjustment_mode();
    test_button_interactive_border_colors();
    test_textured_button_border();
    test_other_chrome_active_borders();
    test_builtin_theme_border_states();
    test_container_driven_theme_tree();
    test_labeled_control_text_follows_theme();
    test_labeled_controls_preserve_label_base_style();
    test_presentation_translation_animation();
    test_presentation_translation_subtree_render_and_hit_test();
    test_render_command_range_translation();
    test_gamepad_scroll_synthesizer_axes();
    test_callback_exceptions_reach_window_scene_and_boundary();
    test_child_host_tolerates_callback_tree_mutation();
    test_text_input_callback_can_remove_its_owner();
    test_button_group_preserves_button_callback_after_selection();
    test_logger_file_modes_and_noexcept();
    test_logger_console_sink();
    std::cout << "ui lifecycle tests passed\n";
    return EXIT_SUCCESS;
}
