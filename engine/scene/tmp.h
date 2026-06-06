#pragma once

#include <SDL.h>

#include <algorithm>
#include <array>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "../core/depth_layer.h"
#include "../core/game_object.h"
#include "../core/interface/updatable.h"
#include "../core/render/sdl_render_command_executor.h"
#include "../input/contracts/input_event_receiver.h"
#include "../input/contracts/input_snapshot_receiver.h"
#include "../ui/core/ui_element.h"

class Scene
{
public:
    Scene() = default;
    virtual ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) = default;
    Scene& operator=(Scene&&) = default;

    virtual void on_enter() = 0;
    virtual void on_exit() = 0;
    virtual void reset() = 0;

    virtual void on_update(double delta)
    {
        if (!_paused)
            update_objects(delta);

        remove_destroyed_objects();
    }

    virtual void on_render(SDL_Renderer* renderer)
    {
        std::vector<RenderCommand> render_commands;
        render_commands.reserve(512);

        for (const auto& layer : _object_layers)
        {
            render_commands.clear();

            for (const std::unique_ptr<GameObject>& object : layer)
            {
                if (!object || object->is_destroyed() || !object->is_visible())
                    continue;

                object->submit_render_commands(render_commands);
            }

            execute_render_commands(renderer, render_commands);
        }
    }

    virtual void on_input(
        const InputSnapshot& input,
        const std::vector<InputEvent>& events
    )
    {
        if (_paused)
            return;

        dispatch_input_snapshot(input);
        dispatch_input_events(events);
    }

    template <typename T, typename... Args>
    T* create_object(Args&&... args)
    {
        static_assert(std::is_base_of_v<GameObject, T>);

        return add_object(
            std::make_unique<T>(std::forward<Args>(args)...)
        );
    }

    template <typename T>
    T* add_object(std::unique_ptr<T> object)
    {
        static_assert(std::is_base_of_v<GameObject, T>);

        if (!object)
            return nullptr;

        T* raw_object = object.get();

        std::unique_ptr<GameObject> base_object = std::move(object);
        adopt_object(std::move(base_object));

        return raw_object;
    }

    void clear_objects()
    {
        for (auto& layer : _object_layers)
            layer.clear();

        clear_dispatch_entries();
    }

    template <typename T, typename... Args>
    T* create_ui_root(Args&&... args)
    {
        static_assert(std::is_base_of_v<UiElement, T>);

        return add_ui_root(
            std::make_unique<T>(std::forward<Args>(args)...)
        );
    }

    template <typename T>
    T* add_ui_root(std::unique_ptr<T> ui_root)
    {
        static_assert(std::is_base_of_v<UiElement, T>);

        if (!ui_root)
            return nullptr;

        T* raw_ui_root = ui_root.get();
        _ui_roots.push_back(std::move(ui_root));

        return raw_ui_root;
    }

    void clear_ui_roots()
    {
        _ui_roots.clear();
    }

    void pause() noexcept { _paused = true; }
    void resume() noexcept { _paused = false; }
    [[nodiscard]] bool is_paused() const noexcept { return _paused; }

protected:
    static constexpr size_t to_index(DepthLayer layer) noexcept
    {
        return static_cast<size_t>(layer);
    }

    void reset_objects()
    {
        for (auto& layer : _object_layers)
        {
            for (const std::unique_ptr<GameObject>& object : layer)
            {
                if (object)
                    object->reset();
            }
        }
    }

    void remove_destroyed_objects()
    {
        remove_destroyed_dispatch_entries();

        for (auto& layer : _object_layers)
        {
            std::erase_if(
                layer,
                [](const std::unique_ptr<GameObject>& object)
                {
                    return !object || object->is_destroyed();
                }
            );
        }
    }

private:
    struct UpdatableEntry
    {
        GameObject* object = nullptr;
        Updatable* updatable = nullptr;
    };

    struct InputSnapshotReceiverEntry
    {
        GameObject* object = nullptr;
        InputSnapshotReceiver* receiver = nullptr;
    };

    struct InputEventReceiverEntry
    {
        GameObject* object = nullptr;
        InputEventReceiver* receiver = nullptr;
    };

