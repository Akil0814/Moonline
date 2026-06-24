#pragma once

#include <functional>

#include "ui_label.h"
#include "../../../core/interface/updatable.h"
#include "../../effects/ui_opacity_fade_core.h"

namespace elysia::ui
{
class UiFadeLabel : public UiLabel, public elysia::core::Updatable
{
public:
    using FadeLabelOnEnd = std::function<void()>;

    explicit UiFadeLabel(const elysia::core::Rect& rect = elysia::core::Rect::zero(),int order = 0,std::string text_key = {}) noexcept;
    UiFadeLabel(
        const elysia::core::Vector2& position,
        const elysia::core::Vector2& size,
        int order = 0,
        std::string text_key = {}
    ) noexcept;
    UiFadeLabel(
        const elysia::core::Vector2& center,
        const elysia::core::Vector2& size,
        UiFromCenterTag,
        int order = 0,
        std::string text_key = {}
    ) noexcept;

    void reset() noexcept override;
    void configure_playback(effects::UiOpacityFadeMode mode,double hold_time,double fade_in_duration,double fade_out_duration);
    void play() noexcept;
    void update(double delta) override;
    void set_on_end(FadeLabelOnEnd on_end);

private:
    void notify_finished();

private:
    elysia::ui::effects::UiOpacityFadeCore _fade;
    FadeLabelOnEnd _on_end;
    bool _end_emitted = false;
};
}