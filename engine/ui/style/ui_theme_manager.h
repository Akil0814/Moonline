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

    // Releases one registration reference. Safe to call repeatedly and after the manager died.
    void reset() noexcept;
    // Reports whether this handle still owns a live registration entry.
    [[nodiscard]] bool registered() const noexcept;

private:
    // Shared state lets moved handles and manager teardown observe the same registration lifetime.
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

    // Registration and theme opt-in are separate: the element must also use_theme()==true
    // before theme refreshes will call apply_theme().
    [[nodiscard]] UiThemeRegistration register_element(UiElement& element);
    // Low-level escape hatch for explicit unregistration when the RAII handle is not used.
    void unregister_element(UiElement& element) noexcept;

    // Rebuilds the active theme snapshot and reapplies it to every still-registered element.
    void set_theme(UiBuiltinTheme theme);
    [[nodiscard]] UiBuiltinTheme current_builtin_theme() const noexcept;
    [[nodiscard]] const UiTheme& current_theme() const noexcept;
    // Replays the current theme onto the registered element set without changing the theme id.
    void reapply_theme();

private:
    // Applies theme data only to one explicitly-registered element. This is not a recursive
    // scene-tree walker; composite controls must forward the theme to their internal children.
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
