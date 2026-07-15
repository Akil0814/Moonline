#pragma once

#include "../core/geometry/rect.h"
#include "../core/geometry/vector2.h"
#include "../tools/singleton.h"

#include <cstdint>
#include <expected>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace elysia::config
{
struct ConfigOrigin
{
    std::string config_path;
    std::string json_pointer;
    std::string key_namespace;
    std::string full_key;

    [[nodiscard]] std::string describe() const;
};

enum class ConfigLoadError { OpenFailed, InvalidSchema, InvalidKey, InvalidValue, DuplicateKey };
struct ConfigLoadFailure
{
    ConfigLoadError error = ConfigLoadError::InvalidSchema;
    std::string message;
    ConfigOrigin first;
    ConfigOrigin second;
};

enum class ConfigAccessError { NotInitialized, MissingKey, TypeMismatch, InvalidValue };
struct ConfigAccessFailure
{
    ConfigAccessError error = ConfigAccessError::MissingKey;
    std::string key;
    std::string expected_type;
    std::string actual_type;
    ConfigOrigin origin;
    std::string message;
};

class ConfigService final : public elysia::tools::Singleton<ConfigService>
{
    friend elysia::tools::Singleton<ConfigService>;
public:
    struct Snapshot;
    [[nodiscard]] std::expected<void,ConfigLoadFailure> initialize(const std::filesystem::path& manifest_path);
    void shutdown() noexcept;
    [[nodiscard]] bool is_initialized() const noexcept;
    [[nodiscard]] bool contains(std::string_view key) const;

    [[nodiscard]] std::expected<std::int64_t,ConfigAccessFailure> get_int(std::string_view key) const;
    [[nodiscard]] std::expected<double,ConfigAccessFailure> get_double(std::string_view key) const;
    [[nodiscard]] std::expected<bool,ConfigAccessFailure> get_bool(std::string_view key) const;
    [[nodiscard]] std::expected<std::string,ConfigAccessFailure> get_string(std::string_view key) const;
    [[nodiscard]] std::expected<elysia::core::Vector2,ConfigAccessFailure> get_vector2(std::string_view key) const;
    [[nodiscard]] std::expected<elysia::core::Rect,ConfigAccessFailure> get_rect(std::string_view key) const;

    [[nodiscard]] std::expected<std::vector<std::int64_t>,ConfigAccessFailure> get_int_array(std::string_view key) const;
    [[nodiscard]] std::expected<std::vector<double>,ConfigAccessFailure> get_double_array(std::string_view key) const;
    [[nodiscard]] std::expected<std::vector<bool>,ConfigAccessFailure> get_bool_array(std::string_view key) const;
    [[nodiscard]] std::expected<std::vector<std::string>,ConfigAccessFailure> get_string_array(std::string_view key) const;
    [[nodiscard]] std::expected<std::vector<elysia::core::Vector2>,ConfigAccessFailure> get_vector2_array(std::string_view key) const;
    [[nodiscard]] std::expected<std::vector<elysia::core::Rect>,ConfigAccessFailure> get_rect_array(std::string_view key) const;

private:
    ConfigService() = default;
    void log_once(const ConfigAccessFailure& failure) const;
    mutable std::mutex _mutex;
    std::shared_ptr<const Snapshot> _snapshot;
    mutable std::unordered_set<std::string> _logged_access_errors;
};
}
