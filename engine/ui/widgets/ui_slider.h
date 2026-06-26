#pragma once
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include "../core/ui_control.h"
#include "label/ui_label.h"
#include "number/ui_number.h"
#include "ui_bar.h"
#include "ui_drag_handle.h"

struct SDL_Texture;

namespace elysia::ui
{
    enum class UiSliderLabelPlacement { None, Left, Right, Above, Below, Center };
    enum class UiSliderOrientation { Horizontal, Vertical };
    enum class UiSliderValueLabelMode { None, Value, Percent };

    struct UiSliderSounds
    {
        std::string on_focus{};
        std::string on_slide{};
        std::string on_settle{};
        double min_slide_sound_interval = 0.05;
    };

    struct UiSliderTextContent { std::string text_key{}; };
    struct UiSliderIconContent { SDL_Texture* texture = nullptr; };

    using UiSliderLabelContent = std::variant<std::monostate,UiSliderTextContent,UiSliderIconContent>;

    struct UiSliderConfig
    {
        UiSliderLabelContent label_content{};
        UiSliderLabelPlacement label_placement = UiSliderLabelPlacement::None;
        UiSliderOrientation orientation = UiSliderOrientation::Horizontal;
        UiSliderValueLabelMode value_label_mode = UiSliderValueLabelMode::None;
        std::optional<UiSliderSounds> slider_sound;
        float min_value = 0.0f;
        float max_value = 1.0f;
        float value = 0.0f;
        std::optional<float> step = std::nullopt;
        bool draw_background = true;
        bool draw_border = true;
        UiDragHandleConfig handle{};
        float bar_thickness = 6.0f;
        int value_decimal_places = 0;
        bool value_trim_trailing_zeros = true;
        bool value_keep_decimal_point = false;
        float value_digit_spacing = 0.0f;
        std::optional<float> value_fixed_glyph_advance = std::nullopt;
        std::optional<float> value_target_height = std::nullopt;
    };

    using UiSliderValueChangedCallback = std::function<void(float value)>;

