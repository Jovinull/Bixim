// =============================================================================
// Bixim — ESP32 Input Driver (Definition)
// File   : src/hal/InputESP32.cpp
// Platform: PLATFORM_ESP32
// =============================================================================
#ifdef PLATFORM_ESP32

#include "hal/InputESP32.h"
#include <Arduino.h>

bool InputESP32::Init()
{
    // Configure all button pins as INPUT_PULLUP.
    // The internal pull-up resistor (~45kΩ) holds each pin at 3.3V (HIGH)
    // when no button is pressed. Pressing shorts the pin to GND (LOW).
    // This is the active-LOW convention used by Tamalib's original hardware.
    for (int i = 0; i < static_cast<int>(Button::COUNT); ++i) {
        pinMode(m_buttons[i].pin, INPUT_PULLUP);

        // Read the initial raw state to avoid a spurious "just_pressed" on
        // the first Update() call if a button happens to be held at boot.
        m_buttons[i].raw_last       = (digitalRead(m_buttons[i].pin) == LOW);
        m_buttons[i].debounced      = m_buttons[i].raw_last;
        m_buttons[i].just_pressed   = false;
        m_buttons[i].last_change_ms = millis();
    }
    return true;
}

void InputESP32::Update()
{
    for (int i = 0; i < static_cast<int>(Button::COUNT); ++i) {
        updateButton(m_buttons[i]);
    }
}

void InputESP32::updateButton(ButtonState& state)
{
    // -------------------------------------------------------------------------
    // Step 1 — Raw edge detection
    // -------------------------------------------------------------------------
    // Read the physical pin. Active-LOW: LOW = pressed (true), HIGH = released (false).
    const bool raw_now = (digitalRead(state.pin) == LOW);

    if (raw_now != state.raw_last) {
        // The raw signal changed (rising or falling edge detected).
        // This could be a legitimate press/release OR the start of a bounce
        // burst. Either way, reset the stability timer.
        //
        // We do NOT accept the new state yet — we only start measuring
        // how long it stays stable.
        state.last_change_ms = millis();
        state.raw_last       = raw_now;

        // Safety: also clear just_pressed whenever we see a raw edge.
        // This prevents a stale just_pressed from leaking into the next tick
        // if Update() is called more than once between logic ticks.
        state.just_pressed = false;
    }

    // -------------------------------------------------------------------------
    // Step 2 — Stability check (the debounce gate)
    // -------------------------------------------------------------------------
    // Only proceed if raw_now has been stable for at least DEBOUNCE_MS.
    //
    // Unsigned subtraction is intentional: if millis() rolls over (every ~49
    // days), (millis() - last_change_ms) still yields the correct elapsed time
    // because uint32_t wraps around correctly.
    //
    //   Example of correct rollover handling:
    //     last_change_ms = 0xFFFFFF00 (close to rollover)
    //     millis()       = 0x00000032 (just rolled over, = 50 decimal)
    //     elapsed        = 0x00000032 - 0xFFFFFF00 = 0x00000132 = 306 ms ✓
    //
    const uint32_t elapsed_ms = millis() - state.last_change_ms;

    if (elapsed_ms < DEBOUNCE_MS) {
        // Signal has not yet been stable long enough.
        // Do not change the debounced state. Do not fire just_pressed.
        // Any bounce pulses that occurred within this window are ignored.
        return;
    }

    // -------------------------------------------------------------------------
    // Step 3 — Accept the stable state and detect edges
    // -------------------------------------------------------------------------
    // The signal has been stable for >= DEBOUNCE_MS. We can trust raw_now.

    // Rising edge: button was released (false) and is now confirmed pressed (true).
    // This fires just_pressed for exactly ONE call to Update() — the first one
    // after the debounce window closes on a press event.
    state.just_pressed = (!state.debounced && raw_now);

    // Update the accepted stable state.
    state.debounced = raw_now;
}

bool InputESP32::IsPressed(Button b) const
{
    return m_buttons[static_cast<int>(b)].just_pressed;
}

bool InputESP32::IsHeld(Button b) const
{
    return m_buttons[static_cast<int>(b)].debounced;
}

#endif // PLATFORM_ESP32
