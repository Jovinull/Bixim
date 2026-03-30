// =============================================================================
// Bixim — HAL Input Interface
// File   : include/hal/IInput.h
// =============================================================================
// Abstract interface for all platform-specific input drivers.
//
// Design contract:
//   - Update() MUST be called once per logic tick, before any IsPressed() or
//     IsHeld() queries. It samples the raw hardware state and advances the
//     debounce state machine.
//   - IsPressed() returns true for exactly ONE logic tick — the tick on which
//     the button transitioned from released to held. Use this for navigation,
//     menu selection, and any action that must fire once per press.
//   - IsHeld() returns true for every logic tick that the button remains
//     physically held. Use this for continuous actions (e.g., scroll, charge).
//   - The FSM has no knowledge of debouncing, GPIO pins, or keyboard scancodes.
//     It works exclusively through this interface.
//
// Debouncing responsibility:
//   - PC (Raylib): no manual debounce needed. IsKeyPressed() in Raylib already
//     fires exactly once per physical key press, handled by the OS key-repeat
//     and event queue. IsKeyDown() is also clean.
//   - ESP32 (GPIO): mechanical bounce must be filtered in InputESP32::Update().
//     See InputESP32.h for the implementation strategy.
// =============================================================================
#pragma once

#include <cstdint>

// The three physical buttons available on the Bixim device.
// Mapped from Tamalib's BTN_LEFT / BTN_MIDDLE / BTN_RIGHT:
//   Button::A = Select / Navigate left   (BTN_LEFT  → K02)
//   Button::B = Confirm / Action         (BTN_MIDDLE → K01)
//   Button::C = Cancel / Navigate right  (BTN_RIGHT  → K00)
enum class Button : uint8_t {
    A = 0,
    B = 1,
    C = 2,
    COUNT = 3   // sentinel — number of buttons, used for array sizing
};

class IInput {
public:
    virtual ~IInput() = default;

    // Initializes the input hardware (GPIO config, key maps, etc.).
    // Must be called once before any Update() or query.
    virtual bool Init() = 0;

    // Samples hardware state and advances the debounce state machine.
    // Must be called exactly once per logic tick, at the start of UpdateLogic().
    virtual void Update() = 0;

    // Returns true on the single logic tick when the button transitioned
    // from released → held (rising edge, post-debounce).
    // Subsequent ticks while held return false.
    virtual bool IsPressed(Button b) const = 0;

    // Returns true on every logic tick that the button is physically held
    // (post-debounce). Becomes false on the tick after release.
    virtual bool IsHeld(Button b) const = 0;
};
