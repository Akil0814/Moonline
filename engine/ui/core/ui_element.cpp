#include "ui_element.h"
#include "ui_child_host.h"

namespace elysia::ui
{
UiElement::~UiElement() = default;

void UiElement::notify_base_style_invalidated() noexcept
{
    if (_layout_parent)
        _layout_parent->on_child_base_style_invalidated(*this);
}
}
