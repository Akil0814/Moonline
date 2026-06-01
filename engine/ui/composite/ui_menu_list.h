#pragma once

#include "../containers/ui_scroll_panel.h"
#include "../base/ui_focusable.h"
#include "../style/ui_style.h"
#include "../widgets/ui_text_button.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

struct UiMenuListItem
{
    std::string _id;
    std::string _text;
    bool _enabled = true;
};

using UiMenuSelectCallback = std::function<void(
    int index,
    const std::string& id,
    const std::string& text
)>;

class UiMenuList : public UiScrollPanel, public UiFocusable
{
public:
    explicit UiMenuList(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void on_input_event(const InputEvent& event) override;
    void reset() override;

    int add_item(const std::string& id, const std::string& text, bool enabled = true);
    void clear_items();
    void set_items(const std::vector<UiMenuListItem>& items);

    [[nodiscard]] size_t item_count() const;
    [[nodiscard]] int selected_index() const;
    void set_selected_index(int index);
    [[nodiscard]] const UiMenuListItem* selected_item() const;

    bool set_item_enabled(int index, bool enabled);
    bool set_item_text(int index, const std::string& text);

    void set_item_size(const Vector2& item_size);
    [[nodiscard]] const Vector2& item_size() const;

    void set_font_key(const std::string& font_key);
    [[nodiscard]] const std::string& font_key() const;

    void set_text_color(SDL_Color color);
    [[nodiscard]] SDL_Color text_color() const;

    void set_button_style(const ButtonStyle& button_style);
    [[nodiscard]] const ButtonStyle& button_style() const;

    void set_on_select(UiMenuSelectCallback on_select);
    void set_enabled(bool enabled) override;
    [[nodiscard]] bool is_enabled() const override;
    void set_focused(bool focused) override;
    [[nodiscard]] bool is_focused() const override;
    [[nodiscard]] bool handle_focused_input_event(const InputEvent& event) override;
    [[nodiscard]] GameObject* game_object() override;
    [[nodiscard]] const GameObject* game_object() const override;

private:
    void rebuild_items();
    void sync_selection_visuals();
    void handle_item_click(UiTextButton* button);
    void apply_theme(const UiTheme& theme) override;

private:
    std::vector<UiMenuListItem> _items;
    std::vector<std::shared_ptr<UiTextButton>> _buttons;

    Vector2 _item_size{ 320.0f, 56.0f };
    std::string _font_key = "ui.default";
    SDL_Color _text_color{ 255, 255, 255, 255 };
    ButtonStyle _button_style;

    UiMenuSelectCallback _on_select;

    int _selected_index = -1;
    bool _enabled = true;
    bool _is_focused = false;
};
