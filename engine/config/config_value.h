#pragma once

#include <string>
#include <variant>

using ConfigValue = std::variant<
    bool,
    int,
    float,
    std::string
>;