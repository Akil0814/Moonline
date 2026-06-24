#pragma once

#include <functional>

#include "ui_image.h"
#include "../../../core/interface/updatable.h"
#include "../../effects/ui_opacity_blink_core.h"

struct SDL_Texture;

namespace elysia::ui
{
class UiBlinkImage : public UiImage, public elysia::core::Updatable
{
public:
    using BlinkImageOnEnd = std::function<void()>;

    UiBlinkImage(SDL_Texture* texture,elysia::core::Vector2 pos,elysia::core::Vector2 size,int order = 0);
    UiBlinkImage(SDL_Texture* texture,elysia::core::Rect rect,int order = 0);
    UiBlinkImage(SDL_Texture* texture,elysia::core::Vector2 center,elysia::core::Vector2 image_size,UiFromCenterTag,int order = 0);
    UiBlinkImage(SDL_Texture* texture,elysia::core::Vector2 center,elysia::core::Vector2 source_size,elysia::core::Vector2 render_size,
        UiFromCenterTag,int order = 0);

    void reset() noexcept override;
    void configure_playback(effects::UiOpacityBlinkMode mode,double hold_time,
        double visible_duration,double hidden_duration,int blink_cycles = -1);
    void play();
    void update(double delta) override;
    void set_on_end(BlinkImageOnEnd on_end);

private:
    void notify_finished();

private:
    elysia::ui::effects::UiOpacityBlinkCore _blink;
    BlinkImageOnEnd _on_end;
    bool _end_emitted = false;
};
}