#pragma once

#include "../input_snapshot.h"

namespace elysia::input
{
class InputSnapshotReceiver
{
public:
    virtual ~InputSnapshotReceiver() = default;
    virtual void on_input_snapshot(const InputSnapshot& input) = 0;
};

}
