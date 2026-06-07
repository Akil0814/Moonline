#pragma once

#include <SDL.h>

#include <functional>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "scene.h"
#include "scene_factory.h"
#include "scene_manager_observer.h"
#include "scene_request.h"
#include "scene_request_observer.h"

#include "../core/event/subject.h"
#include "../tools/singleton.h"

struct InputSnapshot;
struct InputEvent;

class SceneManager
    : public Singleton<SceneManager>
    , public Subject<SceneManagerObserver>
    , public SceneRequestObserver
{
    friend class Singleton<SceneManager>;

private:
    SceneManager() = default;
    ~SceneManager();

public:
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    SceneManager(SceneManager&&) = delete;
    SceneManager& operator=(SceneManager&&) = delete;

    template <typename T>
    bool register_scene(SceneId scene_id);

    bool start(
        SceneId first_scene,
        const ScenePayload& payload = ScenePayload{}
    );

    void on_input(
        const InputSnapshot& input,
        const std::vector<InputEvent>& events
    );

    void on_update(double delta);
    void on_render(SDL_Renderer* renderer);

    void on_scene_request(const SceneRequest& request) override;

    void shutdown();

private:
    using SceneProvider = std::function<Scene*(SceneReloadMode reload_mode)>;

    void notify_quit_requested();
    void process_pending_request();

    bool switch_to_registered_scene(
        SceneId target,
        const ScenePayload& payload,
        SceneReloadMode reload_mode
    );

    bool switch_to_scene(
        SceneId target,
        Scene* next_scene,
        const ScenePayload& payload,
        SceneReloadMode reload_mode
    );

    void attach_to_scene(Scene* scene);
    void detach_from_scene(Scene* scene);

private:
    Scene* _current_scene = nullptr;
    SceneId _current_scene_id = SceneId::None;

    SceneFactory _scene_factory;
    std::unordered_map<SceneId, SceneProvider> _scene_providers;

    SceneRequest _pending_request{};
    bool _has_pending_request = false;
};

template <typename T>
bool SceneManager::register_scene(SceneId scene_id)
{
    static_assert(
        std::is_base_of_v<Scene, T>,
        "T must derive from Scene."
    );

    if (scene_id == SceneId::None)
        return false;

    if (_scene_providers.find(scene_id) != _scene_providers.end())
        return false;

    _scene_providers.emplace(
        scene_id,
        [this](SceneReloadMode reload_mode) -> Scene*
        {
            T* existing_scene = _scene_factory.try_find_scene<T>();

            if (reload_mode == SceneReloadMode::Reset)
            {
                if (existing_scene && existing_scene != _current_scene)
                    existing_scene->reset();
            }
            else if (reload_mode == SceneReloadMode::Recreate)
            {
                if (existing_scene)
                {
                    if (_current_scene == existing_scene)
                    {
                        detach_from_scene(_current_scene);
                        _current_scene->on_exit();
                        _current_scene = nullptr;
                        _current_scene_id = SceneId::None;
                    }
                    else
                    {
                        detach_from_scene(existing_scene);
                    }

                    _scene_factory.destroy_scene<T>();
                }
            }

            return _scene_factory.get_scene<T>();
        }
    );

    return true;
}
