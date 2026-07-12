#include "logger.h"

#include "../io/path/path_manager.h"

#include <SDL.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif
#include <sstream>

namespace elysia::tools
{
namespace
{
[[nodiscard]] int level_rank(LogLevel level) noexcept
{
    return static_cast<int>(level);
}

[[nodiscard]] const char* level_name(LogLevel level) noexcept
{
    switch (level)
    {
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info: return "INFO";
    case LogLevel::Warn: return "WARN";
    case LogLevel::Error: return "ERROR";
    }
    return "UNKNOWN";
}

[[nodiscard]] SDL_LogPriority sdl_priority(LogLevel level) noexcept
{
    switch (level)
    {
    case LogLevel::Debug: return SDL_LOG_PRIORITY_DEBUG;
    case LogLevel::Info: return SDL_LOG_PRIORITY_INFO;
    case LogLevel::Warn: return SDL_LOG_PRIORITY_WARN;
    case LogLevel::Error: return SDL_LOG_PRIORITY_ERROR;
    }
    return SDL_LOG_PRIORITY_ERROR;
}

[[nodiscard]] std::tm local_time(std::time_t time)
{
    std::tm result{};
#if defined(_WIN32)
    localtime_s(&result,&time);
#else
    localtime_r(&time,&result);
#endif
    return result;
}

[[nodiscard]] int process_id() noexcept
{
#if defined(_WIN32)
    return _getpid();
#else
    return static_cast<int>(getpid());
#endif
}
}

bool Logger::configure(const LoggerConfig& config) noexcept
{
    try
    {
        if (config.file_mode == LogFileMode::Append)
        {
            const std::filesystem::path append_name(config.append_file_name);
            if (append_name.empty() || append_name.is_absolute() || append_name.has_parent_path())
                return false;
        }
        std::lock_guard lock(_mutex);
        if (_initialized)
            return false;
        _config = config;
        return true;
    }
    catch (...)
    {
        return false;
    }
}

void Logger::initialize() noexcept
{
    try
    {
        std::lock_guard lock(_mutex);
        if (_initialized)
            return;
        _initialized = true;
        _active_file_path.reset();
        if (_config.file_mode == LogFileMode::Disabled)
            return;

        const auto* path_manager = elysia::io::PathManager::instance();
        if (!path_manager || !path_manager->is_initialized())
            return;

        const std::filesystem::path path = _config.file_mode == LogFileMode::Append
            ? path_manager->logs() / _config.append_file_name
            : new_run_file_path();
        const std::ios::openmode mode = _config.file_mode == LogFileMode::Append
            ? std::ios::out | std::ios::app
            : std::ios::out;
        _file.open(path,mode);
        if (!_file.is_open())
            return;
        _active_file_path = path;
    }
    catch (...)
    {
        disable_file_sink();
    }
}

void Logger::shutdown() noexcept
{
    try
    {
        std::lock_guard lock(_mutex);
        if (_file.is_open())
        {
            _file.flush();
            _file.close();
        }
        _active_file_path.reset();
        _initialized = false;
    }
    catch (...)
    {
        disable_file_sink();
        _initialized = false;
    }
}

bool Logger::is_initialized() const noexcept
{
    try
    {
        std::lock_guard lock(_mutex);
        return _initialized;
    }
    catch (...)
    {
        return false;
    }
}

std::optional<std::filesystem::path> Logger::active_file_path() const noexcept
{
    try
    {
        std::lock_guard lock(_mutex);
        return _active_file_path;
    }
    catch (...)
    {
        return std::nullopt;
    }
}

void Logger::log(LogLevel level,std::string_view category,std::string_view message,
    std::source_location location) noexcept
{
    try
    {
        std::lock_guard lock(_mutex);
        if (!should_log(level))
            return;
        if (!_file.is_open())
        {
            write_sdl_fallback(level,category,message,location);
            return;
        }

        _file << format_line(level,category,message,location) << '\n';
        if (!_file.good())
        {
            disable_file_sink();
            write_sdl_fallback(level,category,message,location);
            return;
        }
        if (level_rank(level) >= level_rank(LogLevel::Warn))
            _file.flush();
    }
    catch (...)
    {
        disable_file_sink();
        write_sdl_fallback(level,category,message,location);
    }
}

void Logger::debug(std::string_view category,std::string_view message,std::source_location location) noexcept
{
    log(LogLevel::Debug,category,message,location);
}

void Logger::info(std::string_view category,std::string_view message,std::source_location location) noexcept
{
    log(LogLevel::Info,category,message,location);
}

void Logger::warn(std::string_view category,std::string_view message,std::source_location location) noexcept
{
    log(LogLevel::Warn,category,message,location);
}

void Logger::error(std::string_view category,std::string_view message,std::source_location location) noexcept
{
    log(LogLevel::Error,category,message,location);
}

bool Logger::should_log(LogLevel level) const noexcept
{
    return level_rank(level) >= level_rank(_config.minimum_level);
}

std::filesystem::path Logger::new_run_file_path() const
{
    const auto* path_manager = elysia::io::PathManager::instance();
    const std::time_t now = std::time(nullptr);
    std::ostringstream stem;
    const std::tm timestamp = local_time(now);
    stem << "Moonline-" << std::put_time(&timestamp,"%Y%m%d-%H%M%S") << '-' << process_id();
    const std::filesystem::path logs = path_manager->logs();
    for (unsigned int index = 0;; ++index)
    {
        const std::filesystem::path candidate = logs / (stem.str()
            + (index == 0 ? "" : "-" + std::to_string(index)) + ".log");
        if (!std::filesystem::exists(candidate))
            return candidate;
    }
}

std::string Logger::format_line(LogLevel level,std::string_view category,std::string_view message,
    const std::source_location& location) const
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::filesystem::path source_path(location.file_name());
    if (const auto* path_manager = elysia::io::PathManager::instance(); path_manager && path_manager->is_initialized())
    {
        const std::filesystem::path relative = source_path.lexically_relative(path_manager->root());
        if (!relative.empty())
            source_path = relative;
        else
            source_path = source_path.filename();
    }
    const std::tm timestamp = local_time(time);
    std::ostringstream output;
    output << std::put_time(&timestamp,"%Y-%m-%d %H:%M:%S")
        << " [" << level_name(level) << "]"
        << " [" << category << "]"
        << " (" << source_path.generic_string() << ':' << location.line()
        << " " << location.function_name() << ") " << message;
    return output.str();
}

void Logger::disable_file_sink() noexcept
{
    try
    {
        if (_file.is_open())
            _file.close();
        _file.clear();
        _active_file_path.reset();
    }
    catch (...)
    {
    }
}

void Logger::write_sdl_fallback(LogLevel level,std::string_view category,std::string_view message,
    const std::source_location& location) noexcept
{
    try
    {
        const char* category_text = category.empty() ? "" : category.data();
        const char* message_text = message.empty() ? "" : message.data();
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,sdl_priority(level),"[%.*s] %s:%u %s: %.*s",
            static_cast<int>(category.size()),category_text,location.file_name(),location.line(),
            location.function_name(),static_cast<int>(message.size()),message_text);
    }
    catch (...)
    {
    }
}
}
