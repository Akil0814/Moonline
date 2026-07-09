#pragma once

#include "ui_control_focus_scope_host.h"

#include <vector>

namespace elysia::ui
{
class UiDelegatedFocusMixin
{
protected:
    struct DelegatedRegionEntry
    {
        UiElement* region = nullptr;
        UiElement* up = nullptr;
        UiElement* down = nullptr;
        UiElement* left = nullptr;
        UiElement* right = nullptr;
    };

protected:
    void reset_delegated_focus_state() noexcept;

    [[nodiscard]] static UiElement* delegated_focus_region(UiElement* element) noexcept;
    [[nodiscard]] static const UiElement* delegated_focus_region(const UiElement* element) noexcept;
    [[nodiscard]] static UiFocusScope* delegated_scope_for_region(UiElement* region) noexcept;
    [[nodiscard]] static const UiFocusScope* delegated_scope_for_region(const UiElement* region) noexcept;

    [[nodiscard]] std::vector<UiElement*> delegated_focus_regions(const UiChildHost& host) const;
    [[nodiscard]] std::vector<UiControl*> delegated_controls_in_region(const UiElement& element) const;
    [[nodiscard]] UiControl* first_focusable_control_in_delegated_region(const UiElement* element) const noexcept;

    void build_delegated_focus_entries(
        const std::vector<DelegatedRegionEntry>& regions,
        std::vector<UiControlFocusScopeHost::FocusEntry>& out_entries);

    [[nodiscard]] UiFocusScope* delegated_owner_scope_of(const UiControl* control) const noexcept;
    [[nodiscard]] UiElement* delegated_region_of(const UiControl* control,const std::vector<UiElement*>& regions) const noexcept;
    [[nodiscard]] bool focus_delegated_region(UiElement* region,bool focus_first);
    [[nodiscard]] bool enter_delegated_region(UiControlFocusScopeHost& host,UiElement* region);
    void sync_host_delegated_focus_target(UiControlFocusScopeHost& host) noexcept;
    void sync_delegated_owner_scope_target(UiControl* control) noexcept;
    void sync_delegated_scope_focus(UiControl* focused_target,bool scope_focused,const std::vector<UiElement*>& regions) noexcept;
    void sync_delegated_scope_focus(UiElement* active_region,bool scope_focused,const std::vector<UiElement*>& regions) noexcept;

private:
    struct ScopeMember
    {
        UiControl* control = nullptr;
        UiFocusScope* scope = nullptr;
    };

private:
    std::vector<ScopeMember> _delegated_scope_members;
};
}
