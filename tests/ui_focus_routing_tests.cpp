#define SDL_MAIN_HANDLED

#include "../engine/ui/composites/ui_tab_container.h"
#include "../engine/ui/containers/ui_chrome_container.h"
#include "../engine/ui/containers/ui_grid_container.h"
#include "../engine/ui/containers/ui_list_container.h"
#include "../engine/ui/containers/ui_scroll_container.h"
#include "../engine/ui/widgets/ui_button.h"
#include "../engine/ui/widgets/ui_slider.h"
#include "../engine/ui/window/ui_window.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>

namespace
{
using namespace elysia;

void require(bool condition,const char* message)
{
    if (condition)
        return;
    std::cerr << "FAILED: " << message << '\n';
    std::exit(EXIT_FAILURE);
}

ui::UiInputEvent navigation_event(ui::UiAction action,input::InputDevice device)
{
    return {
        .action = action,
        .type = ui::UiInputEventType::ActionPressed,
        .device = device
    };
}

ui::UiInputEvent confirm_event(ui::UiInputEventType type,input::InputDevice device)
{
    return {
        .action = ui::UiAction::Confirm,
        .type = type,
        .device = device
    };
}

struct TabFixture
{
    ui::UiTabContainer tabs{ core::Rect{ 0,0,540,300 } };
    ui::UiListContainer* pages[3]{};
    ui::UiButton* buttons[3][3]{};
    int selection_callbacks = 0;

    TabFixture()
    {
        for (std::size_t page_index = 0; page_index < 3; ++page_index)
        {
            auto page = std::make_unique<ui::UiListContainer>(core::Rect{ 0,0,500,220 });
            pages[page_index] = page.get();
            for (std::size_t button_index = 0; button_index < 3; ++button_index)
            {
                auto button = std::make_unique<ui::UiButton>(core::Rect{ 0,0,160,36 });
                buttons[page_index][button_index] = button.get();
                page->add_back(std::move(button));
            }
            require(tabs.add_tab(ui::ui_raw_text("Tab " + std::to_string(page_index)),std::move(page)).added,
                "fixture tab should be added");
        }
        tabs.set_on_selection_changed([this](std::optional<std::size_t>) { ++selection_callbacks; });
        tabs.set_scope_focused(true);
        require(tabs.focus_first_available(),"tab container should focus its first tab");
    }
};

std::unique_ptr<ui::UiScrollContainer> scrolled_section_page(
    std::unique_ptr<ui::UiElement> first_content,
    ui::UiButton*& trailing_button)
{
    auto page = std::make_unique<ui::UiScrollContainer>(core::Rect{ 0,0,900,390 });
    page->set_scroll_axis(ui::UiScrollAxis::Vertical);
    auto page_list = std::make_unique<ui::UiListContainer>(core::Rect{ 0,0,870,500 });
    auto section = std::make_unique<ui::UiChromeContainer>(core::Rect{ 0,0,840,420 });
    auto section_body = std::make_unique<ui::UiListContainer>(core::Rect{ 0,0,820,360 });
    section_body->add_back(std::move(first_content));
    auto trailing = std::make_unique<ui::UiButton>(core::Rect{ 0,0,240,40 });
    trailing_button = trailing.get();
    section_body->add_back(std::move(trailing));
    section->set_body(std::move(section_body));
    page_list->add_back(std::move(section));
    page->set_content(std::move(page_list));
    return page;
}

struct WindowTabFixture
{
    ui::UiWindow window{ core::Rect{ 0,0,1120,616 } };
    ui::UiTabContainer* tabs = nullptr;

