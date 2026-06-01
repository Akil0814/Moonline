#include "ui_panel.h"

#include "../../resources/resource_manager.h"
#include "../style/ui_theme.h"
#include "../style/ui_style.h"

UiPanel::UiPanel(Vector2 position, Vector2 size, int order)
    : UILayout(position, size, order)
{
}

void UiPanel::on_render(SDL_Renderer* renderer)
{
    if (!renderer)
    {
        return;
    }

    refresh_theme_if_needed();
    const SDL_Rect& object_rect = rect();
    if (object_rect.w > 0 && object_rect.h > 0)
    {
        SDL_Texture* resolved_texture = resolve_background_texture();
        if (resolved_texture)
        {
            Uint8 old_alpha = 255;
            SDL_GetTextureAlphaMod(resolved_texture, &old_alpha);
            SDL_SetTextureAlphaMod(resolved_texture, _background_alpha);
            SDL_RenderCopy(renderer, resolved_texture, nullptr, &object_rect);
            SDL_SetTextureAlphaMod(resolved_texture, old_alpha);
        }
        else if (_draw_background)
        {
            Uint8 old_r = 0;
            Uint8 old_g = 0;
            Uint8 old_b = 0;
            Uint8 old_a = 0;
            const Uint8 effective_alpha = static_cast<Uint8>(
                (static_cast<unsigned int>(_background_color.a)
                    * static_cast<unsigned int>(_background_alpha)) / 255U
            );
            SDL_GetRenderDrawColor(renderer, &old_r, &old_g, &old_b, &old_a);
            SDL_SetRenderDrawColor(
                renderer,
                _background_color.r,
                _background_color.g,
                _background_color.b,
                effective_alpha
            );
            SDL_RenderFillRect(renderer, &object_rect);
            SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);
        }

        if (_draw_border)
        {
            Uint8 old_r = 0;
            Uint8 old_g = 0;
            Uint8 old_b = 0;
            Uint8 old_a = 0;
            SDL_GetRenderDrawColor(renderer, &old_r, &old_g, &old_b, &old_a);
            SDL_SetRenderDrawColor(
                renderer,
                _border_color.r,
                _border_color.g,
                _border_color.b,
                _border_color.a
            );
            SDL_RenderDrawRect(renderer, &object_rect);
            SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);
        }
    }

    SDL_Rect previous_clip_rect{};
    const SDL_bool had_clip_rect = SDL_RenderIsClipEnabled(renderer);
    if (had_clip_rect)
    {
        SDL_RenderGetClipRect(renderer, &previous_clip_rect);
    }

    if (_clip_children)
    {
        SDL_RenderSetClipRect(renderer, &object_rect);
    }

    UILayout::on_render(renderer);

    if (_clip_children)
    {
        if (had_clip_rect)
        {
            SDL_RenderSetClipRect(renderer, &previous_clip_rect);
        }
        else
        {
            SDL_RenderSetClipRect(renderer, nullptr);
        }
    }
}

void UiPanel::reset()
{
    UILayout::reset();
    _background_texture = nullptr;
    _background_texture_key.clear();
    _background_color = SDL_Color{ 24, 24, 32, 220 };
    _border_color = SDL_Color{ 220, 220, 220, 255 };
    _background_alpha = 255;
    _draw_background = true;
    _draw_border = false;
    _clip_children = false;
    _panel_theme_role = UiPanelThemeRole::Default;
    mark_theme_dirty();
}

void UiPanel::set_draw_background(bool draw_background)
{
    _draw_background = draw_background;
}

bool UiPanel::draws_background() const
{
    return _draw_background;
}

void UiPanel::set_draw_border(bool draw_border)
{
    _draw_border = draw_border;
}

bool UiPanel::draws_border() const
{
    return _draw_border;
}

void UiPanel::set_background_color(SDL_Color color)
{
    _background_color = color;
}

SDL_Color UiPanel::background_color() const
{
    return _background_color;
}

void UiPanel::set_border_color(SDL_Color color)
{
    _border_color = color;
}

SDL_Color UiPanel::border_color() const
{
    return _border_color;
}

void UiPanel::set_background_texture(SDL_Texture* texture)
{
    _background_texture = texture;
}

SDL_Texture* UiPanel::background_texture() const
{
    return _background_texture;
}

void UiPanel::set_background_texture_key(const std::string& texture_key)
{
    _background_texture_key = texture_key;
}

const std::string& UiPanel::background_texture_key() const
{
    return _background_texture_key;
}

void UiPanel::set_background_alpha(Uint8 alpha)
{
    _background_alpha = alpha;
}

Uint8 UiPanel::background_alpha() const
{
    return _background_alpha;
}

void UiPanel::set_panel_theme_role(UiPanelThemeRole panel_theme_role)
{
    _panel_theme_role = panel_theme_role;
    mark_theme_dirty();
}

UiPanelThemeRole UiPanel::panel_theme_role() const
{
    return _panel_theme_role;
}

void UiPanel::set_clip_children(bool clip_children)
{
    _clip_children = clip_children;
}

bool UiPanel::clips_children() const
{
    return _clip_children;
}

SDL_Texture* UiPanel::resolve_background_texture() const
{
    if (_background_texture)
    {
        return _background_texture;
    }

    if (_background_texture_key.empty())
    {
        return nullptr;
    }

    return ResourceManager::instance()->find_texture(_background_texture_key);
}

void UiPanel::apply_theme(const UiTheme& theme)
{
    const PanelStyle* style = &theme._default_panel;
    switch (_panel_theme_role)
    {
    case UiPanelThemeRole::Default:
        style = &theme._default_panel;
        break;

    case UiPanelThemeRole::Screen:
        style = &theme._screen_panel;
        break;

    case UiPanelThemeRole::Dialog:
        style = &theme._dialog_panel;
        break;

    case UiPanelThemeRole::List:
        style = &theme._list_panel;
        break;
    }

    UiStyle::apply_panel(*this, *style);
}