private:
    void adopt_object(std::unique_ptr<GameObject> object)
    {
        if (!object)
            return;

        GameObject* raw_object = object.get();
        const size_t layer_index = to_index(raw_object->depth_layer());

        register_object_interfaces(raw_object);

        std::vector<std::unique_ptr<GameObject>>& layer =
            _object_layers[layer_index];

        const int order = raw_object->order_in_layer();

        auto iter = std::lower_bound(
            layer.begin(),
            layer.end(),
            order,
            [](const std::unique_ptr<GameObject>& existing, int target_order)
            {
                return existing && existing->order_in_layer() < target_order;
            }
        );

        layer.insert(iter, std::move(object));
    }

    void register_object_interfaces(GameObject* object)
    {
        if (!object)
            return;

        const size_t layer_index = to_index(object->depth_layer());

        if (Updatable* updatable = dynamic_cast<Updatable*>(object))
        {
            insert_dispatch_entry(
                _updatable_layers[layer_index],
                UpdatableEntry{ object, updatable }
            );
        }

        if (InputSnapshotReceiver* receiver =
            dynamic_cast<InputSnapshotReceiver*>(object))
        {
            insert_dispatch_entry(
                _snapshot_receiver_layers[layer_index],
                InputSnapshotReceiverEntry{ object, receiver }
            );
        }

        if (InputEventReceiver* receiver =
            dynamic_cast<InputEventReceiver*>(object))
        {
            insert_dispatch_entry(
                _event_receiver_layers[layer_index],
                InputEventReceiverEntry{ object, receiver }
            );
        }
    }

    template <typename Entry>
    void insert_dispatch_entry(std::vector<Entry>& entries, const Entry& entry)
    {
        const int order = entry.object ? entry.object->order_in_layer() : 0;

        auto iter = std::lower_bound(
            entries.begin(),
            entries.end(),
            order,
            [](const Entry& existing, int target_order)
            {
                return existing.object
                    && existing.object->order_in_layer() < target_order;
            }
        );

        entries.insert(iter, entry);
    }

    void update_objects(double delta)
    {
        for (const auto& layer : _updatable_layers)
        {
            for (const UpdatableEntry& entry : layer)
            {
                GameObject* object = entry.object;

                if (!object || object->is_destroyed() || !object->is_active())
                    continue;

                entry.updatable->update(object->scaled_delta(delta));
            }
        }
    }

    void dispatch_input_snapshot(const InputSnapshot& input)
    {
        for (const auto& layer : _snapshot_receiver_layers)
        {
            for (const InputSnapshotReceiverEntry& entry : layer)
            {
                GameObject* object = entry.object;

                if (!object || object->is_destroyed() || !object->is_active())
                    continue;

                entry.receiver->on_input_snapshot(input);
            }
        }
    }

    void dispatch_input_events(const std::vector<InputEvent>& events)
    {
        for (const InputEvent& input_event : events)
        {
            bool consumed = false;

            for (const auto& layer : _event_receiver_layers)
            {
                for (const InputEventReceiverEntry& entry : layer)
                {
                    GameObject* object = entry.object;

                    if (!object || object->is_destroyed() || !object->is_active())
                        continue;

                    if (entry.receiver->on_input_event(input_event))
                    {
                        consumed = true;
                        break;
                    }
                }

                if (consumed)
                    break;
            }
        }
    }

    void clear_dispatch_entries()
    {
        for (auto& layer : _updatable_layers)
            layer.clear();

        for (auto& layer : _snapshot_receiver_layers)
            layer.clear();

        for (auto& layer : _event_receiver_layers)
            layer.clear();
    }

    void remove_destroyed_dispatch_entries()
    {
        for (auto& layer : _updatable_layers)
        {
            std::erase_if(layer, [](const UpdatableEntry& entry)
                {
                    return !entry.object || entry.object->is_destroyed();
                });
        }

        for (auto& layer : _snapshot_receiver_layers)
        {
            std::erase_if(layer, [](const InputSnapshotReceiverEntry& entry)
                {
                    return !entry.object || entry.object->is_destroyed();
                });
        }

        for (auto& layer : _event_receiver_layers)
        {
            std::erase_if(layer, [](const InputEventReceiverEntry& entry)
                {
                    return !entry.object || entry.object->is_destroyed();
                });
        }
    }

protected:
    bool _paused = false;

private:
    static constexpr size_t k_layer_count =
        static_cast<size_t>(DepthLayer::Count);

    std::array<std::vector<std::unique_ptr<GameObject>>, k_layer_count>
        _object_layers;

    std::array<std::vector<UpdatableEntry>, k_layer_count>
        _updatable_layers;

    std::array<std::vector<InputSnapshotReceiverEntry>, k_layer_count>
        _snapshot_receiver_layers;

    std::array<std::vector<InputEventReceiverEntry>, k_layer_count>
        _event_receiver_layers;

    std::vector<std::unique_ptr<UiElement>> _ui_roots;
};