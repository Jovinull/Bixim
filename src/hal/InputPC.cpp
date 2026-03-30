// =============================================================================
// Bixim — PC Input Driver (Definition)
// File   : src/hal/InputPC.cpp
// Platform: PLATFORM_PC
// =============================================================================
#ifdef PLATFORM_PC

#include "hal/InputPC.h"
#include <raylib.h>

// Key bindings — primary and alternate for each button.
// Using Raylib's KEY_* constants (defined in raylib.h).
const int InputPC::KEY_A_PRIMARY = KEY_Z;
const int InputPC::KEY_A_ALT     = KEY_LEFT;
const int InputPC::KEY_B_PRIMARY = KEY_X;
const int InputPC::KEY_B_ALT     = KEY_ENTER;
const int InputPC::KEY_C_PRIMARY = KEY_C;
const int InputPC::KEY_C_ALT     = KEY_RIGHT;

bool InputPC::Init()
{
    // Nothing to initialize for keyboard input on PC.
    // Raylib handles the OS event queue automatically after InitWindow().
    return true;
}

void InputPC::Update()
{
    // Button A
    // IsKeyPressed(): fires true for exactly one frame per physical key press.
    // Two bindings are OR'd — either key activates the button.
    m_pressed[0] = IsKeyPressed(KEY_A_PRIMARY) || IsKeyPressed(KEY_A_ALT);
    m_held   [0] = IsKeyDown(KEY_A_PRIMARY)    || IsKeyDown(KEY_A_ALT);

    // Button B
    m_pressed[1] = IsKeyPressed(KEY_B_PRIMARY) || IsKeyPressed(KEY_B_ALT);
    m_held   [1] = IsKeyDown(KEY_B_PRIMARY)    || IsKeyDown(KEY_B_ALT);

    // Button C
    m_pressed[2] = IsKeyPressed(KEY_C_PRIMARY) || IsKeyPressed(KEY_C_ALT);
    m_held   [2] = IsKeyDown(KEY_C_PRIMARY)    || IsKeyDown(KEY_C_ALT);
}

bool InputPC::IsPressed(Button b) const
{
    return m_pressed[static_cast<int>(b)];
}

bool InputPC::IsHeld(Button b) const
{
    return m_held[static_cast<int>(b)];
}

#endif // PLATFORM_PC
