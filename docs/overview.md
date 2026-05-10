# Overview

## Project Positioning

Moonline is a C++ 2D game project

Estimated Code Volume 15-20k

The current source layout shows a clear separation between:

- `application/`: application startup and main loop
- `framework/`: reusable engine-like foundation
- `gameplay/`: game-specific logic
- `assets/`: textures, audio, fonts, configs, preload resources
- `docs/`: project notes and planning documents

The current development priority is to complete the core framework and data pipeline first, then build gameplay systems on top of that foundation.


## Planned Game Features

The intended target feature set currently includes:

- multi-stage progression
- support for multiple input schemes
- selectable player characters
- enemy system
- inventory / backpack system
- achievement system
- auto save
- manual save
- difficulty selection
- player progression, such as level-related growth
- drops and rewards tied to combat or stage completion

These systems are expected to share a common save data model and common gameplay state flow.

### Framework Responsibilities

The framework layer should eventually provide:

- application lifecycle
- scene switching and scene stack behavior if needed
- input abstraction
- resource loading and caching
- JSON/config loading
- audio playback management
- animation playback support
- base UI widgets and menu flow support
- save file IO support
- logging and runtime path management

### Gameplay Responsibilities

The gameplay layer should eventually provide:

- player runtime state
- enemy runtime state
- stage flow
- combat rules
- drop rules
- inventory logic
- achievement trigger logic
- difficulty modifiers
- save/load mapping between runtime state and persisted data

## Priority Development Areas

The next major work should focus on the framework rather than adding many end-user features immediately.

Recommended implementation order:

1. `ResourceManager` and related asset loading flow
2.  animation runtime support
3. `InputManager` for keyboard and future multi-input support
4. config/data loaders for character, enemy, stage, and global JSON
5. audio management wrapper
6. save system base interfaces and save data schema
7. reusable UI components for menus and selection screens
8. minimum playable gameplay loop

## Minimum Playable Milestone

Before implementing all advanced systems, the project should aim for a minimum playable slice:

- enter the game
- load resources successfully
- show a main menu
- choose one character
- enter test scene
- allow movement and attack
- spawn one enemy type
- resolve damage and enemy defeat
- enter one game stage
- return to result or next-stage flow
- save and load basic progress

## Notes On Scope

Even if the final game direction changes later, many systems remain useful and should still be built carefully:

- save/load
- inventory
- achievements
- difficulty settings
- config-driven content
- UI flow
- input abstraction

The part most likely to change with genre direction is the combat core. For that reason, framework work should avoid overcommitting to 
one combat implementation too early unless the direction is confirmed.