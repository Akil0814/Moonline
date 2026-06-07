#pragma once

#include <SDL.h>

#include <functional>
#include <stdexcept>
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
    void register_scene(SceneKey scene_key);

    void start(
        SceneKey first_scene,
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

    void switch_to_registered_scene(
        SceneKey target,
        const ScenePayload& payload,
        SceneReloadMode reload_mode
    );

    void switch_to_scene(
        SceneKey target,
        Scene* next_scene,
        const ScenePayload& payload,
        SceneReloadMode reload_mode
    );

    void attach_to_scene(Scene* scene);
    void detach_from_scene(Scene* scene);

private:
    Scene* _current_scene = nullptr;
    SceneKey _current_scene_key = SceneKeys::Invalid;

    SceneFactory _scene_factory;
    std::unordered_map<SceneKey, SceneProvider> _scene_providers;

    SceneRequest _pending_request{};
    bool _has_pending_request = false;
    bool _is_processing_request = false;
};

template <typename T>
void SceneManager::register_scene(SceneKey scene_key)
{
    static_assert(
        std::is_base_of_v<Scene, T>,
        "T must derive from Scene."
    );

    if (scene_key == SceneKeys::Invalid)
        throw std::logic_error("SceneManager::register_scene received SceneKeys::Invalid.");

    if (_scene_providers.find(scene_key) != _scene_providers.end())
        throw std::logic_error("SceneManager::register_scene received a duplicate SceneKey.");

    _scene_providers.emplace(
        scene_key,
        [this](SceneReloadMode reload_mode) -> Scene*
        {
            T* existing_scene = _scene_factory.try_find_scene<T>();

            if (reload_mode == SceneReloadMode::Recreate)
            {
                if (existing_scene)
                {
                    if (_current_scene == existing_scene)
                    {
                        detach_from_scene(_current_scene);
                        _current_scene->on_exit();
                        _current_scene = nullptr;
                        _current_scene_key = SceneKeys::Invalid;
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
}