    class UiSlider : public UiControl
    {
    public:
        explicit UiSlider(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
        UiSlider(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
        UiSlider(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
        UiSlider(const elysia::core::Rect& rect,const UiSliderConfig& config,int order = 0) noexcept;
        UiSlider(const elysia::core::Vector2& position,const elysia::core::Vector2& size,const UiSliderConfig& config,int order = 0) noexcept;
        UiSlider(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,const UiSliderConfig& config,int order = 0) noexcept;
        ~UiSlider() override = default;

        void reset() noexcept override;
        void set_enabled(bool enabled);
        void set_focused(bool focused);

        bool on_ui_input_event(const UiInputEvent& event) override;
        void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

        void set_slider_config(const UiSliderConfig& config);
        void set_range(float min_value,float max_value);
        void set_value(float value);
        void set_ratio(float ratio);
        [[nodiscard]] float min_value() const noexcept;
        [[nodiscard]] float max_value() const noexcept;
        [[nodiscard]] float value() const noexcept;
        [[nodiscard]] float ratio() const noexcept;
        void set_step(std::optional<float> step);
        [[nodiscard]] const std::optional<float>& step() const noexcept;

        void set_label_content(const UiSliderLabelContent& content);
        void clear_label_content() noexcept;
        void set_text_key(std::string text_key);
        [[nodiscard]] const std::string& text_key() const noexcept;
        void set_icon_texture(SDL_Texture* texture) noexcept;

        void set_label_placement(UiSliderLabelPlacement placement) noexcept;
        [[nodiscard]] UiSliderLabelPlacement label_placement() const noexcept;
        void set_orientation(UiSliderOrientation orientation) noexcept;
        [[nodiscard]] UiSliderOrientation orientation() const noexcept;
        void set_value_label_mode(UiSliderValueLabelMode mode) noexcept;
        [[nodiscard]] UiSliderValueLabelMode value_label_mode() const noexcept;

        void set_sounds(const UiSliderSounds& sounds);
        void clear_sounds() noexcept;
        [[nodiscard]] const std::optional<UiSliderSounds>& sounds() const noexcept;
        void set_on_value_changed(UiSliderValueChangedCallback on_value_changed);

        void set_background_color(elysia::core::Color color) noexcept;
        [[nodiscard]] elysia::core::Color background_color() const noexcept;
        void set_disabled_background_color(elysia::core::Color color) noexcept;
        [[nodiscard]] elysia::core::Color disabled_background_color() const noexcept;
        void set_border_color(elysia::core::Color color) noexcept;
        [[nodiscard]] elysia::core::Color border_color() const noexcept;
        void set_disabled_border_color(elysia::core::Color color) noexcept;
        [[nodiscard]] elysia::core::Color disabled_border_color() const noexcept;
        void set_fill_color(elysia::core::Color color) noexcept;
        [[nodiscard]] elysia::core::Color fill_color() const noexcept;
        void set_disabled_fill_color(elysia::core::Color color) noexcept;
        [[nodiscard]] elysia::core::Color disabled_fill_color() const noexcept;
        void set_handle_config(const UiDragHandleConfig& config);
        [[nodiscard]] const UiDragHandleConfig& handle_config() const noexcept;
        void set_text_color(elysia::core::Color color) noexcept;
        [[nodiscard]] elysia::core::Color text_color() const noexcept;
        void set_disabled_text_color(elysia::core::Color color) noexcept;
        [[nodiscard]] elysia::core::Color disabled_text_color() const noexcept;

        void set_value_decimal_places(int decimal_places);
        [[nodiscard]] int value_decimal_places() const noexcept;
        void set_value_trim_trailing_zeros(bool trim_trailing_zeros);
        [[nodiscard]] bool value_trims_trailing_zeros() const noexcept;
        void set_value_keep_decimal_point(bool keep_decimal_point);
        [[nodiscard]] bool value_keeps_decimal_point() const noexcept;
        void set_value_digit_spacing(float spacing);
        [[nodiscard]] float value_digit_spacing() const noexcept;
        void set_value_fixed_glyph_advance(float advance);
        [[nodiscard]] std::optional<float> value_fixed_glyph_advance() const noexcept;
        void clear_value_fixed_glyph_advance();
        void set_value_target_height(float height);
        [[nodiscard]] std::optional<float> value_target_height() const noexcept;
        void clear_value_target_height();

        void set_bar_thickness(float thickness) noexcept;
        [[nodiscard]] float bar_thickness() const noexcept;
        void set_draw_background(bool draw_background) noexcept;
        [[nodiscard]] bool draws_background() const noexcept;
        void set_draw_border(bool draw_border) noexcept;
        [[nodiscard]] bool draws_border() const noexcept;

    private:
        struct SliderLayout;
        void apply_slider_config(const UiSliderConfig& config);
        void apply_label_content(const UiSliderLabelContent& content);
        void sync_child_widgets(const SliderLayout& layout) const;
        void sync_bar_widget(const SliderLayout& layout) const;
        void sync_handle_widget(const SliderLayout& layout) const;
        void sync_label_widget(const SliderLayout& layout) const;
        void sync_value_number_widget(const SliderLayout& layout) const;
        [[nodiscard]] SliderLayout compute_layout() const noexcept;
        [[nodiscard]] float clamped_ratio() const noexcept;
        [[nodiscard]] float snapped_value(float value) const noexcept;
        [[nodiscard]] float action_step() const noexcept;
        [[nodiscard]] float pointer_ratio(const SliderLayout& layout,int mouse_x,int mouse_y) const noexcept;
        [[nodiscard]] float handle_ratio(const SliderLayout& layout,const elysia::core::Vector2& center) const noexcept;
        [[nodiscard]] elysia::core::Rect handle_drag_bounds(const SliderLayout& layout) const noexcept;
        [[nodiscard]] bool can_interact() const noexcept;
        [[nodiscard]] bool can_receive_pointer() const noexcept;
        [[nodiscard]] bool contains_pointer(int mouse_x,int mouse_y) const noexcept;
        [[nodiscard]] bool contains_track_or_handle(const SliderLayout& layout,int mouse_x,int mouse_y) const noexcept;
        [[nodiscard]] bool is_primary_pointer_event(const UiInputEvent& event) const noexcept;
        [[nodiscard]] bool set_value_internal(float value,bool notify) noexcept;
        [[nodiscard]] bool update_value_from_pointer(int mouse_x,int mouse_y) noexcept;
        [[nodiscard]] elysia::core::Rect fitted_texture_rect(const elysia::core::Rect& bounds,SDL_Texture* texture) const noexcept;
        [[nodiscard]] elysia::core::Color current_background_color() const noexcept;
        [[nodiscard]] elysia::core::Color current_border_color() const noexcept;
        [[nodiscard]] elysia::core::Color current_fill_color() const noexcept;
        [[nodiscard]] elysia::core::Color current_text_color() const noexcept;
        void bind_handle_callbacks();
        void clear_drag_state() noexcept;
        void play_sound_if_set(std::string_view sound_key) const;
        void play_slide_sound_if_allowed();

    private:
        mutable UiBar _bar;
        mutable UiDragHandle _handle;
        mutable UiLabel _label;
        mutable UiNumber _value_number;
        std::string _text_key;
        SDL_Texture* _icon = nullptr;
        std::optional<UiSliderSounds> _sounds;
        UiDragHandleConfig _handle_config{};
        UiSliderValueChangedCallback _on_value_changed;
        UiSliderLabelPlacement _label_placement = UiSliderLabelPlacement::None;
        UiSliderOrientation _orientation = UiSliderOrientation::Horizontal;
        UiSliderValueLabelMode _value_label_mode = UiSliderValueLabelMode::None;
        bool _draw_background = true;
        bool _draw_border = true;
        bool _drag_value_changed = false;
        float _bar_thickness = 6.0f;
        float _min_value = 0.0f;
        float _max_value = 1.0f;
        float _value = 0.0f;
        std::optional<float> _step = std::nullopt;
        std::uint32_t _last_slide_sound_ticks = 0;
        bool _has_last_slide_sound_tick = false;
        elysia::core::Color _background_color = elysia::core::colors::cobalt_blue;
        elysia::core::Color _disabled_background_color = elysia::core::colors::gray_700;
        elysia::core::Color _border_color = elysia::core::colors::sky_blue;
        elysia::core::Color _disabled_border_color = elysia::core::colors::gray_500;
        elysia::core::Color _fill_color = elysia::core::colors::glacial_white;
        elysia::core::Color _disabled_fill_color = elysia::core::colors::gray_500;
        elysia::core::Color _text_color = elysia::core::colors::white;
        elysia::core::Color _disabled_text_color = elysia::core::colors::gray_300;
    };
}