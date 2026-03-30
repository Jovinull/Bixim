// =============================================================================
// Bixim — PC Display Driver (Declaration)
// File   : include/hal/DisplayPC.h
// Platform: PLATFORM_PC
// =============================================================================
// Implements IDisplay for the PC using Raylib.
//
// Rendering strategy:
//   The SSD1306 is 128x64. Rendering at native resolution on a modern monitor
//   produces a 128x64 pixel window — invisible. The driver scales each pixel
//   by PIXEL_SCALE, producing a window that faithfully represents the OLED.
//
//   Each pixel in the framebuffer is drawn as a filled rectangle of
//   PIXEL_SCALE x PIXEL_SCALE screen pixels using Raylib's DrawRectangle().
//
// Color mapping:
//   bit = 1 (white) → PIXEL_COLOR_ON  (white on black, like a real OLED)
//   bit = 0 (black) → background (not drawn, window is cleared to black)
// =============================================================================
#pragma once

#ifdef PLATFORM_PC

#include "IDisplay.h"
#include "FrameBuffer.h"

class DisplayPC final : public IDisplay {
public:
    static constexpr int PIXEL_SCALE = 6; // Each OLED pixel = 6x6 screen pixels

    DisplayPC() = default;

    bool Init()                       override;
    void Flush(const FrameBuffer& fb) override;
    bool IsRunning()            const override;
    void Shutdown()                   override;
};

#endif // PLATFORM_PC
