#include "scene_manager.h"

#include <stdexcept>

SceneManager::~SceneManager()
{
    shutdown();
}

void SceneManager::start(
    SceneKey first_scene,
    const ScenePayload& payload
)
{
    if (_current_scene)
        throw std::logic_error("SceneManager::start called while a scene is already active.");

    switch_to_registered_scene(
        first_scene,
        payload,
        SceneReloadMode::Reuse
    );
}

void SceneManager::on_input(
    const RawInputFrame& input,
    const std::vector<RawInputEvent>& events
)
{
    if (_current_scene)
        _current_scene->on_input(input, events);

    process_pending_request();
}

void SceneManager::on_update(double delta)
{
    if (_current_scene)
        _current_scene->on_update(delta);

    process_pending_request();
}

void SceneManager::on_render(SDL_Renderer* renderer)
{
    if (_current_scene)
        _current_scene->on_render(renderer);
}

void SceneManager::on_scene_request(const SceneRequest& request)
{
    // Scene requests are only valid during normal input/update execution.
    // They must not be emitted from on_enter(), on_exit(), on_render(), reset(),
    // or destructors. Those phases are part of scene lifecycle management, not
    // scene flow decision-making.

    // A processing cycle may queue at most one scene request.
    // Reentrant requests while dispatching are also treated as programmer errors.

    if (_is_processing_request)
        throw std::logic_error("SceneManager received a reentrant scene request while processing another request.");


    if (_has_pending_request)
        throw std::logic_error("SceneManager received multiple scene requests in a single processing cycle.");

    _pending_request = request;
    _has_pending_request = true;
}

void SceneManager::notify_quit_requested()
{
    notify_observers(
        [](SceneManagerObserver& observer)
        {
            observer.on_scene_manager_quit_requested();
        }
    );
}

void SceneManager::process_pending_request()
{
    if (!_has_pending_request)
        return;

    struct ProcessingRequestGuard
    {
        bool& flag;

        explicit ProcessingRequestGuard(bool& processing_flag)
            : flag(processing_flag)
        {
            flag = true;
        }

        ~ProcessingRequestGuard()
        {
            flag = false;
        }
    };

    const SceneRequest request = _pending_request;

    _pending_request = SceneRequest{};
    _has_pending_request = false;

    ProcessingRequestGuard processing_guard(_is_processing_request);

    switch (request.type)
    {
    case SceneRequestType::Switch:
        switch_to_registered_scene(
            request.target,
            request.payload,
            request.reload_mode
        );
        break;

    case SceneRequestType::Quit:
        notify_quit_requested();
        break;

    case SceneRequestType::None:
    default:
        break;
    }
}

void SceneManager::switch_to_registered_scene(
    SceneKey target,
    const ScenePayload& payload,
    SceneReloadMode reload_mode
)
{
    if (target == SceneKeys::Invalid)
        throw std::logic_error("SceneManager::switch_to_registered_scene received SceneKeys::Invalid.");

    const auto iter = _scene_providers.find(target);

    if (iter == _scene_providers.end())
        throw std::logic_error("SceneManager::switch_to_registered_scene received an unregistered SceneKey.");

    Scene* next_scene = iter->second(reload_mode);

    switch_to_scene(
        target,
        next_scene,
        payload,
        reload_mode
    );
}

void SceneManager::switch_to_scene(
    SceneKey target,
    Scene* next_scene,
    const ScenePayload& payload,
    SceneReloadMode reload_mode
)
{
    if (!next_scene)
        throw std::logic_error("SceneManager::switch_to_scene received a null scene from provider.");

    if (_current_scene == next_scene)
    {
        if (reload_mode == SceneReloadMode::Reuse)
            return;

        detach_from_scene(_current_scene);
        _current_scene->on_exit();

        if (reload_mode == SceneReloadMode::Reset)
            _current_scene->reset();

        attach_to_scene(_current_scene);
        _current_scene_key = target;
        _current_scene->on_enter(payload);
        return;
    }

    if (_current_scene)
    {
        detach_from_scene(_current_scene);
        _current_scene->on_exit();
    }

    _current_scene = next_scene;
    _current_scene_key = target;

    if (reload_mode == SceneReloadMode::Reset)
        _current_scene->reset();

    attach_to_scene(_current_scene);
    _current_scene->on_enter(payload);
}

void SceneManager::attach_to_scene(Scene* scene)
{
    if (!scene)
        return;

    scene->attach(this);
}

void SceneManager::detach_from_scene(Scene* scene)
{
    if (!scene)
        return;

    scene->detach(this);
}

void SceneManager::shutdown()
{
    if (_current_scene)
    {
        detach_from_scene(_current_scene);
        _current_scene->on_exit();

        _current_scene = nullptr;
        _current_scene_key = SceneKeys::Invalid;
    }

    _scene_factory.destroy_all_scene();
    _scene_providers.clear();

    _pending_request = SceneRequest{};
    _has_pending_request = false;
    _is_processing_request = false;
}
