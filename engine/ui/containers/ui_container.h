#include "../core/ui_element.h"

//后期实现 每个ui组件可以加入容器 实现位置锚点与显示状态的托管，也可以选择将组件或容器向UiThemeManager注册，实现主题的托管


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
