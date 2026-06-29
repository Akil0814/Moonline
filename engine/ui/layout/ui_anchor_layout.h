#pragma once

#include "ui_layout_geometry.h"
#include "../core/ui_child_host.h"

namespace elysia::ui::layout
{
void layout_anchored_children(std::vector<UiChildHost::ChildEntry>& children,const elysia::core::Rect& bounds) noexcept;
}
