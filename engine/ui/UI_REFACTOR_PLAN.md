# UI Refactor Plan

## Summary
This document tracks the current UI architecture direction in `engine/ui` and the intended cleanup path.
The current structure is:
- `core/`: `UiElement`, `UiControl`, `UiChildHost`
- `layout/`: layout types, geometry helpers, stateless layout drivers
- `containers/`: concrete layout containers and scroll container
- `window/`: focus-domain window

## Current Direction
- `screen_rect` is the single geometry truth for widgets.
- `opacity` stays single-channel and is applied through render command modulation.
- Complex controls prefer composition over deep inheritance.
- `UiChildHost` owns child trees, recursive update/input/render, layout dirty state, and command-range clip/opacity post-processing.
- `layout/` owns layout formulas; containers and windows choose a layout driver but do not reimplement the math.

## Completed Refactors
- `UiElement` opacity was unified and pushed down to the base element layer.
- `from_center` became the common centered-construction tag.
- image and label widgets were grouped into `widgets/image/` and `widgets/label/`.
- reusable opacity animation cores live in `effects/`.
- `UiSlider` was split into composed child widgets and now uses `UiNumber`.
- `UiDragHandle` was extracted as a reusable primitive.
- `UiChildHost` replaced the old generic container base and now lives in `core/`.
- `UiWindow` reuses `UiChildHost` for child-tree hosting and keeps focus/navigation behavior local.
- layout types and stateless layout drivers now live in `layout/`.

## Current Pain Points
- `UiWindow` still owns custom focus-domain behavior that will likely need another cleanup pass.
- `UiScrollContainer` remains a layout special case because it mixes viewport clipping with scroll offsets.
- theme, focus, and layout are intentionally still separate systems.

## Recommended Next Steps
- Keep adding new layout algorithms under `layout/` rather than inside concrete containers.
- Continue narrowing `containers/` so each concrete container only chooses a layout driver and supplies config.
- Revisit window focus registration convenience and scroll/focus cooperation in a later pass.
- Render-command range post-processing lives in `core/ui_render_command_range_utils.*`.


