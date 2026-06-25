#pragma once
#include <functional>
#include <string>
#include <optional>
#include <string_view>
#include <variant>
#include "../core/ui_control.h"
#include "label/ui_label.h"
#include "ui_bar.h"

namespace elysia::ui
{
    enum class UiSliderLabelPlacement
    {
        None,
        Left,
        Right,
        Above,
        Below,
        Center
    };

    enum class UiSliderOrientation
    {
        Horizontal,
        Vertical
    };

    enum class UiSliderValueLabelMode
    {
        None,
        Value,
        Percent
    };

    struct UiSliderSounds
    {
        std::string on_focus{};
        std::string on_slide{};
        std::string on_settle{};

        double min_slide_sound_interval = 0.05;
    };

    struct UiSliderTextContent
    {
        std::string text_key{};
    };

    struct UiSliderIconContent
    {
        SDL_Texture* texture = nullptr;
    };

    using UiSliderLabelContent = std::variant<std::monostate,
        UiSliderTextContent,UiSliderIconContent>;

    struct UiSliderConfig
    {
        UiSliderLabelContent label_content{};
        UiSliderLabelPlacement label_placement = UiSliderLabelPlacement::None;
        UiSliderOrientation orientation = UiSliderOrientation::Horizontal;
        UiSliderValueLabelMode value_label_mode = UiSliderValueLabelMode::None;

        std::optional<UiSliderSounds> slider_sound;

        bool draw_background = true;
        bool draw_border = true;

        elysia::core::Vector2 handle_size{ 18.0f, 18.0f };
        float bar_thickness = 6.0f;
    };

    using UiSliderValueChangedCallback = std::function<void(float value)>;


    class UiSlider : public UiControl
    {
    public:
        explicit UiSlider(const elysia::core::Rect& rect = elysia::core::Rect::zero(), int order = 0) noexcept;
        UiSlider(const elysia::core::Vector2& position, const elysia::core::Vector2& size, int order = 0) noexcept;
        UiSlider(const elysia::core::Vector2& center, const elysia::core::Vector2& size, UiFromCenterTag, int order = 0) noexcept;

        UiSlider(const elysia::core::Rect& rect, const UiSliderConfig& config, int order = 0) noexcept;
        UiSlider(const elysia::core::Vector2& position, const elysia::core::Vector2& size, const UiSliderConfig& config, int order = 0) noexcept;
        UiSlider(const elysia::core::Vector2& center, const elysia::core::Vector2& size, UiFromCenterTag, const UiSliderConfig& config, int order = 0) noexcept;

        void set_enabled(bool enabled);
        void set_focused(bool focused);

        bool on_ui_input_event(const UiInputEvent& event) override;
        void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

        void set_text_key(std::string text_key);
        [[nodiscard]] const std::string& text_key() const noexcept;

        void set_draw_background(bool draw_background);
        [[nodiscard]] bool draws_background() const noexcept;

        void set_draw_border(bool draw_border);
        [[nodiscard]] bool draws_border() const noexcept;

    private:
        UiBar _bar;
        UiLabel _label;

        std::string _text_key;
        SDL_Texture* _icon = nullptr;

        UiSliderLabelPlacement label_placement = UiSliderLabelPlacement::None;
        UiSliderOrientation orientation = UiSliderOrientation::Horizontal;
        UiSliderValueLabelMode value_label_mode = UiSliderValueLabelMode::None;

        std::optional<UiSliderSounds> slider_sound;

        bool draw_background = true;
        bool draw_border = true;

        elysia::core::Vector2 handle_size{ 18.0f, 18.0f };
        float bar_thickness = 6.0f;

        elysia::core::Color _background_color = elysia::core::colors::cobalt_blue;
        elysia::core::Color _disabled_background_color = elysia::core::colors::gray_700;

        elysia::core::Color _border_color = elysia::core::colors::sky_blue;
        elysia::core::Color _disabled_border_color = elysia::core::colors::gray_500;

        elysia::core::Color _text_color = elysia::core::colors::white;
        elysia::core::Color _disabled_text_color = elysia::core::colors::gray_300;
    };

}
