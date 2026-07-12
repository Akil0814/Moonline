#define SDL_MAIN_HANDLED

#include "../engine/ui/containers/ui_grid_container.h"
#include "../engine/ui/containers/ui_list_container.h"
#include "../engine/ui/containers/ui_scroll_container.h"
#include "../engine/ui/widgets/ui_button.h"
#include "../engine/ui/window/ui_window.h"

#include <cstdlib>
#include <iostream>
#include <memory>

namespace
{
void require(bool condition,const char* message)
{
    if (condition)
        return;
    std::cerr << "FAILED: " << message << '\n';
    std::exit(EXIT_FAILURE);
}

elysia::ui::UiInputEvent navigate(elysia::ui::UiAction action)
{
    return {
        .action = action,
        .type = elysia::ui::UiInputEventType::ActionPressed,
        .device = elysia::input::InputDevice::Gamepad
    };
}

void test_nested_boundary_restores_preferred_leaf()
{
    using namespace elysia;
    ui::UiListContainer outer(core::Rect{ 0,0,320,240 });
    auto grid = std::make_unique<ui::UiGridContainer>(core::Rect{ 0,0,300,120 });
    grid->set_column_count(2);
    auto* grid_raw = grid.get();
    auto first = std::make_unique<ui::UiButton>(core::Rect{ 0,0,100,40 });
    auto second = std::make_unique<ui::UiButton>(core::Rect{ 0,0,100,40 });
    auto* first_raw = first.get();
    auto* second_raw = second.get();
    grid->add_child(std::move(first));
    grid->add_child(std::move(second));
    auto trailing = std::make_unique<ui::UiButton>(core::Rect{ 0,0,100,40 });
    auto* trailing_raw = trailing.get();
    outer.add_back(std::move(grid));
    outer.add_back(std::move(trailing));
    outer.set_scope_focused(true);

    require(outer.focus_first_available(),"outer list should enter its nested grid");
    require(outer.focused_target() == first_raw,"nested grid should initially select its first leaf");
    require(outer.on_ui_input_event(navigate(ui::UiAction::NavigateRight)),"inner grid navigation should be handled");
    require(grid_raw->focused_target() == second_raw,"inner navigation should update the nested scope target");

    require(outer.on_ui_input_event(navigate(ui::UiAction::NavigateDown)),"boundary navigation should leave the nested region");
    require(outer.focused_target() == trailing_raw && trailing_raw->is_focused(),
        "outer list should enter its trailing direct-control region");
    require(!grid_raw->is_scope_focused(),"inactive nested scope should lose scope focus");

    require(outer.on_ui_input_event(navigate(ui::UiAction::NavigateUp)),"reverse boundary navigation should re-enter nested region");
    require(outer.focused_target() == second_raw && grid_raw->focused_target() == second_raw,
        "re-entering a nested region should restore its last valid leaf");
    require(second_raw->is_focused() && !first_raw->is_focused(),"nested leaf focus visuals should remain exclusive");
}

void test_window_repairs_focus_across_invalid_scope()
{
    using namespace elysia;
    ui::UiWindow window(core::Rect{ 0,0,640,240 });
    auto left = std::make_unique<ui::UiListContainer>(core::Rect{ 0,0,240,120 });
    auto right = std::make_unique<ui::UiListContainer>(core::Rect{ 300,0,240,120 });
    auto* left_raw = left.get();
    auto* right_raw = right.get();
    auto left_first = std::make_unique<ui::UiButton>(core::Rect{ 0,0,100,40 });
    auto left_second = std::make_unique<ui::UiButton>(core::Rect{ 0,0,100,40 });
    auto right_button = std::make_unique<ui::UiButton>(core::Rect{ 0,0,100,40 });
    auto* left_first_raw = left_first.get();
    auto* left_second_raw = left_second.get();
    auto* right_button_raw = right_button.get();
    left->add_back(std::move(left_first));
    left->add_back(std::move(left_second));
    right->add_back(std::move(right_button));
    window.add_child(std::move(left));
    window.add_child(std::move(right));
    window.register_focus_scope(*left_raw);
    window.register_focus_scope(*right_raw);

    require(window.focus_first_available_scope(),"window should focus the first registered usable scope");
    left_first_raw->set_visible(false);
    window.update(0.0);
    require(left_raw->focused_target() == left_second_raw && left_second_raw->is_focused(),
        "a scope should repair focus to its next usable direct child");

    left_second_raw->set_enabled(false);
    window.update(0.0);
    require(window.focused_scope() == right_raw,"window should fall back when its focused scope loses every target");
    require(right_raw->is_scope_focused() && right_button_raw->is_focused(),
        "fallback scope and leaf should receive focus together");
    require(!left_first_raw->is_focused() && !left_second_raw->is_focused(),
        "invalidated scope must not retain stale leaf visuals");
}

void test_scroll_scope_suppresses_and_restores_nested_focus()
{
    using namespace elysia;
    ui::UiScrollContainer scroll(core::Rect{ 0,0,200,80 });
    auto content = std::make_unique<ui::UiListContainer>(core::Rect{ 0,0,200,240 });
    auto button = std::make_unique<ui::UiButton>(core::Rect{ 0,0,100,40 });
    auto* button_raw = button.get();
    content->add_back(std::move(button));
    scroll.set_content(std::move(content));
    scroll.set_scope_focused(true);

    require(scroll.focus_first_available() && scroll.focused_target() == button_raw,
        "scroll scope should delegate initial focus to content");
    require(button_raw->is_focused(),"delegated content leaf should render focused");
    require(scroll.clear_focus_for_gamepad_scroll(),"scrolling should temporarily clear an existing nested focus target");
    require(scroll.focused_target() == nullptr && !button_raw->is_focused(),
        "temporary scroll focus suppression should clear leaf focus visuals");
    require(scroll.restore_focus_after_gamepad_scroll(),"scroll scope should restore focus after scrolling ends");
    require(scroll.focused_target() == button_raw && button_raw->is_focused(),
        "restoration should recover the previous valid nested focus target");
}
}

int main()
{
    test_nested_boundary_restores_preferred_leaf();
    test_window_repairs_focus_across_invalid_scope();
    test_scroll_scope_suppresses_and_restores_nested_focus();
    std::cout << "ui focus container tests passed\n";
    return EXIT_SUCCESS;
}
