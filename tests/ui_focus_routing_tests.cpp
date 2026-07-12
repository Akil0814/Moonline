#define SDL_MAIN_HANDLED

#include "../engine/ui/composites/ui_tab_container.h"
#include "../engine/ui/containers/ui_list_container.h"
#include "../engine/ui/widgets/ui_button.h"

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

}

int main()
{
    run_keyboard_gamepad_matrix(input::InputDevice::Keyboard);
    run_keyboard_gamepad_matrix(input::InputDevice::Gamepad);
    std::cout << "ui focus routing tests passed\n";
    return EXIT_SUCCESS;
}
