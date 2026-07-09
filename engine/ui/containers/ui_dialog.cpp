#include "ui_dialog.h"

#include "ui_chrome_container.h"
#include "ui_panel.h"
#include "ui_scroll_container.h"
#include "../focus/ui_focus_scope_utils.h"
#include "../style/ui_style_defaults.h"
#include "../style/ui_theme.h"
#include "../widgets/ui_button.h"
#include "../widgets/label/ui_label.h"
#include "../widgets/text/ui_text_block.h"
#include "../window/ui_window.h"

#include <algorithm>

namespace elysia::ui
{
UiDialog::UiDialog(const elysia::core::Rect& rect,int order) noexcept
    : UiControlFocusScopeHost(rect,order)
{
    reset();
}

UiDialog::UiDialog(const elysia::core::Vector2& position,const elysia::core::Vector2& size,int order) noexcept
    : UiDialog(elysia::core::Rect(position.x,position.y,size.x,size.y),order) {}

UiDialog::UiDialog(const elysia::core::Vector2& center,const elysia::core::Vector2& size,UiFromCenterTag,int order) noexcept
    : UiDialog(elysia::core::Rect::from_center(center,size),order) {}

void UiDialog::reset() noexcept
{
    UiControlFocusScopeHost::reset();
    reset_delegated_focus_state();
    _chrome = nullptr;
    _title_label = nullptr;
    _body_panel = nullptr;
    _body_scroll = nullptr;
    _body_text = nullptr;
    _close_button = nullptr;
    _registered_window = nullptr;
    _title_content = UiTextContent{};
    _body_content = UiTextContent{};
    _action_content = ui_text_key("menu_scene.exit_confirm.cancel");
    _style_state.reset(UiDialogStyle{});
    _theme_role = UiDialogThemeRole::Default;
    _body_scroll_enabled = true;

    UiDialogStyle defaults = _style_state.theme_style();
    defaults.overlay_defaults.open = false;
    defaults.overlay_defaults.modal = true;
    defaults.overlay_defaults.close_on_cancel = true;
    defaults.overlay_defaults.close_on_outside_click = false;
    defaults.overlay_defaults.placement = UiOverlayPlacement::Center;
    defaults.overlay_defaults.transition = UiOverlayTransition::None;
    defaults.overlay_defaults.fallback_size = elysia::core::Vector2(480.0f,360.0f);
    defaults.overlay_defaults.order = 1000;
    _style_state.set_theme_style(defaults);

    create_internal_children();
    sync_sources_to_children();
    sync_style_to_children();
}

void UiDialog::update(double delta)
{
    UiControlFocusScopeHost::update(delta);
    sync_delegated_scope_focus(UiControlFocusScopeHost::focused_target(),is_scope_focused(),delegated_focus_regions(*this));
    sync_body_scroll_gamepad_focus();
}

void UiDialog::on_ui_input_frame(const UiInputFrame& input)
{
    UiControlFocusScopeHost::on_ui_input_frame(input);
    sync_delegated_scope_focus(UiControlFocusScopeHost::focused_target(),is_scope_focused(),delegated_focus_regions(*this));
    sync_body_scroll_gamepad_focus();
}

bool UiDialog::on_ui_input_event(const UiInputEvent& event)
{
    sync_body_scroll_gamepad_focus();
    if (event.type == UiInputEventType::MouseWheel
        && event.device == elysia::input::InputDevice::Gamepad
        && _body_scroll_enabled
        && _body_scroll
        && _body_scroll->on_ui_input_event(event))
    {
        return true;
    }

    const bool handled = UiControlFocusScopeHost::on_ui_input_event(event);
    sync_delegated_scope_focus(UiControlFocusScopeHost::focused_target(),is_scope_focused(),delegated_focus_regions(*this));
    sync_body_scroll_gamepad_focus();
    return handled;
}

void UiDialog::submit_ui_render_commands(std::vector<elysia::core::UiRenderCommand>& out_commands) const
{
    if (!is_visible())
        return;

    auto* self = const_cast<UiDialog*>(this);
    self->cleanup_destroyed_children();
    self->update_layout_if_dirty();
    self->refresh_focus_registry();
    self->ensure_valid_focus();
    self->sync_delegated_scope_focus(
        UiControlFocusScopeHost::focused_target(),
        is_scope_focused(),
        delegated_focus_regions(*this));
    self->apply_focus_state();

    UiControlFocusScopeHost::submit_ui_render_commands(out_commands);
}

elysia::core::Vector2 UiDialog::content_extent() const noexcept
{
    const elysia::core::Vector2 explicit_size = size();
    if (_chrome)
    {
        const elysia::core::Vector2 chrome_extent = _chrome->content_extent();
        return elysia::core::Vector2(
            std::max(explicit_size.x,chrome_extent.x),
            std::max(explicit_size.y,chrome_extent.y));
    }
    return explicit_size;
}

void UiDialog::set_title_content(UiTextContent title_content)
{
    _title_content = std::move(title_content);
    sync_sources_to_children();
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

const UiTextContent& UiDialog::title_content() const noexcept
{
    return _title_content;
}

void UiDialog::set_body_content(UiTextContent body_content)
{
    _body_content = std::move(body_content);
    sync_sources_to_children();
    notify_layout_parent_of_intrinsic_layout_invalidation();
}

const UiTextContent& UiDialog::body_content() const noexcept
{
    return _body_content;
}

void UiDialog::set_action_content(UiTextContent action_content)
{
    _action_content = std::move(action_content);
    if (_action_content.empty())
        _action_content = ui_text_key("menu_scene.exit_confirm.cancel");
    sync_sources_to_children();
}

const UiTextContent& UiDialog::action_content() const noexcept
{
    return _action_content;
}

void UiDialog::set_body_scroll_enabled(bool enabled) noexcept
{
    _body_scroll_enabled = enabled;
    sync_style_to_children();
    sync_body_scroll_gamepad_focus();
}

bool UiDialog::body_scroll_enabled() const noexcept
{
    return _body_scroll_enabled;
}

void UiDialog::set_header_visible(bool visible) noexcept
{
    if (_chrome)
        _chrome->set_header_visible(visible);
    mark_layout_dirty();
}

bool UiDialog::header_visible() const noexcept
{
    return _chrome ? _chrome->header_visible() : true;
}

void UiDialog::set_style(const UiDialogStyle& style) noexcept
{
    _style_state.set_style_override(style);
    sync_style_to_children();
    mark_layout_dirty();
}

const UiDialogStyle& UiDialog::style() const noexcept
{
    return _style_state.effective_style();
}

bool UiDialog::has_style_override() const noexcept
{
    return _style_state.has_style_override();
}

void UiDialog::clear_style_override() noexcept
{
    _style_state.clear_style_override();
    sync_style_to_children();
    mark_layout_dirty();
}

void UiDialog::set_theme_role(UiDialogThemeRole role) noexcept
{
    _theme_role = role;
    request_theme_reapply();
}

UiDialogThemeRole UiDialog::theme_role() const noexcept
{
    return _theme_role;
}

void UiDialog::register_as_overlay(UiWindow& window,UiOverlayOptions options)
{
    _registered_window = &window;
    if (is_default_overlay_options(options))
        options = style().overlay_defaults;

    if (size().x <= elysia::core::Vector2::k_epsilon || size().y <= elysia::core::Vector2::k_epsilon)
        options.fallback_size = style().overlay_defaults.fallback_size;
    else
        options.fallback_size = size();

    window.register_overlay(*this,options);
}

void UiDialog::open(UiWindow& window)
{
    _registered_window = &window;
    window.set_overlay_open(*this,true);
}

void UiDialog::close(UiWindow& window)
{
    _registered_window = &window;
    window.set_overlay_open(*this,false);
}

void UiDialog::rebuild_layout()
{
    if (!_chrome || !_body_panel || !_body_scroll || !_body_text || !_close_button)
        return;

    _chrome->set_screen_rect(content_rect());
    _chrome->update_layout_if_dirty();

    const UiDialogStyle& current_style = style();
    const elysia::core::Rect panel_rect = _body_panel->screen_rect();
    const float close_height = std::max(0.0f,current_style.close_button_height);
    const float footer_spacing = std::max(0.0f,current_style.body_footer_spacing);
    const float button_width = std::min(180.0f,std::max(120.0f,panel_rect.width() - 24.0f));
    const float scroll_height = std::max(0.0f,panel_rect.height() - close_height - footer_spacing);

    _body_panel->set_child_layout_options(0,UiLayoutChildOptions{
        ._anchor = UiLayoutAnchor::TopLeft,
        ._margin = UiLayoutMargin{},
        ._cross_align = UiLayoutAlign::Start,
        ._size_override = elysia::core::Vector2(panel_rect.width(),scroll_height),
        ._use_custom_cross_align = false,
        ._fill_cross_axis = false,
        ._use_size_override = true
    });
    _body_panel->set_child_layout_options(1,UiLayoutChildOptions{
        ._anchor = UiLayoutAnchor::BottomCenter,
        ._margin = UiLayoutMargin{},
        ._cross_align = UiLayoutAlign::Start,
        ._size_override = elysia::core::Vector2(button_width,close_height),
        ._use_custom_cross_align = false,
        ._fill_cross_axis = false,
        ._use_size_override = true
    });

    _body_panel->update_layout_if_dirty();
    _body_scroll->update_layout_if_dirty();
    _body_text->set_size(elysia::core::Vector2(std::max(0.0f,_body_scroll->screen_rect().width()),0.0f));
    _body_scroll->mark_layout_dirty();
    _body_scroll->update_layout_if_dirty();
}

void UiDialog::rebuild_focus_registry()
{
    std::vector<DelegatedRegionEntry> regions;
    for (UiElement* region : delegated_focus_regions(*this))
        regions.push_back(DelegatedRegionEntry{ region,nullptr,nullptr,nullptr,nullptr });

    std::vector<FocusEntry> entries;
    build_delegated_focus_entries(regions,entries);
    set_focus_entries(std::move(entries));
}

void UiDialog::apply_theme(const UiTheme& theme)
{
    sync_theme_to_children(&theme);
}

void UiDialog::create_internal_children()
{
    auto chrome = std::make_unique<UiChromeContainer>(screen_rect());
    chrome->set_use_theme(false);
    chrome->set_header_height(48.0f);
    chrome->set_header_padding(UiLayoutPadding{ 12.0f,6.0f,12.0f,6.0f });
    chrome->set_body_padding(UiLayoutPadding{
        static_cast<float>(style().body_padding),
        static_cast<float>(style().body_padding),
        static_cast<float>(style().body_padding),
        static_cast<float>(style().body_padding)
    });
    _chrome = chrome.get();
    UiChildHost::add_child(std::move(chrome));

    auto title = std::make_unique<UiLabel>(elysia::core::Rect{ 0,0,280,36 });
    title->set_theme_role(UiLabelThemeRole::Title);
    title->set_typography_role(UiTypographyRole::DialogTitle);
    title->set_vertical_align(TextVerticalAlign::Center);
    title->set_horizontal_align(TextHorizontalAlign::Left);
    title->set_use_theme(false);
    _title_label = title.get();
    _chrome->add_title_child(std::move(title));

    auto body = std::make_unique<UiPanel>(elysia::core::Rect{ 0,0,360,260 });
    body->set_use_theme(false);
    _body_panel = body.get();

    auto scroll = std::make_unique<UiScrollContainer>(elysia::core::Rect{ 0,0,360,200 });
    scroll->set_use_theme(false);
    scroll->set_scroll_axis(UiScrollAxis::Vertical);
    scroll->set_scrollbar_visibility(UiScrollBarVisibility::Auto);
    scroll->set_scroll_step(elysia::core::Vector2(20.0f,28.0f));
    _body_scroll = scroll.get();

    auto text = std::make_unique<UiTextBlock>(elysia::core::Rect{ 0,0,360,0 });
    text->set_use_theme(false);
    text->set_typography_role(UiTypographyRole::DialogBody);
    text->set_horizontal_align(TextHorizontalAlign::Left);
    _body_text = text.get();
    _body_scroll->set_content(std::move(text));

    auto close_button = std::make_unique<UiButton>(elysia::core::Rect{ 0,0,160,42 });
    close_button->set_use_theme(false);
    close_button->set_typography_role(UiTypographyRole::DialogAction);
    close_button->set_on_click([this]()
    {
        if (_registered_window)
            _registered_window->set_overlay_open(*this,false);
    });
    _close_button = close_button.get();

    _body_panel->add_child(std::move(scroll),UiPanelInsertDirection::Down);
    _body_panel->add_child(std::move(close_button),UiPanelInsertDirection::Down);
    _chrome->set_body(std::move(body));
}

void UiDialog::sync_sources_to_children()
{
    if (_title_label)
        _title_label->set_text_content(_title_content);

    if (_body_text)
        _body_text->set_text_content(_body_content);

    if (_close_button)
        _close_button->set_text_content(_action_content);
}

void UiDialog::sync_style_to_children()
{
    if (!_chrome || !_body_scroll || !_body_text || !_close_button || !_title_label)
        return;

    const UiDialogStyle& current_style = style();
    _chrome->set_body_padding(UiLayoutPadding{
        static_cast<float>(current_style.body_padding),
        static_cast<float>(current_style.body_padding),
        static_cast<float>(current_style.body_padding),
        static_cast<float>(current_style.body_padding)
    });
    _title_label->set_typography_role(UiTypographyRole::DialogTitle);
    _body_text->set_typography_role(UiTypographyRole::DialogBody);
    _body_text->set_padding(current_style.text_padding);
    _close_button->set_typography_role(UiTypographyRole::DialogAction);
    _body_scroll->set_scrollbar_visibility(_body_scroll_enabled ? UiScrollBarVisibility::Auto : UiScrollBarVisibility::Hidden);
    mark_layout_dirty();
}

void UiDialog::sync_theme_to_children(const UiTheme* theme)
{
    const UiTheme& resolved_theme = theme ? *theme : builtin_theme(UiBuiltinTheme::BlueGlassMoon);

    if (_chrome)
        _chrome->set_style(apply_theme_colors(UiChromeContainerStyle{},resolved_theme.chrome_container_style));
    if (_title_label)
        _title_label->set_style(apply_theme_colors(UiLabelStyle{},resolved_theme.label(UiLabelThemeRole::Title)));
    if (_body_text)
        _body_text->set_style(apply_theme_colors(UiTextBlockStyle{},resolved_theme.label(UiLabelThemeRole::Default)));
    if (_body_scroll)
        _body_scroll->set_style(apply_theme_colors(UiScrollContainerStyle{},resolved_theme.scroll_container_style));
    if (_close_button)
        _close_button->set_style(apply_theme_colors(UiButtonStyle{},resolved_theme.dialog_style.action_button));
}

void UiDialog::sync_body_scroll_gamepad_focus() noexcept
{
    if (_body_scroll)
        _body_scroll->set_scope_focused(_body_scroll_enabled && is_scope_focused());
}

bool UiDialog::is_default_overlay_options(const UiOverlayOptions& options) noexcept
{
    return options.open == UiOverlayOptions{}.open
        && options.modal == UiOverlayOptions{}.modal
        && options.close_on_cancel == UiOverlayOptions{}.close_on_cancel
        && options.close_on_outside_click == UiOverlayOptions{}.close_on_outside_click
        && options.placement == UiOverlayOptions{}.placement
        && options.transition == UiOverlayOptions{}.transition
        && options.fallback_size.nearly_equals(UiOverlayOptions{}.fallback_size)
        && options.order == UiOverlayOptions{}.order;
}
}
