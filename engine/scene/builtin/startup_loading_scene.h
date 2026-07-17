#pragma once

#include "startup_loading_flow.h"
#include "startup_loading_scene_payload.h"
#include "../scene.h"
#include "../../loading/game_content_loader.h"

#include <string_view>

namespace elysia::ui
{
class UiBar;
class UiFadeImage;
class UiLabel;
}

namespace elysia::scene::builtin
{
class StartupLoadingScene final : public Scene
{
    friend class StartupLoadingSceneTestAccess;

public:
    StartupLoadingScene() = default;
    ~StartupLoadingScene() override = default;

    void on_enter(const ScenePayload& payload) override;
    void on_exit() override;
    void reset() override;

    void on_update(double delta) override;
    void on_input(
        const elysia::input::RawInputFrame& input,
        const std::vector<elysia::input::RawInputEvent>& events
    ) override;

private:
    bool create_presentation();
    void create_loading_ui();
    void begin_logo_sequence();
    void on_engine_logo_finished();
    void on_project_logo_finished();
    void mark_intro_finished();
    void mark_loading_finished();
    void handle_logo_action(StartupLogoAction action);
    void handle_completion_action(StartupLoadingAction action);
    void transition_to_success();
    void handle_failure(std::string_view message);
    void destroy_ui();
    void clear_state() noexcept;

private:
    StartupLoadingScenePayload _startup_payload;
    elysia::loading::GameContentLoader _content_loader;

    elysia::ui::UiBar* _loading_bar = nullptr;
    elysia::ui::UiFadeImage* _engine_logo = nullptr;
    elysia::ui::UiFadeImage* _project_logo = nullptr;
    elysia::ui::UiLabel* _start_prompt = nullptr;

    StartupLogoSequence _logo_sequence;
    StartupLoadingCompletion _completion;
};
}
