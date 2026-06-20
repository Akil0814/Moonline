#pragma once

#include <string>
#include <variant>

using ConfigValue = std::variant<
    bool,
    int,
    double,
    std::string
>;