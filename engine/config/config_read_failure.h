#pragma once

#include <string>

namespace elysia::config
{
enum class ConfigReadError
{
    NotInitialized,
    DocumentNotFound,
    InvalidJsonPointer,
    PathNotFound,
    TypeMismatch
};

struct ConfigReadFailure
{
    ConfigReadError error = ConfigReadError::NotInitialized;
    std::string document_key;
    std::string json_pointer;
    std::string message;
};
}
