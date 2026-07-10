#pragma once

#include "../focus/ui_control_focus_scope_host.h"
#include "../focus/ui_delegated_focus_mixin.h"
#include "../style/ui_theme_roles.h"
#include "../text/ui_text_content.h"
#include "../window/ui_overlay.h"

#include <functional>

namespace elysia::ui
{
class UiButton;
class UiChromeContainer;
class UiLabel;
class UiListContainer;
class UiPanel;
class UiWindow;

struct UiConfirmationDialogConfig
{
    UiTextContent title{};
    UiTextContent message{};
    UiTextContent confirm{};
    UiTextContent cancel{};
    UiTextContent close{};
    UiButtonThemeRole confirm_theme_role = UiButtonThemeRole::Primary;
    UiButtonThemeRole cancel_theme_role = UiButtonThemeRole::Default;
};

using UiConfirmationDialogCallback = std::function<void()>;

class UiConfirmationDialog final : public UiControlFocusScopeHost, private UiDelegatedFocusMixin
{
public:
    explicit UiConfirmationDialog(const elysia::core::Rect& rect = elysia::core::Rect{ 0,0,420,240 },int order = 0) noexcept;
    UiConfirmationDialog(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order = 0) noexcept;
    UiConfirmationDialog(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order = 0) noexcept;
    ~UiConfirmationDialog() override = default;

    void reset() noexcept override;
    void update(double delta) override;
    void on_ui_input_frame(const UiInputFrame& input) override;
    bool on_ui_input_event(const UiInputEvent& event) override;
    void submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const override;
    [[nodiscard]] elysia::core::Vector2 content_extent() const noexcept override;
    bool focus_first_available() override;

    void set_config(const UiConfirmationDialogConfig& config);
    [[nodiscard]] const UiConfirmationDialogConfig& config() const noexcept;
    void set_on_confirm(UiConfirmationDialogCallback on_confirm);

    void register_as_overlay(UiWindow& window,UiOverlayOptions options = {});
    void open();
    void close();

protected:
    void rebuild_layout() override;
    void rebuild_focus_registry() override;
    void apply_theme(const UiTheme& theme) override;

private:
    void create_internal_children();
    void sync_config_to_children();
    void sync_theme_to_children(const UiTheme* theme = nullptr);
    void sync_delegated_focus() noexcept;
    void confirm();
    [[nodiscard]] static bool is_default_overlay_options(const UiOverlayOptions& options) noexcept;

private:
    UiChromeContainer* _chrome = nullptr;
    UiLabel* _title_label = nullptr;
    UiButton* _close_button = nullptr;
    UiPanel* _body_panel = nullptr;
    UiLabel* _message_label = nullptr;
    UiListContainer* _action_row = nullptr;
    UiButton* _cancel_button = nullptr;
    UiButton* _confirm_button = nullptr;
    UiWindow* _registered_window = nullptr;
    UiConfirmationDialogConfig _config{};
    UiConfirmationDialogCallback _on_confirm;
};
}
