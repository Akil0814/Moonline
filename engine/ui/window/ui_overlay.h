#pragma once

#include "../../core/geometry/vector2.h"

namespace elysia::ui
{
enum class UiOverlayPlacement
{
    Center,
    LeftDrawer,
    RightDrawer,
    TopSheet,
    BottomSheet
};

enum class UiOverlayTransition
{
    None,
    Slide
};

// Window-managed overlay behavior, placement, and dismissal policy.
struct UiOverlayOptions
{
    bool open = true;
    bool modal = true;
    bool close_on_cancel = true;
    bool close_on_outside_click = true;
    UiOverlayPlacement placement = UiOverlayPlacement::Center;
    UiOverlayTransition transition = UiOverlayTransition::None;
    elysia::core::Vector2 fallback_size{ 360.0f,220.0f };
    int order = 1000;
};
}
