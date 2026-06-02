#pragma once

#include "ui_selectable_scroll_list.h"
#include "../style/ui_style.h"
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

class UiOptionList : public UiSelectableScrollList
{
public:
    explicit UiOptionList(Vector2 position = Vector2::zero(), Vector2 size = Vector2::zero(), int order = 0);

    void on_input(const InputSnapshot& input) override;
    void reset() override;

    void set_items(const std::vector<UiOptionListItem>& items);
    int add_item(const UiOptionListItem& item);
    void clear_items();

    [[nodiscard]] size_t item_count() const;
    [[nodiscard]] const UiOptionListItem* selected_item() const;

    bool set_item_enabled(int index, bool enabled);
    bool set_item_toggle_value(int index, bool value);
    bool set_item_slider_value(int index, float value);

    void set_font_key(const std::string& font_key);
    [[nodiscard]] const std::string& font_key() const;

    void set_style(const UiOptionListStyle& style);
    [[nodiscard]] const UiOptionListStyle& style() const;

    void set_on_value_changed(UiOptionValueChangedCallback on_value_changed);
    [[nodiscard]] bool handle_focused_input_event(const InputEvent& event) override;

private:
    struct RowWidgets
    {
        std::shared_ptr<UiPanel> _panel;
        std::shared_ptr<UiLabel> _label;
        std::shared_ptr<GameObject> _control_object;
        std::shared_ptr<UiFocusable> _control;
        std::shared_ptr<UiSlider> _slider;
        std::shared_ptr<UiToggle> _toggle;
    };

private:
    void apply_layout_metrics();
    void refresh_rows();
    void refresh_row(size_t index);
    void rebuild_rows();
    void sync_selection_state() override;
    void emit_value_changed(int index);
    [[nodiscard]] std::string value_text(const UiOptionListItem& item) const;
    [[nodiscard]] size_t selectable_item_count() const override;
    [[nodiscard]] bool selectable_item_enabled(int index) const override;
    [[nodiscard]] const GameObject* selected_item_view_target() const override;
    void apply_theme(const UiTheme& theme) override;

private:
    std::vector<UiOptionListItem> _items;
    std::vector<RowWidgets> _rows;

    std::string _font_key = "ui.default";
    UiOptionListStyle _style;
    UiOptionValueChangedCallback _on_value_changed;
    bool _was_mouse_down = false;
};
