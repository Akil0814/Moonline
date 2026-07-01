#include "ui_container_test_scene.h"

#include "../../engine/ui/window/ui_window.h"
#include "../../engine/ui/containers/ui_grid_container.h"
#include "../../engine/ui/containers/ui_list_container.h"
#include "../../engine/ui/containers/ui_panel.h"
#include "../../engine/ui/containers/ui_scroll_container.h"
#include "../../engine/ui/widgets/ui_button.h"
#include "../../engine/ui/layout/ui_layout_types.h"

#include <iostream>
#include <memory>
#include <utility>

namespace arcneco::scene
{
namespace
{
[[nodiscard]] elysia::ui::UiButtonConfig make_button_config(const char* text_key)
{
    return elysia::ui::UiButtonConfig{ .content = elysia::ui::UiButtonTextContent{ text_key } };
}

[[nodiscard]] elysia::ui::UiLayoutChildOptions make_window_child_options(float left,float top)
{
    return elysia::ui::UiLayoutChildOptions{
        ._anchor = elysia::ui::UiLayoutAnchor::TopLeft,
        ._margin = elysia::ui::UiLayoutMargin{ left,top,0.0f,0.0f },
        ._cross_align = elysia::ui::UiLayoutAlign::Start,
        ._size_override = elysia::core::Vector2(0.0f,0.0f),
        ._use_custom_cross_align = false,
        ._fill_cross_axis = false,
        ._use_size_override = false
    };
}

std::unique_ptr<elysia::ui::UiButton> make_button(const char* text_key,int index,const char* scope)
{
    auto button = std::make_unique<elysia::ui::UiButton>(
        elysia::core::Rect{ 0,0,180,44 },
        make_button_config(text_key),
        0);
    button->set_on_click([index,scope]()
    {
        std::cout << scope << " button " << index << std::endl;
    });
    return button;
}
}

void UiContainerTestScene::on_enter(const elysia::scene::ScenePayload& payload)
{
    (void)payload;
    _paused = false;
    rebuild_ui();
}

void UiContainerTestScene::on_input(
    const elysia::input::RawInputFrame& input,
    const std::vector<elysia::input::RawInputEvent>& events)
{
    ApplicationScene::on_input(input,events);
}

void UiContainerTestScene::on_exit()
{
    _paused = false;
    clear_ui();
}

void UiContainerTestScene::reset()
{
    _paused = false;
    clear_ui();
}

void UiContainerTestScene::rebuild_ui()
{
    clear_ui();

    _root_window = Scene::create_and_add_object<elysia::ui::UiWindow>(elysia::core::Rect{ 120,80,1040,560 },100);
    _root_window->set_draw_background(true);
    _root_window->set_draw_border(true);
    _root_window->set_padding(elysia::ui::UiLayoutPadding{ 24.0f,24.0f,24.0f,24.0f });
    _root_window->set_on_cancel([this]()
    {
        request_back_to_menu();
    });

    auto* vertical_scroll = _root_window->create_child<elysia::ui::UiScrollContainer>(elysia::core::Rect{ 0,0,260,420 });
    vertical_scroll->set_scroll_axis(elysia::ui::UiScrollAxis::Auto);
    vertical_scroll->set_scrollbar_visibility(elysia::ui::UiScrollBarVisibility::Auto);
    vertical_scroll->set_scroll_step(elysia::core::Vector2(32.0f,32.0f));
    vertical_scroll->set_content_size(elysia::core::Vector2(260.0f,760.0f));

    auto vertical_list = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,260,760 });
    vertical_list->set_padding(elysia::ui::UiLayoutPadding{ 20.0f,20.0f,20.0f,20.0f });
    vertical_list->set_item_spacing(18.0f);
    for (int index = 0; index < 10; ++index)
        vertical_list->add_back(make_button("menu_scene.ui_button",index,"vertical"));
    vertical_scroll->set_content(std::move(vertical_list));

    auto* horizontal_scroll = _root_window->create_child<elysia::ui::UiScrollContainer>(elysia::core::Rect{ 0,0,320,180 });
    horizontal_scroll->set_scroll_axis(elysia::ui::UiScrollAxis::Auto);
    horizontal_scroll->set_scrollbar_visibility(elysia::ui::UiScrollBarVisibility::Auto);
    horizontal_scroll->set_scroll_step(elysia::core::Vector2(36.0f,36.0f));
    horizontal_scroll->set_content_size(elysia::core::Vector2(760.0f,180.0f));

    auto horizontal_panel = std::make_unique<elysia::ui::UiPanel>(elysia::core::Rect{ 0,0,760,180 });
    horizontal_panel->set_draw_background(true);
    horizontal_panel->set_draw_border(true);
    horizontal_panel->set_background_color(elysia::core::colors::cobalt_blue);

    auto horizontal_button_0 = std::make_unique<elysia::ui::UiButton>(
        elysia::core::Rect{ 70,68,120,44 },
        make_button_config("menu_scene.ui_button"),
        0);
    horizontal_button_0->set_on_click([]()
    {
        std::cout << "horizontal button 0" << std::endl;
    });
    horizontal_panel->add_child(std::move(horizontal_button_0),elysia::ui::UiPanelInsertDirection::Down);

    for (int index = 1; index < 5; ++index)
    {
        auto button = std::make_unique<elysia::ui::UiButton>(
            elysia::core::Rect{ 70.0f + 140.0f * static_cast<float>(index),68.0f,120.0f,44.0f },
            make_button_config("menu_scene.ui_button"),
            0);
        button->set_on_click([index]()
        {
            std::cout << "horizontal button " << index << std::endl;
        });
        horizontal_panel->add_child(std::move(button),elysia::ui::UiPanelInsertDirection::Right);
    }
    horizontal_scroll->set_content(std::move(horizontal_panel));

    auto* grid_scroll = _root_window->create_child<elysia::ui::UiScrollContainer>(elysia::core::Rect{ 0,0,340,260 });
    grid_scroll->set_scroll_axis(elysia::ui::UiScrollAxis::Auto);
    grid_scroll->set_scrollbar_visibility(elysia::ui::UiScrollBarVisibility::Auto);
    grid_scroll->set_scroll_step(elysia::core::Vector2(28.0f,28.0f));
    grid_scroll->set_content_size(elysia::core::Vector2(520.0f,360.0f));

    auto grid_content = std::make_unique<elysia::ui::UiGridContainer>(elysia::core::Rect{ 0,0,520,360 });
    grid_content->set_padding(elysia::ui::UiLayoutPadding{ 18.0f,18.0f,18.0f,18.0f });
    grid_content->set_column_count(4);
    grid_content->set_cell_spacing(elysia::core::Vector2(16.0f,16.0f));
    for (int index = 0; index < 20; ++index)
    {
        auto button = std::make_unique<elysia::ui::UiButton>(
            elysia::core::Rect{ 0,0,96,48 },
            make_button_config("menu_scene.ui_button"),
            0);
        button->set_on_click([index]()
        {
            std::cout << "grid button " << index << std::endl;
        });
        grid_content->add_child(std::move(button));
    }
    grid_scroll->set_content(std::move(grid_content));

    auto* hidden_scroll = _root_window->create_child<elysia::ui::UiScrollContainer>(elysia::core::Rect{ 0,0,320,180 });
    hidden_scroll->set_scroll_axis(elysia::ui::UiScrollAxis::Auto);
    hidden_scroll->set_scrollbar_visibility(elysia::ui::UiScrollBarVisibility::Hidden);
    hidden_scroll->set_scroll_step(elysia::core::Vector2(30.0f,30.0f));
    hidden_scroll->set_content_size(elysia::core::Vector2(320.0f,360.0f));

    auto first_hidden_content = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,320,360 });
    first_hidden_content->set_padding(elysia::ui::UiLayoutPadding{ 18.0f,18.0f,18.0f,18.0f });
    first_hidden_content->set_item_spacing(14.0f);
    for (int index = 0; index < 4; ++index)
        first_hidden_content->add_back(make_button("menu_scene.ui_button",index,"hidden-initial"));
    hidden_scroll->set_content(std::move(first_hidden_content));

    auto replacement_hidden_content = std::make_unique<elysia::ui::UiListContainer>(elysia::core::Rect{ 0,0,320,520 });
    replacement_hidden_content->set_padding(elysia::ui::UiLayoutPadding{ 18.0f,18.0f,18.0f,18.0f });
    replacement_hidden_content->set_item_spacing(14.0f);
    for (int index = 0; index < 8; ++index)
        replacement_hidden_content->add_back(make_button("menu_scene.ui_button",index,"hidden"));
    hidden_scroll->set_content(std::move(replacement_hidden_content));
    hidden_scroll->set_content_size(elysia::core::Vector2(320.0f,520.0f));

    _root_window->set_child_layout_options(0,make_window_child_options(0.0f,0.0f));
    _root_window->set_child_layout_options(1,make_window_child_options(288.0f,0.0f));
    _root_window->set_child_layout_options(2,make_window_child_options(624.0f,0.0f));
    _root_window->set_child_layout_options(3,make_window_child_options(288.0f,220.0f));

    _root_window->register_focus_scope(*vertical_scroll,elysia::ui::UiFocusScopeNeighbors{ nullptr,hidden_scroll,nullptr,horizontal_scroll });
    _root_window->register_focus_scope(*horizontal_scroll,elysia::ui::UiFocusScopeNeighbors{ nullptr,hidden_scroll,vertical_scroll,grid_scroll });
    _root_window->register_focus_scope(*grid_scroll,elysia::ui::UiFocusScopeNeighbors{ nullptr,nullptr,horizontal_scroll,nullptr });
    _root_window->register_focus_scope(*hidden_scroll,elysia::ui::UiFocusScopeNeighbors{ vertical_scroll,nullptr,nullptr,nullptr });
    _root_window->focus_first_available_scope();
}

void UiContainerTestScene::clear_ui()
{
    if (_root_window)
    {
        _root_window->destroy();
        _root_window = nullptr;
    }
}

void UiContainerTestScene::request_back_to_menu()
{
    std::cout << "ui container test back" << std::endl;
}
}
