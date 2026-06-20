#pragma once

#include <string>
#include <variant>

namespace elysia::config
{
using ConfigValue = std::variant<
    bool,
    int,
    double,
    std::string
>;
}
