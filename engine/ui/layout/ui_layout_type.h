#include "../../core/geometry/vector2.h"

enum class UiLayoutAnchor
{
    TopLeft,
    TopCenter,
    TopRight,

    CenterLeft,
    Center,
    CenterRight,

    BottomLeft,
    BottomCenter,
    BottomRight
};

enum class UiLayoutDirection
{
    Horizontal,
    Vertical
};

enum class UiLayoutAlign
{
    Start,
    Center,
    End
};

struct UiLayoutPadding
{
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
};

struct UiLayoutMargin
{
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
};

struct UiLayoutChildOptions
{
    UiLayoutMargin _margin;
    UiLayoutAlign _cross_align = UiLayoutAlign::Start;
    Vector2 _size_override{ 0.0f, 0.0f };

    bool _use_custom_cross_align = false;
    bool _fill_cross_axis = false;
    bool _use_size_override = false;
};

struct UiLayoutTransform
{
    Vector2 translation{ 0.0f, 0.0f };
    Vector2 scale{ 1.0f, 1.0f };
};