# UI Architecture

## Directory responsibilities

- `core/`: element ownership hooks, control state, render-command range processing.
- `widgets/`: atomic or externally indivisible leaf controls and visual elements.
- `containers/`: generic ownership, layout, scrolling, grouping, and page hosting.
- `composites/`: concrete controls assembled from widgets and containers.
- `layout/`, `focus/`, `input/`, `style/`, `effects/`: shared policies and mechanisms.
- `window/`: window-level focus, overlay, transient popup, and passive tooltip coordination.

## Dependency direction

The intended dependency direction is:

```text
core -> widgets / containers -> composites -> window and scene integration
```

Window headers should use forward declarations or narrow contracts for composite types. A window implementation may include a composite when it coordinates that composite at runtime.

## Component rules

- Widgets expose one indivisible state and interaction model. Button content, text-input placeholder, Slider value display, and Number suffix remain widget internals.
- A concrete assembly with independently meaningful children belongs in `composites/`.
- Composites use ownership and delegation instead of inheriting concrete widgets.
- Containers arrange arbitrary children and must not embed screen-specific titles, descriptions, or actions.
- `screen_rect` remains the geometry source of truth; layout code computes it and render/input code consumes it.
- Focus navigation describes directional targets and remains independent of the physical input device.

## Current component split

- Atomic controls: `UiButton`, `UiCheckbox`, `UiRadioButton`, `UiSlider`, `UiDragHandle`, `UiBar`, `UiTextInput`.
- Generic containers: Panel, List, Grid, Scroll, Chrome, ButtonGroup, RadioGroup, TabView.
- Composites: LabeledCheckbox, LabeledRadioButton, Dropdown, Tooltip, Dialog, ConfirmationDialog, TabBar, TabContainer.

## Verification

- New `.cpp` files require CMake regeneration because source discovery uses recursive globs.
- Structural changes must update include paths, theme/style types, test-scene coverage, and the full Debug build together.
