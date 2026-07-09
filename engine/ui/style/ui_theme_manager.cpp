#include "ui_theme_manager.h"

#include "../core/ui_element.h"

#include <algorithm>

namespace elysia::ui
{
UiThemeRegistration::UiThemeRegistration(std::weak_ptr<RegistrationRecord> record) noexcept
    : _record(std::move(record))
{
}

UiThemeRegistration::~UiThemeRegistration()
{
    reset();
}

UiThemeRegistration::UiThemeRegistration(UiThemeRegistration&& other) noexcept
    : _record(std::move(other._record))
{
}

UiThemeRegistration& UiThemeRegistration::operator=(UiThemeRegistration&& other) noexcept
{
    if (this == &other)
        return *this;

    reset();
    _record = std::move(other._record);
    return *this;
}

void UiThemeRegistration::reset() noexcept
{
    const std::shared_ptr<RegistrationRecord> record = _record.lock();
    if (record && record->manager)
        record->manager->release_registration(*record);
    _record.reset();
}

bool UiThemeRegistration::registered() const noexcept
{
    const std::shared_ptr<RegistrationRecord> record = _record.lock();
    return record
        && record->manager
        && record->element
        && record->registration_count > 0U;
}

UiThemeManager::UiThemeManager() = default;

UiThemeManager::~UiThemeManager()
{
    detach_all_elements();
}

UiThemeRegistration UiThemeManager::register_element(UiElement& element)
{
    auto found = std::find_if(_records.begin(),_records.end(),[&element](const auto& record)
    {
        return record && record->element == &element;
    });

    std::shared_ptr<UiThemeRegistration::RegistrationRecord> record;
    if (found == _records.end())
    {
        record = std::make_shared<UiThemeRegistration::RegistrationRecord>();
        record->manager = this;
        record->element = &element;
        record->registration_count = 1U;
        _records.push_back(record);
        element.attach_theme_manager(*this);
    }
    else
    {
        record = *found;
        ++record->registration_count;
    }

    apply_theme_to_element(element);
    return UiThemeRegistration(record);
}

void UiThemeManager::unregister_element(UiElement& element) noexcept
{
    auto found = std::find_if(_records.begin(),_records.end(),[&element](const auto& record)
    {
        return record && record->element == &element;
    });
    if (found != _records.end() && *found)
        release_registration(*(*found));
}

void UiThemeManager::set_theme(UiBuiltinTheme theme)
{
    _current_builtin_theme = theme;
    _current_theme = make_builtin_theme(theme);
    reapply_theme();
}

UiBuiltinTheme UiThemeManager::current_builtin_theme() const noexcept
{
    return _current_builtin_theme;
}

const UiTheme& UiThemeManager::current_theme() const noexcept
{
    return _current_theme;
}

void UiThemeManager::reapply_theme()
{
    std::vector<UiElement*> elements;
    elements.reserve(_records.size());
    for (const auto& record : _records)
    {
        if (record && record->element)
            elements.push_back(record->element);
    }

    for (UiElement* element : elements)
    {
        if (!element || element->is_destroyed())
            continue;
        apply_theme_to_element(*element);
    }
}

void UiThemeManager::apply_theme_to_element(UiElement& element) const
{
    if (element.is_destroyed() || !element.uses_theme())
        return;

    element.apply_theme(_current_theme);
}

void UiThemeManager::release_registration(UiThemeRegistration::RegistrationRecord& record) noexcept
{
    if (record.manager != this || record.registration_count == 0U)
        return;

    --record.registration_count;
    if (record.registration_count > 0U)
        return;

    UiElement* element = record.element;
    record.manager = nullptr;
    record.element = nullptr;

    if (element)
        element->detach_theme_manager(*this);

    _records.erase(std::remove_if(_records.begin(),_records.end(),[&record](const auto& candidate)
    {
        return candidate.get() == &record;
    }),_records.end());
}

void UiThemeManager::invalidate_element_registration(UiElement& element) noexcept
{
    auto found = std::find_if(_records.begin(),_records.end(),[&element](const auto& record)
    {
        return record && record->element == &element;
    });
    if (found == _records.end() || !*found)
        return;

    std::shared_ptr<UiThemeRegistration::RegistrationRecord> record = *found;
    record->manager = nullptr;
    record->element = nullptr;
    record->registration_count = 0U;
    _records.erase(found);
}

void UiThemeManager::detach_all_elements() noexcept
{
    std::vector<UiElement*> elements;
    elements.reserve(_records.size());
    for (const auto& record : _records)
    {
        if (record)
        {
            if (record->element)
                elements.push_back(record->element);
            record->manager = nullptr;
            record->element = nullptr;
            record->registration_count = 0U;
        }
    }

    _records.clear();
    for (UiElement* element : elements)
    {
        if (element)
            element->detach_theme_manager(*this);
    }
}
}
