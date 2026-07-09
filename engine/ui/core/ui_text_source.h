#pragma once

#include <string>

namespace elysia::ui
{
enum class UiTextSourceKind
{
    None,
    TextKey,
    RawText
};

struct UiTextSource
{
    UiTextSourceKind kind = UiTextSourceKind::None;
    std::string value{};
};
}
