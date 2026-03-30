// =============================================================================
// Bixim — ESP32 Input Driver (Declaration)
// File   : include/hal/InputESP32.h
// Platform: PLATFORM_ESP32
// =============================================================================
// Reads three GPIO pins with INPUT_PULLUP and applies software debouncing.
//
// GPIO pin assignment (ESP32-WROOM-32):
//   Button::A (Select)  → GPIO 25
//   Button::B (Confirm) → GPIO 26
//   Button::C (Cancel)  → GPIO 27
//
// Pin selection criteria:
//   - Must support INPUT_PULLUP (pins 34-39 are input-only with no internal
//     pull resistor — excluded).
//   - Must not interfere with boot strapping:
//       GPIO 0  → boot mode selection (must be HIGH at reset) — excluded
//       GPIO 2  → connected to onboard LED, affects boot on some boards — excluded
//       GPIO 12 → MTDI, sets flash voltage (must be LOW at boot on WROOM) — excluded
//       GPIO 15 → MTDO, controls boot log output — excluded
//   - Pins 25, 26, 27 are general-purpose, safe, and physically adjacent on
//     most WROOM-32 breakout boards, simplifying PCB routing.
//
// Active-LOW convention (matches Tamalib's hardware model):
//   Button released → pin reads HIGH (pull-up holds it there)
//   Button pressed  → pin reads LOW  (switch shorts pin to GND)
//
// Debounce strategy — Stable-Window / Time-Hysteresis:
//   A state change is only accepted when the raw signal has remained stable
//   (unchanged) for at least DEBOUNCE_MS consecutive milliseconds.
//   This rejects all bounce pulses, which are typically shorter than 5ms,
//   while reliably detecting intentional presses (human reaction >> 50ms).
//
//   State machine per button (evaluated in Update()):
//
//     raw_now = (digitalRead(pin) == LOW)  ← true if physically pressed
//
//     if (raw_now != raw_last):
//         last_change_ms = millis()        ← reset stability timer on any change
//         raw_last = raw_now
//
//     if ((millis() - last_change_ms) >= DEBOUNCE_MS):
//         just_pressed = (!debounced && raw_now)  ← rising edge, post-debounce
//         debounced    = raw_now                  ← accept stable state
//
// Why millis() instead of ITimer here?
//   millis() returns uint32_t ms, which is sufficient for debounce intervals
//   and avoids threading the ITimer dependency into the input module.
//   The debounce timer is reset on raw edge detection, so the 49-day
//   millis() rollover is handled correctly by unsigned subtraction.
// =============================================================================
#pragma once

#ifdef PLATFORM_ESP32

#include "IInput.h"
#include <cstdint>

class InputESP32 final : public IInput {
public:
    // Debounce threshold in milliseconds.
    // 50ms comfortably exceeds worst-case mechanical bounce (~20ms)
    // while remaining imperceptible to the user (human reaction ~150ms).
    static constexpr uint32_t DEBOUNCE_MS = 50;

    // GPIO pin numbers for each button.
    static constexpr int PIN_A = 25;
    static constexpr int PIN_B = 26;
    static constexpr int PIN_C = 27;

    InputESP32() = default;

    bool Init()               override;
    void Update()             override;
    bool IsPressed(Button b)  const override;
    bool IsHeld(Button b)     const override;

private:
    // Internal state tracked per button by the debounce state machine.
    struct ButtonState {
        int      pin;             // GPIO pin number
        bool     raw_last;        // raw reading on the previous Update() call
        bool     debounced;       // last accepted stable state (post-debounce)
        bool     just_pressed;    // true for exactly one tick (rising edge)
        uint32_t last_change_ms;  // millis() snapshot of last raw edge
    };

    ButtonState m_buttons[static_cast<int>(Button::COUNT)] = {
        { PIN_A, false, false, false, 0 },
        { PIN_B, false, false, false, 0 },
        { PIN_C, false, false, false, 0 },
    };

    // Runs the debounce state machine for one button. Called by Update().
    void updateButton(ButtonState& state);
};

#endif // PLATFORM_ESP32
