#include "elysia_animation_loader.h"

namespace elysia::realm
{
void ElysiaAnimationLoader::start() noexcept
{
    _error_message.clear();
    _state = ElysiaAnimationLoaderState::Loading;
}

void ElysiaAnimationLoader::update() noexcept
{
}

void ElysiaAnimationLoader::unload() noexcept
{
    _error_message.clear();
    _state = ElysiaAnimationLoaderState::Unloaded;
}

ElysiaAnimationLoaderState ElysiaAnimationLoader::state() const noexcept
{
    return _state;
}

bool ElysiaAnimationLoader::is_loading() const noexcept
{
    return _state == ElysiaAnimationLoaderState::Loading;
}

bool ElysiaAnimationLoader::is_ready() const noexcept
{
    return _state == ElysiaAnimationLoaderState::Ready;
}

bool ElysiaAnimationLoader::has_failed() const noexcept
{
    return _state == ElysiaAnimationLoaderState::Failed;
}

const std::string& ElysiaAnimationLoader::error_message() const noexcept
{
    return _error_message;
}
}
