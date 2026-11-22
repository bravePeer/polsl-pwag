#pragma once

enum class InputButtonId : unsigned int
{
    // Keyboard
    W,
    A,
    S,
    D,
    Esc,

    // Mouse
    MouseLeft,
    MouseRight,
    MouseMiddle,

    Count
};

enum class InputAction
{
    Release,
    Press,
    Repeat
};
