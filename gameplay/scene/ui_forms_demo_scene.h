#pragma once

#include "../../engine/core/scene/scene.h"
#include "../../engine/ui/containers/ui_screen.h"
#include "../../engine/ui/layout/ui_grid_layout.h"
#include "../../engine/ui/widgets/ui_label.h"
#include "../../engine/ui/widgets/ui_text_button.h"
#include "../../engine/ui/widgets/ui_text_input.h"
#include "../../engine/ui/widgets/ui_slider.h"
#include "../../engine/ui/widgets/ui_toggle.h"

#include <memory>

class UiFormsDemoScene : public Scene
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
    void rebuild_form();
    void return_to_main_menu();
    void set_status_text(const std::string& text);

private:
    std::shared_ptr<UiScreen> _screen;
    std::shared_ptr<UiLabel> _title_label;
    std::shared_ptr<UiLabel> _subtitle_label;
    std::shared_ptr<UiLabel> _footer_label;
    std::shared_ptr<UiGridLayout> _form_grid;
    std::shared_ptr<UiLabel> _name_label;
    std::shared_ptr<UiTextInput> _name_input;
    std::shared_ptr<UiLabel> _password_label;
    std::shared_ptr<UiTextInput> _password_input;
    std::shared_ptr<UiLabel> _toggle_label;
    std::shared_ptr<UiToggle> _toggle;
    std::shared_ptr<UiLabel> _slider_label;
    std::shared_ptr<UiSlider> _slider;
    std::shared_ptr<UiTextButton> _back_button;
};
