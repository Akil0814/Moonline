#pragma once

#include "../containers/ui_panel.h"
#include "../base/ui_focusable.h"
#include "../style/ui_style.h"
#include "../widgets/ui_button_group.h"
#include "../widgets/ui_selectable_button.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

struct UiTabBarItem
{
    std::string _id;
    std::string _text;
    bool _enabled = true;
};

using UiTabBarSelectionChangedCallback = std::function<void(
    int index,
    const std::string& id,
    const std::string& text
)>;

class UiTabBar : public UiPanel, public UiFocusable
{
public:
    explicit UiTabBar(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void reset() override;

    int add_tab(const std::string& id, const std::string& text, bool enabled = true);
    void set_tabs(const std::vector<UiTabBarItem>& items);
    void clear_tabs();

    [[nodiscard]] size_t tab_count() const;
    [[nodiscard]] int selected_index() const;
    void set_selected_index(int index);
    [[nodiscard]] const UiTabBarItem* selected_tab() const;

    void set_tab_size(const Vector2& tab_size);
    [[nodiscard]] const Vector2& tab_size() const;

    void set_font_key(const std::string& font_key);
    [[nodiscard]] const std::string& font_key() const;

    void set_button_theme_role(UiButtonThemeRole button_theme_role);
    [[nodiscard]] UiButtonThemeRole button_theme_role() const;

    void set_button_style(const ButtonStyle& button_style);
    [[nodiscard]] const ButtonStyle& button_style() const;
    void clear_button_style_override();
    [[nodiscard]] bool has_button_style_override() const;

    void set_on_selection_changed(UiTabBarSelectionChangedCallback on_selection_changed);

    void set_enabled(bool enabled) override;
    [[nodiscard]] bool is_enabled() const override;
    void set_focused(bool focused) override;
    [[nodiscard]] bool is_focused() const override;
    [[nodiscard]] bool handle_focused_input_event(const InputEvent& event) override;
    [[nodiscard]] GameObject* game_object() override;
    [[nodiscard]] const GameObject* game_object() const override;

private:
    void rebuild_tabs();
    void sync_button_focus();
    void handle_group_selection_changed(int index, UiSelectableButton* button);
    void emit_selection_changed();

private:
    std::vector<UiTabBarItem> _items;
    std::vector<std::shared_ptr<UiSelectableButton>> _buttons;
    UiButtonGroup _button_group;
    UiTabBarSelectionChangedCallback _on_selection_changed;

    Vector2 _tab_size{ 160.0f, 52.0f };
    std::string _font_key = "ui.default";
    ButtonStyle _button_style;
    UiButtonThemeRole _button_theme_role = UiButtonThemeRole::Primary;

    int _selected_index = -1;
    bool _enabled = true;
    bool _is_focused = false;
    bool _has_button_style_override = false;
};