    explicit WindowTabFixture(std::unique_ptr<ui::UiElement> page)
    {
        auto owned_tabs = std::make_unique<ui::UiTabContainer>(core::Rect{ 0,0,1080,530 });
        tabs = owned_tabs.get();
        require(tabs->add_tab(ui::ui_raw_text("Page"),std::move(page)).added,"window fixture page should be added");
        window.add_child(std::move(owned_tabs));
        window.register_focus_scope(*tabs);
        require(window.focus_first_available_scope(),"window should focus the tab scope");
    }
};

void run_keyboard_gamepad_matrix(input::InputDevice device)
{
    TabFixture fixture;
    require(fixture.tabs.focused_index() == 0,"first tab should be focused after scope entry");
    require(fixture.tabs.selected_index() == 0,"first tab should be selected after construction");

    require(fixture.tabs.on_ui_input_event(navigation_event(ui::UiAction::NavigateRight,device)),
        "tab bar should consume right navigation");
    require(fixture.tabs.focused_index() == 1 && fixture.tabs.selected_index() == 0,
        "tab focus and selection must remain independent");

    fixture.tabs.on_ui_input_event(confirm_event(ui::UiInputEventType::ActionPressed,device));
    fixture.tabs.on_ui_input_event(confirm_event(ui::UiInputEventType::ActionReleased,device));
    require(fixture.tabs.focused_index() == 1 && fixture.tabs.selected_index() == 1,
        "confirm should select the focused tab");
    require(fixture.selection_callbacks == 1,"confirm should emit one selection callback");

    require(fixture.tabs.on_ui_input_event(navigation_event(ui::UiAction::NavigateDown,device)),
        "down from the tab bar should enter the selected page");
    require(fixture.pages[1]->focused_target() == fixture.buttons[1][0],
        "page entry should focus the first available control");
    require(fixture.tabs.focused_target() == fixture.buttons[1][0],
        "tab container should mirror the page's focused leaf");

    require(fixture.tabs.on_ui_input_event(navigation_event(ui::UiAction::NavigateDown,device)),
        "page scope should consume navigation to its second control");
    require(fixture.pages[1]->focused_target() == fixture.buttons[1][1],
        "second down should move beyond the first page control");
    require(fixture.tabs.focused_target() == fixture.buttons[1][1],
        "nested focus movement should propagate back to the tab container");

    require(fixture.tabs.on_ui_input_event(navigation_event(ui::UiAction::NavigateDown,device)),
        "page scope should consume navigation to its third control");
    require(fixture.pages[1]->focused_target() == fixture.buttons[1][2],
        "third down should reach the last page control");

    require(fixture.tabs.on_ui_input_event(navigation_event(ui::UiAction::NavigateUp,device)),
        "up inside the page should be consumed by the page scope first");
    require(fixture.pages[1]->focused_target() == fixture.buttons[1][1],
        "up inside the page should move to the previous control");
    fixture.tabs.on_ui_input_event(navigation_event(ui::UiAction::NavigateUp,device));
    require(fixture.pages[1]->focused_target() == fixture.buttons[1][0],
        "page should return to its first control before leaving the page");
    require(fixture.tabs.on_ui_input_event(navigation_event(ui::UiAction::NavigateUp,device)),
        "up at the page boundary should return to the tab bar");
    require(fixture.tabs.focused_index() == 1,"returning to the tab bar should restore its focused tab");
}

void run_real_nested_page_matrix(input::InputDevice device)
{
    auto first_button = std::make_unique<ui::UiButton>(core::Rect{ 0,0,240,40 });
    ui::UiButton* first = first_button.get();
    ui::UiButton* second = nullptr;
    WindowTabFixture overlays(scrolled_section_page(std::move(first_button),second));

    require(overlays.window.on_ui_input_event(navigation_event(ui::UiAction::NavigateDown,device)),
        "window should route down through tab, scroll and chrome into the section body");
    require(overlays.tabs->focused_target() == first,"nested overlay-style page should focus its first button");
    overlays.window.update(1.0 / 60.0);
    require(overlays.tabs->focused_target() == first,"frame synchronization must preserve the nested focused leaf");
    require(overlays.window.on_ui_input_event(navigation_event(ui::UiAction::NavigateDown,device)),
        "nested section body should consume its second down");
    require(overlays.tabs->focused_target() == second,"nested overlay-style page should reach its second button");
    overlays.window.update(1.0 / 60.0);
    require(overlays.tabs->focused_target() == second,"chrome update must not reset body focus to its first control");

    auto grid = std::make_unique<ui::UiGridContainer>(core::Rect{ 0,0,560,170 });
    grid->set_column_count(3);
    ui::UiButton* grid_buttons[6]{};
    for (std::size_t index = 0; index < 6; ++index)
    {
        auto button = std::make_unique<ui::UiButton>(core::Rect{ 0,0,160,40 });
        grid_buttons[index] = button.get();
        grid->add_child(std::move(button));
    }
    ui::UiButton* after_grid = nullptr;
    WindowTabFixture containers(scrolled_section_page(std::move(grid),after_grid));

    require(containers.window.on_ui_input_event(navigation_event(ui::UiAction::NavigateDown,device)),
        "window should route down into a grid nested below scroll and chrome");
    require(containers.tabs->focused_target() == grid_buttons[0],"container-style page should enter the first grid cell");
    require(containers.window.on_ui_input_event(navigation_event(ui::UiAction::NavigateDown,device)),
        "grid should consume vertical navigation inside the page");
    require(containers.tabs->focused_target() == grid_buttons[3],"grid down should reach the next row");
    containers.window.update(1.0 / 60.0);
    require(containers.tabs->focused_target() == grid_buttons[3],"frame synchronization must preserve nested grid focus");
    require(containers.window.on_ui_input_event(navigation_event(ui::UiAction::NavigateDown,device)),
        "grid boundary should propagate to the next section-body item");
    require(containers.tabs->focused_target() == after_grid,"grid boundary should enter the following control");
}

void run_slider_focus_matrix(input::InputDevice device)
{
    ui::UiWindow window{ core::Rect{ 0,0,640,480 } };
    auto list = std::make_unique<ui::UiListContainer>(core::Rect{ 0,0,400,300 });
    ui::UiListContainer* list_raw = list.get();
    auto before = std::make_unique<ui::UiButton>(core::Rect{ 0,0,200,40 });
    ui::UiButton* before_raw = before.get();
    auto slider = std::make_unique<ui::UiSlider>(core::Rect{ 0,0,240,40 });
    ui::UiSlider* slider_raw = slider.get();
    slider->set_step(0.1f);
    slider->set_value(0.5f);
    auto after = std::make_unique<ui::UiButton>(core::Rect{ 0,0,200,40 });
    ui::UiButton* after_raw = after.get();
    list->add_back(std::move(before));
    list->add_back(std::move(slider));
    list->add_back(std::move(after));
    window.add_child(std::move(list));
    window.register_focus_scope(*list_raw);
    require(window.focus_first_available_scope() && list_raw->focused_target() == before_raw,
        "slider focus fixture should start on the preceding button");

    window.on_ui_input_event(navigation_event(ui::UiAction::NavigateDown,device));
    require(list_raw->focused_target() == slider_raw && !slider_raw->is_adjusting(),
        "normal navigation should enter the focused slider without beginning adjustment");
    window.on_ui_input_event(navigation_event(ui::UiAction::NavigateDown,device));
    require(list_raw->focused_target() == after_raw,
        "unconfirmed slider must yield its primary container direction");

    window.on_ui_input_event(navigation_event(ui::UiAction::NavigateUp,device));
    require(list_raw->focused_target() == slider_raw,"up should return focus to the slider");
    window.on_ui_input_event(confirm_event(ui::UiInputEventType::ActionPressed,device));
    require(slider_raw->is_adjusting(),"confirm should begin slider adjustment inside a list");
    const float before_adjustment = slider_raw->value();
    window.on_ui_input_event(navigation_event(ui::UiAction::NavigateRight,device));
    require(slider_raw->value() > before_adjustment,"adjusting slider should consume and apply its horizontal axis");
    window.on_ui_input_event(confirm_event(ui::UiInputEventType::ActionPressed,device));
    require(!slider_raw->is_adjusting(),"second confirm should exit slider adjustment");
    window.on_ui_input_event(navigation_event(ui::UiAction::NavigateDown,device));
    require(list_raw->focused_target() == after_raw,"exited slider adjustment must restore container navigation");

    auto nested_slider = std::make_unique<ui::UiSlider>(core::Rect{ 0,0,240,40 });
    ui::UiSlider* nested_slider_raw = nested_slider.get();
    nested_slider->set_step(0.1f);
    nested_slider->set_value(0.5f);
    ui::UiButton* nested_after = nullptr;
    WindowTabFixture nested(scrolled_section_page(std::move(nested_slider),nested_after));
    nested.window.on_ui_input_event(navigation_event(ui::UiAction::NavigateDown,device));
    require(nested.tabs->focused_target() == nested_slider_raw,"nested scroll/chrome page should enter its slider");
    nested.window.on_ui_input_event(confirm_event(ui::UiInputEventType::ActionPressed,device));
    require(nested_slider_raw->is_adjusting(),"nested slider should enter adjustment mode");
    nested.window.on_ui_input_event(navigation_event(ui::UiAction::NavigateDown,device));
    require(nested.tabs->focused_target() == nested_after && !nested_slider_raw->is_adjusting(),
        "secondary navigation must leave nested slider and clear adjustment mode");
}

}

int main()
{
    run_keyboard_gamepad_matrix(input::InputDevice::Keyboard);
    run_keyboard_gamepad_matrix(input::InputDevice::Gamepad);
    run_real_nested_page_matrix(input::InputDevice::Keyboard);
    run_real_nested_page_matrix(input::InputDevice::Gamepad);
    run_slider_focus_matrix(input::InputDevice::Keyboard);
    run_slider_focus_matrix(input::InputDevice::Gamepad);
    std::cout << "ui focus routing tests passed\n";
    return EXIT_SUCCESS;
}
