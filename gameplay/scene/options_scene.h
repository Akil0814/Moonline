#pragma once

#include "../../engine/core/scene/scene.h"
#include "../../engine/ui/composite/ui_option_list.h"
#include "../../engine/ui/containers/ui_screen.h"
#include "../../engine/ui/widgets/label.h"
#include "../../engine/ui/widgets/ui_scroll_bar.h"

#include <memory>
#include <string>

class OptionsScene : public Scene
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
    void rebuild_options();
    void on_option_value_changed(
        int row_index,
        const std::string& id,
        const std::string& value
    );
    void return_to_main_menu();

private:
    std::shared_ptr<UiScreen> _screen;
    std::shared_ptr<Label> _title_label;
    std::shared_ptr<Label> _subtitle_label;
    std::shared_ptr<Label> _footer_label;
    std::shared_ptr<UiOptionList> _option_list;
    std::shared_ptr<UiScrollBar> _scroll_bar;
};
