#pragma once

#include "../core/ui_text_source.h"
#include "../focus/ui_control_focus_scope_host.h"
#include "../focus/ui_delegated_focus_mixin.h"
#include "../style/ui_style.h"
#include "../style/ui_theme_roles.h"
#include "../style/ui_visual_styles.h"

namespace elysia::ui
{
class UiWindow;
class UiChromeContainer;
class UiPanel;
class UiScrollContainer;
class UiButton;
class UiLabel;
class UiTextBlock;

class UiDialog : public UiControlFocusScopeHost, private UiDelegatedFocusMixin
{
public:
    explicit UiDialog(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0) noexcept;
    UiDialog(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiDialog(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiDialog() override = default;

    void reset() noexcept override;
    void update(double delta) override;
    void on_ui_input_frame(const UiInputFrame& input) override;
    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;
    [[nodiscard]] elysia::core::Vector2 content_extent() const noexcept override;

    void set_title_source(UiTextSource title_source);
    [[nodiscard]] const UiTextSource& title_source() const noexcept;
    void set_title_key(std::string text_key);
    void set_title_raw_text(std::string raw_text);

    void set_body_source(UiTextSource body_source);
    [[nodiscard]] const UiTextSource& body_source() const noexcept;
    void set_body_text_key(std::string text_key);
    void set_body_raw_text(std::string raw_text);

    void set_close_button_source(UiTextSource close_button_source);
    [[nodiscard]] const UiTextSource& close_button_source() const noexcept;
    void set_close_button_text_key(std::string text_key);
    void set_close_button_raw_text(std::string raw_text);

    void set_body_scroll_enabled(bool enabled) noexcept;
    [[nodiscard]] bool body_scroll_enabled() const noexcept;
    void set_header_visible(bool visible) noexcept;
    [[nodiscard]] bool header_visible() const noexcept;

    void set_style(const UiDialogStyle& style) noexcept;
    [[nodiscard]] const UiDialogStyle& style() const noexcept;
    [[nodiscard]] bool has_style_override() const noexcept;
    void clear_style_override() noexcept;

    void set_theme_role(UiDialogThemeRole role) noexcept;
    [[nodiscard]] UiDialogThemeRole theme_role() const noexcept;

    void register_as_overlay(UiWindow& window,UiOverlayOptions options = {});
    void open(UiWindow& window);
    void close(UiWindow& window);

protected:
    void rebuild_layout() override;
    void rebuild_focus_registry() override;
    void apply_theme(const UiTheme& theme) override;

private:
    void create_internal_children();
    void sync_sources_to_children();
    void sync_style_to_children();
    void sync_theme_to_children(const UiTheme* theme = nullptr);
    [[nodiscard]] static bool is_default_overlay_options(const UiOverlayOptions& options) noexcept;

private:
    UiChromeContainer* _chrome = nullptr;
    UiLabel* _title_label = nullptr;
    UiPanel* _body_panel = nullptr;
    UiScrollContainer* _body_scroll = nullptr;
    UiTextBlock* _body_text = nullptr;
    UiButton* _close_button = nullptr;
    UiWindow* _registered_window = nullptr;
    UiTextSource _title_source{ UiTextSourceKind::None,{} };
    UiTextSource _body_source{ UiTextSourceKind::None,{} };
    UiTextSource _close_button_source{ UiTextSourceKind::TextKey,"menu_scene.exit_confirm.cancel" };
    UiStyleState<UiDialogStyle> _style_state;
    UiDialogThemeRole _theme_role = UiDialogThemeRole::Default;
    bool _body_scroll_enabled = true;
};
}
