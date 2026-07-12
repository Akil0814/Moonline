#define SDL_MAIN_HANDLED

#include "../engine/ui/composites/ui_tab_container.h"
#include "../engine/ui/containers/ui_list_container.h"
#include "../engine/ui/widgets/ui_button.h"
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

ui::UiInputEvent action_event(ui::UiAction action,input::InputDevice device)
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
    ui::UiWindow window{ core::Rect{ 0,0,900,480 } };
    ui::UiListContainer* left_scope = nullptr;
    ui::UiTabContainer* tabs = nullptr;
    ui::UiListContainer* right_scope = nullptr;
    ui::UiListContainer* pages[3]{};
    ui::UiButton* page_buttons[3][2]{};
    int selection_callbacks = 0;

    TabFixture()
    {
        auto left = std::make_unique<ui::UiListContainer>(core::Rect{ 0,0,100,100 });
        left_scope = left.get();
        left->add_back(std::make_unique<ui::UiButton>(core::Rect{ 0,0,80,32 }));

        auto tab_container = std::make_unique<ui::UiTabContainer>(core::Rect{ 120,0,540,300 });
        tabs = tab_container.get();
        for (std::size_t index = 0; index < 3; ++index)
        {
            auto page = std::make_unique<ui::UiListContainer>(core::Rect{ 0,0,500,220 });
            pages[index] = page.get();
            for (std::size_t button_index = 0; button_index < 2; ++button_index)
            {
                auto button = std::make_unique<ui::UiButton>(core::Rect{ 0,0,160,36 });
                page_buttons[index][button_index] = button.get();
                page->add_back(std::move(button));
            }
            require(tabs->add_tab(ui::ui_raw_text("Tab " + std::to_string(index)),std::move(page)).added,
                "fixture tab should be added");
        }
        tabs->set_on_selection_changed([this](std::optional<std::size_t>) { ++selection_callbacks; });

        auto right = std::make_unique<ui::UiListContainer>(core::Rect{ 700,0,100,100 });
        right_scope = right.get();
        right->add_back(std::make_unique<ui::UiButton>(core::Rect{ 0,0,80,32 }));

        window.add_child(std::move(left));
        window.add_child(std::move(tab_container));
        window.add_child(std::move(right));
        window.register_focus_scope(*left_scope,ui::UiFocusScopeNeighbors{ nullptr,nullptr,nullptr,tabs });
        window.register_focus_scope(*tabs,ui::UiFocusScopeNeighbors{ nullptr,nullptr,left_scope,right_scope });
        window.register_focus_scope(*right_scope,ui::UiFocusScopeNeighbors{ nullptr,nullptr,tabs,nullptr });
        window.set_focused_scope(tabs);
        require(tabs->set_focused_index(0),"fixture should initialize the first tab focus identity");
    }
};

void run_tab_keyboard_gamepad_matrix(input::InputDevice device)
{
    TabFixture fixture;
    require(fixture.tabs->selected_index() == 0,"first tab should be selected during construction");
    require(fixture.tabs->focused_index() == 0,"scope entry should establish focus on the first tab");
    require(fixture.pages[0]->is_visible() && fixture.pages[0]->is_active(),"selected page should be live");
    require(!fixture.pages[1]->is_visible() && !fixture.pages[1]->is_active(),"unselected page must be inactive");

    fixture.window.on_ui_input_event(action_event(ui::UiAction::NavigateRight,device));
    require(fixture.tabs->focused_index() == 1,"right navigation should move tab focus");
    require(fixture.tabs->selected_index() == 0,"right navigation must not select a tab");
    require(fixture.selection_callbacks == 0,"focus movement must not emit selection callbacks");

    fixture.window.on_ui_input_event(confirm_event(ui::UiInputEventType::ActionPressed,device));
    fixture.window.on_ui_input_event(confirm_event(ui::UiInputEventType::ActionReleased,device));
    require(fixture.tabs->focused_index() == 1 && fixture.tabs->selected_index() == 1,
        "confirm should select the focused tab without moving focus");
    require(fixture.selection_callbacks == 1,"confirm should emit exactly one selection callback");
    require(!fixture.pages[0]->is_visible() && !fixture.pages[0]->is_active(),"previous page must be hidden after selection");
    require(fixture.pages[1]->is_visible() && fixture.pages[1]->is_active(),"new selected page must be live");

    fixture.window.on_ui_input_event(action_event(ui::UiAction::NavigateLeft,device));
    require(fixture.tabs->focused_index() == 0 && fixture.tabs->selected_index() == 1,
        "left navigation should remain independent from selected tab");
    fixture.window.on_ui_input_event(action_event(ui::UiAction::NavigateLeft,device));
    require(fixture.window.focused_scope() == fixture.left_scope,"left tab boundary should transfer to window neighbor scope");
    fixture.window.on_ui_input_event(action_event(ui::UiAction::NavigateRight,device));
    require(fixture.window.focused_scope() == fixture.tabs,"reverse boundary navigation should return to tabs");
    require(fixture.tabs->focused_index() == 0,"returning to tab scope should preserve its focused tab");
}

