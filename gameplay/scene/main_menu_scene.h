#pragma once

#include "../../engine/core/scene/scene.h"
#include "../../engine/ui/composite/ui_menu_list.h"
#include "../../engine/ui/composite/ui_dialog.h"
#include "../../engine/ui/containers/ui_screen.h"
#include "../../engine/ui/widgets/ui_label.h"
#include "../../engine/ui/widgets/ui_scroll_bar.h"

#include <memory>

class MainMenuScene : public Scene
{
public:
    void on_enter() override;
    void on_exit() override;

    void on_update(double delta) override;
    void on_render(SDL_Renderer* renderer) override;
    void on_input(
        const InputSnapshot& input,
        const std::vector<InputEvent>& events
    ) override;

    void reset() override;

private:
    void ensure_ui();
    void rebuild_menu_items();
    void select_menu_item(
        int index,
        const std::string& id,
        const std::string& text
    );
    void request_quit();
    void open_quit_dialog();
    void handle_quit_dialog_action(const std::string& id, const std::string& text);

private:
    std::shared_ptr<UiScreen> _screen;
    std::shared_ptr<UiLabel> _title_label;
    std::shared_ptr<UiLabel> _subtitle_label;
    std::shared_ptr<UiLabel> _footer_label;
    std::shared_ptr<UiMenuList> _menu_list;
    std::shared_ptr<UiScrollBar> _menu_scroll_bar;
    std::shared_ptr<UiDialog> _quit_dialog;
};
