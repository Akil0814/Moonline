#pragma once

#include "ui_selectable_scroll_list.h"
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

using UiMenuSelectionChangedCallback = std::function<void(
    int index,
    const std::string& id,
    const std::string& text
)>;

class UiMenuList : public UiSelectableScrollList
{
public:
    explicit UiMenuList(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void reset() override;

    int add_item(const std::string& id, const std::string& text, bool enabled = true);
    void clear_items();
    void set_items(const std::vector<UiMenuListItem>& items);

    [[nodiscard]] size_t item_count() const;
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

    void set_on_selection_changed(UiMenuSelectionChangedCallback on_selection_changed);
    [[nodiscard]] bool handle_focused_input_event(const InputEvent& event) override;

private:
    void apply_layout_metrics();
    void refresh_items();
    void refresh_item(size_t index);
    void rebuild_items();
    void sync_selection_state() override;
    void handle_item_click(UiTextButton* button);
    [[nodiscard]] size_t selectable_item_count() const override;
    [[nodiscard]] bool selectable_item_enabled(int index) const override;
    [[nodiscard]] const GameObject* selected_item_view_target() const override;
    void apply_theme(const UiTheme& theme) override;

private:
    std::vector<UiMenuListItem> _items;
    std::vector<std::shared_ptr<UiTextButton>> _buttons;

    Vector2 _item_size{ 320.0f, 56.0f };
    std::string _font_key = "ui.default";
    SDL_Color _text_color{ 255, 255, 255, 255 };
    ButtonStyle _button_style;

    UiMenuSelectionChangedCallback _on_selection_changed;
};