void test_mouse_selection_and_subsequent_navigation()
{
    TabFixture fixture;
    fixture.window.update_layout_if_dirty();
    const core::Rect tab_rect = fixture.tabs->screen_rect();
    const int second_tab_x = static_cast<int>(tab_rect.left() + 180.0f);
    const int tab_y = static_cast<int>(tab_rect.top() + 20.0f);
    fixture.window.on_ui_input_event({ .type = ui::UiInputEventType::PointerPressed,.device = input::InputDevice::Mouse,
        .control = input::RawInputControl::MouseLeft,.mouse_x = second_tab_x,.mouse_y = tab_y });
    fixture.window.on_ui_input_event({ .type = ui::UiInputEventType::PointerReleased,.device = input::InputDevice::Mouse,
        .control = input::RawInputControl::MouseLeft,.mouse_x = second_tab_x,.mouse_y = tab_y });
    require(fixture.tabs->focused_index() == 1 && fixture.tabs->selected_index() == 1,
        "mouse click should update focused and selected tab through separate paths");

    fixture.window.on_ui_input_event(action_event(ui::UiAction::NavigateRight,input::InputDevice::Keyboard));
    require(fixture.tabs->focused_index() == 2 && fixture.tabs->selected_index() == 1,
        "keyboard navigation should resume from the mouse-focused tab");
}

void test_tab_api_independence_and_mutation_repair()
{
    TabFixture fixture;
    require(fixture.tabs->set_focused_index(2),"focused index should accept a valid tab");
    require(fixture.tabs->selected_index() == 0,"setting focused index must not change selection");
    require(fixture.tabs->set_selected_index(1),"selected index should accept a valid tab");
    require(fixture.tabs->focused_index() == 2,"setting selected index must not move focus");

    std::unique_ptr<ui::UiElement> removed = fixture.tabs->remove_tab(2);
    require(removed != nullptr && fixture.tabs->tab_count() == 2 && fixture.tabs->page_count() == 2,
        "removing a focused tab must preserve the tab/page invariant");
    require(fixture.tabs->focused_index() == 1 && fixture.tabs->selected_index() == 1,
        "removing focused tab should repair focus without corrupting selection");

    fixture.tabs->clear_tabs();
    require(fixture.tabs->tab_count() == 0 && fixture.tabs->page_count() == 0,
        "clearing tabs must leave no orphan pages or tabs");
    require(!fixture.tabs->focused_index() && !fixture.tabs->selected_index(),"clear should reset both indices");
    auto page = std::make_unique<ui::UiListContainer>(core::Rect{ 0,0,300,180 });
    page->add_back(std::make_unique<ui::UiButton>(core::Rect{ 0,0,120,32 }));
    require(fixture.tabs->add_tab(ui::ui_raw_text("Replacement"),std::move(page)).added,"tab should be re-addable after clear");
    require(fixture.tabs->selected_index() == 0 && !fixture.tabs->focused_index(),
        "re-adding after clear should select first page without synthesizing tab focus");
}

}

int main()
{
    test_tab_api_independence_and_mutation_repair();
    std::cout << "ui focus routing tests passed\n";
    std::_Exit(EXIT_SUCCESS);
}
