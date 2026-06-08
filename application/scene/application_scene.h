#pragma once

#include "../../engine/scene/scene.h"
#include "../input/gameplay_input_router.h"
#include "../input/input_context.h"
#include "../input/ui_input_router.h"

#include "../../engine/ui/input/contracts/ui_input_event_receiver.h"
#include "../../engine/ui/input/contracts/ui_input_frame_receiver.h"
#include "../../gameplay/input/contracts/gameplay_input_event_receiver.h"
#include "../../gameplay/input/contracts/gameplay_input_frame_receiver.h"

#include <vector>

class ApplicationScene : public Scene
{
public:
    void on_input(const RawInputFrame& input, const std::vector<RawInputEvent>& events) override;

protected:
    [[nodiscard]] virtual InputContext input_context() const;
    void on_scene_object_registered(SceneObject& object) override;

private:
    struct UiInputFrameReceiverEntry
    {
        SceneObject* object = nullptr;
        UiInputFrameReceiver* receiver = nullptr;
    };

    struct UiInputEventReceiverEntry
    {
        SceneObject* object = nullptr;
        UiInputEventReceiver* receiver = nullptr;
    };

    struct GameplayInputFrameReceiverEntry
    {
        SceneObject* object = nullptr;
        GameplayInputFrameReceiver* receiver = nullptr;
    };

    struct GameplayInputEventReceiverEntry
    {
        SceneObject* object = nullptr;
        GameplayInputEventReceiver* receiver = nullptr;
    };

private:
    void prune_domain_receivers();
    void dispatch_ui_frame(const UiInputFrame& input);
    void dispatch_ui_events(const std::vector<UiInputEvent>& events);
    void dispatch_gameplay_frame(const GameplayInputFrame& input);
    void dispatch_gameplay_events(const std::vector<GameplayInputEvent>& events);

private:
    UiInputRouter _ui_input_router;
    GameplayInputRouter _gameplay_input_router;
    std::vector<UiInputFrameReceiverEntry> _ui_frame_receivers;
    std::vector<UiInputEventReceiverEntry> _ui_event_receivers;
    std::vector<GameplayInputFrameReceiverEntry> _gameplay_frame_receivers;
    std::vector<GameplayInputEventReceiverEntry> _gameplay_event_receivers;
};
