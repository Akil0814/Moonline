#pragma once

#include "../../core/render/colors.h"
#include "../style/ui_style.h"
#include "../style/ui_theme_roles.h"
#include "../style/ui_interaction_style.h"
#include "../core/ui_control.h"

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

struct SDL_Texture;

namespace elysia::ui
{
// Optional textures used to skin button states instead of chrome rendering.
struct UiButtonTextures
{
    SDL_Texture* idle = nullptr;
    SDL_Texture* focused = nullptr;
    SDL_Texture* pushed = nullptr;
    SDL_Texture* disabled = nullptr;
};

// Text-only button content resolved through the localized text system.
struct UiButtonTextContent
{
    std::string text_key{};
};

// Icon-only button content rendered from a caller-owned texture.
struct UiButtonIconContent
{
    SDL_Texture* texture = nullptr;
};

// Explicit texture set used when the button renders entirely from textures.
struct UiButtonTextureSetContent
{
    UiButtonTextures textures{};
};

using UiButtonContent = std::variant<
    std::monostate,
    UiButtonTextContent,
    UiButtonIconContent,
    UiButtonTextureSetContent
>;

// Sound keys played as button focus, press, and click state change.
struct UiButtonSounds
{
    std::string focus;
    std::string press;
    std::string click;
};

// Visual styling for button chrome and text.
struct UiButtonStyle
{
    UiChromeStyle chrome{};
    UiEnabledDisabledColors text{};
};

// Bundles content, sound, and style overrides for button construction or updates.
struct UiButtonConfig
{
    UiButtonContent content{};
    std::optional<UiButtonSounds> sounds;
    std::optional<UiButtonStyle> style = std::nullopt;
};

class UiButton : public UiControl
{
public:
    enum class UiButtonVisualMode
    {
        None,
        Text,
        Textured,
        Icon
    };

public:
    using ClickCallback = std::function<void()>;

    explicit UiButton(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiButton(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiButton(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;

    UiButton(const elysia::core::Rect& rect,const UiButtonConfig& config,int order = 0) noexcept;
    UiButton(const elysia::core::Vector2& position,const elysia::core::Vector2& size,const UiButtonConfig& config,int order = 0) noexcept;
    UiButton(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,const UiButtonConfig& config,int order = 0) noexcept;

    ~UiButton() override = default;

    void reset() noexcept override;

    void set_enabled(bool enabled) override;
    void set_focused(bool focused) override;

    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;

    // Applies content, sounds, and style as one atomic button configuration update.
    void set_button_config(const UiButtonConfig& config);

    void set_text_key(std::string text_key);
    [[nodiscard]] const std::string& text_key() const noexcept;

    void set_state_textures(const UiButtonTextures& textures);
    void clear_state_textures();
    [[nodiscard]] bool has_state_textures() const noexcept;
    [[nodiscard]] UiButtonVisualMode visual_mode() const noexcept;

    void set_sounds(const UiButtonSounds& sounds);
    void clear_sounds();
    [[nodiscard]] const UiButtonSounds& sounds() const noexcept;

    void set_on_click(ClickCallback on_click);

    void set_style(const UiButtonStyle& style);
    [[nodiscard]] const UiButtonStyle& style() const noexcept;
    [[nodiscard]] bool has_style_override() const noexcept;
    void clear_style_override() noexcept;

    void set_theme_role(UiButtonThemeRole role) noexcept;
    [[nodiscard]] UiButtonThemeRole theme_role() const noexcept;

    void set_text_point_size(int point_size);
    [[nodiscard]] int text_point_size() const noexcept;

    void set_padding(int padding);
    [[nodiscard]] int padding() const noexcept;

private:
    // Applies the config payload without exposing intermediate visual states.
    void apply_button_config(const UiButtonConfig& config);
    // Switches between text, icon, textured, or empty content modes.
    void apply_button_content(const UiButtonContent& content);
    void set_icon_texture(SDL_Texture* texture) noexcept;
    // Clears the active content payload and resets the visual mode.
    void clear_content() noexcept;

    // Returns true only when the button should react to confirm or pointer input.
    [[nodiscard]] bool can_interact() const noexcept;
    [[nodiscard]] bool can_receive_pointer() const noexcept;
    [[nodiscard]] elysia::core::Color current_background_color() const noexcept;
    [[nodiscard]] elysia::core::Color current_border_color() const noexcept;
    [[nodiscard]] elysia::core::Color current_text_color() const noexcept;
    // Chooses the texture that matches the current button interaction state.
    [[nodiscard]] SDL_Texture* current_state_texture() const noexcept;
    // Returns the padded interior used to place text or icons.
    [[nodiscard]] elysia::core::Rect content_rect() const noexcept;
    // Fits rendered text into the padded button interior using the active alignment.
    [[nodiscard]] elysia::core::Rect text_render_rect(SDL_Texture* text_texture) const noexcept;
    [[nodiscard]] bool contains_pointer(int mouse_x,int mouse_y) const noexcept;
    [[nodiscard]] bool is_primary_pointer_event(const UiInputEvent& event) const noexcept;
    // Clears any pressed state left behind by focus loss or input cancellation.
    void clear_pushed_state() noexcept;
    // Plays a configured sound only when the corresponding key is present.
    void play_sound_if_set(std::string_view sound_key) const;
    void apply_theme(const UiTheme& theme) override;

private:
    std::string _text_key;
    UiButtonSounds _sounds;
    UiButtonTextures _state_textures;
    ClickCallback _on_click;
    UiButtonVisualMode _visual_mode = UiButtonVisualMode::None;
    UiStyleState<UiButtonStyle> _style_state;
    UiButtonThemeRole _theme_role = UiButtonThemeRole::Default;

    int _text_point_size = 24;
    int _padding = 10;
    bool _is_pushed = false;
};
}
