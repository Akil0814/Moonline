#pragma once

#include "../../scene/scene.h"
#include "../../tools/timer.h"
#include "../../ui/style/ui_theme_manager.h"

#include "testbed_scene_payload.h"

#include <string>
#include <vector>

namespace elysia::ui
{
class UiWindow;
class UiListContainer;
}

namespace elysia::testbed
{
class ElysiaScene final : public elysia::scene::Scene
{

public:
    void on_input(const elysia::input::RawInputFrame& input,
        const std::vector<elysia::input::RawInputEvent>& events) override;
    void on_enter(const elysia::scene::ScenePayload& payload) override;
    void on_update(double delta)override;
    void on_exit() override;
    void reset() override;

private:
    std::string get_next_line();
    void add_lable(std::string code_line);
    void build_ui();
    void destroy_ui() noexcept;
    void return_to_caller();

private:
    bool _finish_logo = false;
    bool _finish_code = false;
    bool _finish_entering = false;

    size_t _current_line = 0;
    std::vector<std::string> _text_list;

    elysia::tools::Timer _code_timer;

    elysia::scene::SceneRoute _return_route;
    elysia::ui::UiWindow* _root_window = nullptr;
    elysia::ui::UiListContainer* _code_list = nullptr;
    elysia::ui::UiThemeManager _elysia_theme;
};
}
