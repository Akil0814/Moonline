#pragma once

#include "../containers/ui_scroll_panel.h"
#include "../style/ui_style.h"
#include "../base/ui_focusable.h"
#include "../widgets/ui_label.h"
#include "../widgets/ui_slider.h"
#include "../widgets/ui_toggle.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

enum class UiOptionControlType
{
    Toggle,
    Slider
};

struct UiOptionToggleValue
{
    bool _value = false;
    std::string _off_text = "Off";
    std::string _on_text = "On";
};

struct UiOptionSliderValue
{
    float _min_value = 0.0f;
    float _max_value = 100.0f;
    float _value = 0.0f;
    float _step = 1.0f;
    int _display_precision = 0;
    std::string _suffix;
};

struct UiOptionListItem
{
    std::string _id;
    std::string _label;
    UiOptionControlType _control_type = UiOptionControlType::Toggle;
    UiOptionToggleValue _toggle;
    UiOptionSliderValue _slider;
    bool _enabled = true;
};

struct UiOptionListStyle
{
    Vector2 _row_size{ 620.0f, 72.0f };
    Vector2 _control_size{ 220.0f, 40.0f };
    float _row_spacing = 12.0f;
    float _panel_padding = 12.0f;

    SDL_Color _row_background_color{ 30, 38, 54, 220 };
    SDL_Color _row_selected_background_color{ 52, 78, 116, 235 };
    SDL_Color _row_disabled_background_color{ 22, 26, 34, 180 };
    SDL_Color _row_border_color{ 104, 132, 172, 255 };

    SDL_Color _label_text_color{ 244, 244, 248, 255 };
    SDL_Color _disabled_text_color{ 132, 140, 152, 255 };

    ToggleStyle _toggle_style;
    SliderStyle _slider_style;
};

using UiOptionValueChangedCallback = std::function<void(
    int row_index,
    const std::string& id,
    const std::string& value
)>;

class UiOptionList : public UiScrollPanel, public UiFocusable
{
public:
    explicit UiOptionList(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void on_input(const InputSnapshot& input) override;
    void on_input_event(const InputEvent& event) override;
    void reset() override;

    void set_items(const std::vector<UiOptionListItem>& items);
    int add_item(const UiOptionListItem& item);
    void clear_items();

    [[nodiscard]] size_t item_count() const;
    [[nodiscard]] int selected_row_index() const;
    void set_selected_row_index(int index);
    [[nodiscard]] const UiOptionListItem* selected_item() const;

    bool set_item_enabled(int index, bool enabled);
    bool set_item_toggle_value(int index, bool value);
    bool set_item_slider_value(int index, float value);

    void set_font_key(const std::string& font_key);
    [[nodiscard]] const std::string& font_key() const;

    void set_style(const UiOptionListStyle& style);
    [[nodiscard]] const UiOptionListStyle& style() const;

    void set_on_value_changed(UiOptionValueChangedCallback on_value_changed);

    void set_enabled(bool enabled) override;
    [[nodiscard]] bool is_enabled() const override;
    void set_focused(bool focused) override;
    [[nodiscard]] bool is_focused() const override;
    [[nodiscard]] bool handle_focused_input_event(const InputEvent& event) override;
    [[nodiscard]] GameObject* game_object() override;
    [[nodiscard]] const GameObject* game_object() const override;

private:
    struct RowWidgets
    {
        std::shared_ptr<UiPanel> _panel;
        std::shared_ptr<UiLabel> _label;
        std::shared_ptr<GameObject> _control_object;
        std::shared_ptr<UiFocusable> _control;
    };

private:
    void rebuild_rows();
    void sync_row_visuals();
    void sync_row_focus();
    void emit_value_changed(int index);
    void handle_row_click(UiFocusable* control);
    [[nodiscard]] std::string value_text(const UiOptionListItem& item) const;
    void apply_theme(const UiTheme& theme) override;

private:
    std::vector<UiOptionListItem> _items;
    std::vector<RowWidgets> _rows;

    std::string _font_key = "ui.default";
    UiOptionListStyle _style;
    UiOptionValueChangedCallback _on_value_changed;

    int _selected_row_index = -1;
    bool _enabled = true;
    bool _is_focused = false;
    bool _was_mouse_down = false;
};
