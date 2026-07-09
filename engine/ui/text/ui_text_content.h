#pragma once

#include <string>
#include <utility>

namespace elysia::ui
{
enum class UiTextContentKind
{
    None,
    TextKey,
    RawText
};

struct UiTextContent
{
    UiTextContentKind kind = UiTextContentKind::None;
    std::string value{};

    [[nodiscard]] bool empty() const noexcept
    {
        return kind == UiTextContentKind::None || value.empty();
    }
};

[[nodiscard]] inline UiTextContent ui_text_key(std::string value)
{
    if (value.empty())
        return {};
    return UiTextContent{ UiTextContentKind::TextKey,std::move(value) };
}

[[nodiscard]] inline UiTextContent ui_raw_text(std::string value)
{
    if (value.empty())
        return {};
    return UiTextContent{ UiTextContentKind::RawText,std::move(value) };
}
}
