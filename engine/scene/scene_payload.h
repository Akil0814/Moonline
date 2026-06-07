#pragma once
#include <any>

using ScenePayload = std::any;

template <typename T>
const T& require_scene_payload(const ScenePayload& payload)
{
	return std::any_cast<const T&>(payload);
}
