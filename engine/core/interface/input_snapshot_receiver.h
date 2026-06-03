#include "../../input/input_system.h"
class InputSnapshotReceiver
{
public:
    virtual ~InputSnapshotReceiver() = default;
    virtual void on_input_snapshot(const InputSnapshot& input) = 0;
};