#pragma once

#include "ui_input_types.h"

#include "../../input/raw_input_frame.h"

#include <optional>

class UiGamepadScrollSynthesizer
{
public:
    std::optional<UiInputEvent> synthesize(const RawInputFrame& input);
    void reset();

private:
    [[nodiscard]] float normalize_axis(float axis_value) const;

private:
    float _scroll_accumulator = 0.0f;
};
