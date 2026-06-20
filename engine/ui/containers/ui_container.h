#include "../core/ui_element.h"

#include <memory>

namespace elysia::ui
{
class UiContainer : public UiElement
{
public:
    void add_child(std::unique_ptr<UiElement> child);
    void set_layout(UiLayout layout);

    void mark_layout_dirty();
    void update_layout_if_dirty();

private:
    std::vector<std::unique_ptr<UiElement>> _children;
    UiLayout _layout;
    bool _layout_dirty = true;
};
}
