#pragma once

class SceneManagerObserver
{
public:
    virtual ~SceneManagerObserver() = default;

    virtual void on_scene_manager_quit_requested() = 0;
};
