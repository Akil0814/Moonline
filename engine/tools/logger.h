#pragma once

#include "singleton.h"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <optional>
#include <source_location>
#include <string>
#include <string_view>

namespace elysia::tools
{
enum class LogLevel
{
    Debug,
    Info,
    Warn,
    Error
};

enum class LogFileMode
{
    Disabled,
    Append,
    NewRunFile
};

struct LoggerConfig
{
    LogLevel minimum_level{ LogLevel::Debug };
    LogFileMode file_mode{ LogFileMode::NewRunFile };
    std::string append_file_name{ "Moonline.log" };
};

class Logger : public Singleton<Logger>
{
    friend class Singleton<Logger>;

public:
    [[nodiscard]] bool configure(const LoggerConfig& config) noexcept;
    void initialize() noexcept;
    void shutdown() noexcept;
    [[nodiscard]] bool is_initialized() const noexcept;
    [[nodiscard]] std::optional<std::filesystem::path> active_file_path() const noexcept;

    void log(LogLevel level,std::string_view category,std::string_view message,
        std::source_location location = std::source_location::current()) noexcept;
    void debug(std::string_view category,std::string_view message,
        std::source_location location = std::source_location::current()) noexcept;
    void info(std::string_view category,std::string_view message,
        std::source_location location = std::source_location::current()) noexcept;
    void warn(std::string_view category,std::string_view message,
        std::source_location location = std::source_location::current()) noexcept;
    void error(std::string_view category,std::string_view message,
        std::source_location location = std::source_location::current()) noexcept;

private:
    Logger() = default;

    [[nodiscard]] bool should_log(LogLevel level) const noexcept;
    [[nodiscard]] std::filesystem::path new_run_file_path() const;
    [[nodiscard]] std::string format_line(LogLevel level,std::string_view category,
        std::string_view message,const std::source_location& location) const;
    void disable_file_sink() noexcept;
    static void write_sdl_fallback(LogLevel level,std::string_view category,
        std::string_view message,const std::source_location& location) noexcept;

private:
    mutable std::mutex _mutex;
    LoggerConfig _config;
    std::ofstream _file;
    std::optional<std::filesystem::path> _active_file_path;
    bool _initialized = false;
};
}
