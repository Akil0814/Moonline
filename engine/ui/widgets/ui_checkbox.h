#pragma once

#include "../../core/render/colors.h"
#include "../core/ui_control.h"

#include <functional>
#include <optional>
#include <string>

struct SDL_Texture;

namespace elysia::ui
{
enum class UiCheckboxState
{
    Unchecked,
    Checked,
    Indeterminate
};

enum class UiCheckboxMarkStyle
{
    Checkmark,
    FilledBox
};

struct UiCheckboxVisualStateTextures
{
    SDL_Texture* idle = nullptr;
    SDL_Texture* focused = nullptr;
    SDL_Texture* pushed = nullptr;
    SDL_Texture* disabled = nullptr;
};

struct UiCheckboxTextures
{
    UiCheckboxVisualStateTextures unchecked{};
    UiCheckboxVisualStateTextures checked{};
    UiCheckboxVisualStateTextures indeterminate{};
};

struct UiCheckboxSounds
{
    std::string focus;
    std::string press;
    std::string toggle;
};

struct UiCheckboxConfig
{
    std::optional<UiCheckboxTextures> textures = std::nullopt;
    std::optional<UiCheckboxSounds> sounds = std::nullopt;
    UiCheckboxMarkStyle mark_style = UiCheckboxMarkStyle::Checkmark;
    bool draw_background = true;
    bool draw_border = true;
};

using UiCheckboxToggledCallback = std::function<void(UiCheckboxState state)>;

class UiCheckbox : public UiControl
{
public:
    explicit UiCheckbox(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiCheckbox(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiCheckbox(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;

    UiCheckbox(const elysia::core::Rect& rect,const UiCheckboxConfig& config,int order = 0) noexcept;
    UiCheckbox(const elysia::core::Vector2& position,const elysia::core::Vector2& size,const UiCheckboxConfig& config,int order = 0) noexcept;
    UiCheckbox(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,const UiCheckboxConfig& config,int order = 0) noexcept;

    ~UiCheckbox() override = default;

    void reset() noexcept override;

    void set_enabled(bool enabled) override;
    void set_focused(bool focused) override;

    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

    void set_checkbox_config(const UiCheckboxConfig& config);

    void set_state(UiCheckboxState state) noexcept;
    [[nodiscard]] UiCheckboxState state() const noexcept;
    void set_checked(bool checked) noexcept;
    [[nodiscard]] bool is_checked() const noexcept;
    [[nodiscard]] bool is_indeterminate() const noexcept;
    void toggle();
    void set_mark_style(UiCheckboxMarkStyle mark_style) noexcept;
    [[nodiscard]] UiCheckboxMarkStyle mark_style() const noexcept;

    void set_state_textures(const UiCheckboxTextures& textures);
    void clear_state_textures() noexcept;
    [[nodiscard]] const std::optional<UiCheckboxTextures>& state_textures() const noexcept;
    [[nodiscard]] bool has_complete_state_textures() const noexcept;

    void set_sounds(const UiCheckboxSounds& sounds);
    void clear_sounds() noexcept;
    [[nodiscard]] const std::optional<UiCheckboxSounds>& sounds() const noexcept;

    void set_on_toggled(UiCheckboxToggledCallback on_toggled);

    void set_idle_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color idle_color() const noexcept;

    void set_focused_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color focused_color() const noexcept;

    void set_pushed_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color pushed_color() const noexcept;

    void set_disabled_background_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color disabled_background_color() const noexcept;

    void set_border_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color border_color() const noexcept;

    void set_disabled_border_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color disabled_border_color() const noexcept;

    void set_checkmark_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color checkmark_color() const noexcept;

    void set_disabled_checkmark_color(elysia::core::Color color) noexcept;
    [[nodiscard]] elysia::core::Color disabled_checkmark_color() const noexcept;

    void set_padding(int padding) noexcept;
    [[nodiscard]] int padding() const noexcept;

    void set_draw_background(bool draw_background) noexcept;
    [[nodiscard]] bool draws_background() const noexcept;

    void set_draw_border(bool draw_border) noexcept;
    [[nodiscard]] bool draws_border() const noexcept;

private:
    void apply_checkbox_config(const UiCheckboxConfig& config);
    [[nodiscard]] bool set_state_internal(UiCheckboxState state,bool notify) noexcept;
    [[nodiscard]] bool toggle_internal(bool notify,bool play_toggle_sound) noexcept;
    [[nodiscard]] bool can_interact() const noexcept;
    [[nodiscard]] bool can_receive_pointer() const noexcept;
    [[nodiscard]] bool contains_pointer(int mouse_x,int mouse_y) const noexcept;
    [[nodiscard]] bool is_primary_pointer_event(const UiInputEvent& event) const noexcept;
    void clear_pushed_state() noexcept;
    void play_sound_if_set(const std::string& sound_key) const;

protected:
    [[nodiscard]] elysia::core::Rect content_rect() const noexcept;
    [[nodiscard]] virtual elysia::core::Rect checkbox_rect() const noexcept;
    [[nodiscard]] elysia::core::Color current_background_color() const noexcept;
    [[nodiscard]] elysia::core::Color current_border_color() const noexcept;

private:
    [[nodiscard]] const UiCheckboxVisualStateTextures* current_state_textures() const noexcept;
    [[nodiscard]] SDL_Texture* current_state_texture() const noexcept;
    [[nodiscard]] bool uses_texture_rendering() const noexcept;
    [[nodiscard]] elysia::core::Color current_checkmark_color() const noexcept;
    static UiCheckboxState toggled_state(UiCheckboxState state) noexcept;

private:
    std::optional<UiCheckboxTextures> _textures;
    std::optional<UiCheckboxSounds> _sounds;
    UiCheckboxToggledCallback _on_toggled;
    UiCheckboxState _state = UiCheckboxState::Unchecked;
    elysia::core::Color _idle_color = elysia::core::colors::cobalt_blue;
    elysia::core::Color _focused_color = elysia::core::colors::royal_blue;
    elysia::core::Color _pushed_color = elysia::core::colors::midnight_blue;
    elysia::core::Color _disabled_background_color = elysia::core::colors::gray_700;
    elysia::core::Color _border_color = elysia::core::colors::sky_blue;
    elysia::core::Color _disabled_border_color = elysia::core::colors::gray_500;
    elysia::core::Color _checkmark_color = elysia::core::colors::glacial_white;
    elysia::core::Color _disabled_checkmark_color = elysia::core::colors::gray_300;
    UiCheckboxMarkStyle _mark_style = UiCheckboxMarkStyle::Checkmark;
    int _padding = 4;
    bool _draw_background = true;
    bool _draw_border = true;
    bool _is_pushed = false;
};
}
