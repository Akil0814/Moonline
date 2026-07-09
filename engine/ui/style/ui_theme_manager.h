#pragma once

#include "ui_theme.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace elysia::ui
{
class UiElement;

class UiThemeManager;

class UiThemeRegistration
{
public:
    UiThemeRegistration() = default;
    ~UiThemeRegistration();

    UiThemeRegistration(const UiThemeRegistration&) = delete;
    UiThemeRegistration& operator=(const UiThemeRegistration&) = delete;

    UiThemeRegistration(UiThemeRegistration&& other) noexcept;
    UiThemeRegistration& operator=(UiThemeRegistration&& other) noexcept;

    void reset() noexcept;
    [[nodiscard]] bool registered() const noexcept;

private:
    struct RegistrationRecord
    {
        UiThemeManager* manager = nullptr;
        UiElement* element = nullptr;
        std::size_t registration_count = 0;
    };

    explicit UiThemeRegistration(std::weak_ptr<RegistrationRecord> record) noexcept;

    std::weak_ptr<RegistrationRecord> _record;

    friend class UiThemeManager;
};

class UiThemeManager
{
public:
    UiThemeManager();
    ~UiThemeManager();

    UiThemeManager(const UiThemeManager&) = delete;
    UiThemeManager& operator=(const UiThemeManager&) = delete;
    UiThemeManager(UiThemeManager&&) = delete;
    UiThemeManager& operator=(UiThemeManager&&) = delete;

    [[nodiscard]] UiThemeRegistration register_element(UiElement& element);
    void unregister_element(UiElement& element) noexcept;

    void set_theme(UiBuiltinTheme theme);
    [[nodiscard]] UiBuiltinTheme current_builtin_theme() const noexcept;
    [[nodiscard]] const UiTheme& current_theme() const noexcept;
    void reapply_theme();

private:
    void apply_theme_to_element(UiElement& element) const;
    void release_registration(UiThemeRegistration::RegistrationRecord& record) noexcept;
    void invalidate_element_registration(UiElement& element) noexcept;
    void detach_all_elements() noexcept;

    UiBuiltinTheme _current_builtin_theme = UiBuiltinTheme::BlueGlassMoon;
    UiTheme _current_theme = make_builtin_theme(UiBuiltinTheme::BlueGlassMoon);
    std::vector<std::shared_ptr<UiThemeRegistration::RegistrationRecord>> _records;

    friend class UiElement;
    friend class UiThemeRegistration;
};
}
