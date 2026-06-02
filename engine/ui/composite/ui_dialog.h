#pragma once

#include "../containers/ui_screen.h"
#include "../widgets/ui_label.h"
#include "ui_menu_list.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

struct UiDialogAction
{
    std::string _id;
    std::string _text;
};

using UiDialogActionCallback = std::function<void(
    const std::string& id,
    const std::string& text
)>;

class UiDialog : public UiScreen
{
public:
    explicit UiDialog(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void on_update(double delta) override;
    void on_input_event(const InputEvent& event) override;
    void reset() override;

    void set_title(const std::string& title);
    [[nodiscard]] const std::string& title() const;

    void set_message(const std::string& message);
    [[nodiscard]] const std::string& message() const;

    void set_actions(const std::vector<UiDialogAction>& actions);
    void set_cancel_action_id(const std::string& cancel_action_id);
    [[nodiscard]] const std::string& cancel_action_id() const;

    void set_on_action(UiDialogActionCallback on_action);
    void show_dialog();
    void hide_dialog();

private:
    void ensure_controls();
    void configure_controls();
    void rebuild_layout();
    void refresh_content();
    void refresh_action_items();
    void handle_action(
        int index,
        const std::string& id,
        const std::string& text
    );

private:
    std::string _title;
    std::string _message;
    std::string _cancel_action_id = "cancel";
    std::vector<UiDialogAction> _actions;

    UiDialogActionCallback _on_action;

    std::shared_ptr<UiLabel> _title_label;
    std::shared_ptr<UiLabel> _message_label;
    std::shared_ptr<UiMenuList> _action_list;
    bool _suppress_input_until_next_update = false;
};
