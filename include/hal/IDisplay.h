// =============================================================================
// Bixim — HAL Display Interface
// File   : include/hal/IDisplay.h
// =============================================================================
// Abstract interface for platform-specific display drivers.
//
// Responsibilities of each concrete implementation:
//   - Initialize the physical display hardware (I2C, SPI, window system, etc.)
//   - Accept a call to Flush() and transfer the engine's framebuffer to the
//     display hardware using whatever protocol is appropriate for the platform.
//
// The engine (FrameBuffer) is completely decoupled from the hardware. It draws
// into a software buffer and calls IDisplay::Flush() once per render frame.
// The display driver is responsible for the last mile only.
// =============================================================================
#pragma once

#include <cstdint>

// Forward declaration — the FrameBuffer struct is defined in FrameBuffer.h.
// IDisplay only needs a pointer to it, so a forward declaration avoids a
// circular dependency between the two headers.
struct FrameBuffer;

class IDisplay {
public:
    virtual ~IDisplay() = default;

    // Initializes the display hardware. Returns true on success.
    // Must be called once before any Flush().
    virtual bool Init() = 0;

    // Transfers the contents of the framebuffer to the physical display.
    // Called once per render frame by the game loop.
    // The implementation must NOT modify the framebuffer contents.
    virtual void Flush(const FrameBuffer& fb) = 0;

    // Returns false when the display signals the application should exit.
    // On PC: wraps Raylib's WindowShouldClose().
    // On ESP32: always returns true (the device never "closes").
    virtual bool IsRunning() const = 0;

    // Shuts down the display hardware (PC: closes window; ESP32: no-op).
    virtual void Shutdown() = 0;
};
