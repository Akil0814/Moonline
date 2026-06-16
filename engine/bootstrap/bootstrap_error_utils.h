#pragma once

#include <string>

inline void append_bootstrap_error(std::string& error, const std::string& message)
{
    if (!error.empty())
        error += '\n';

    error += message;
}
