// =============================================================================
// Bixim — PC Input Driver (Declaration)
// File   : include/hal/InputPC.h
// Platform: PLATFORM_PC
// =============================================================================
// Maps keyboard keys to the three Bixim buttons using Raylib's input API.
//
// Key mapping (chosen to be comfortable for one-handed play on keyboard):
//   Button::A (Select)  → Z key  or LEFT  arrow
//   Button::B (Confirm) → X key  or ENTER
//   Button::C (Cancel)  → C key  or RIGHT arrow
//
// Why no manual debounce?
//   The OS keyboard driver already performs hardware debouncing at the
//   controller level. Raylib's IsKeyPressed() queries the OS event queue,
//   which fires exactly once per physical key press regardless of how long
//   the key is held. IsKeyDown() queries the current scancode state, also
//   clean. No additional filtering is required on the PC.
// =============================================================================
#pragma once

#ifdef PLATFORM_PC

#include "IInput.h"
#include <cstdint>

class InputPC final : public IInput {
public:
    InputPC() = default;

    bool Init()               override;
    void Update()             override;
    bool IsPressed(Button b)  const override;
    bool IsHeld(Button b)     const override;

private:
    // Raylib key codes for each button (primary and alternate binding).
    // Stored as int to avoid including raylib.h in this header.
    // Resolved in InputPC.cpp where raylib.h is included.
    static const int KEY_A_PRIMARY;
    static const int KEY_A_ALT;
    static const int KEY_B_PRIMARY;
    static const int KEY_B_ALT;
    static const int KEY_C_PRIMARY;
    static const int KEY_C_ALT;

    // Cached results from the last Update() call.
    // Index corresponds to Button enum value (0=A, 1=B, 2=C).
    bool m_pressed[static_cast<int>(Button::COUNT)] = {};
    bool m_held   [static_cast<int>(Button::COUNT)] = {};
};

#endif // PLATFORM_PC
