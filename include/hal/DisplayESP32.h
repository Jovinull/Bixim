// =============================================================================
// Bixim — ESP32 Display Driver (Declaration)
// File   : include/hal/DisplayESP32.h
// Platform: PLATFORM_ESP32
// =============================================================================
// Implements IDisplay for the ESP32 using the Adafruit SSD1306 library.
//
// Transfer strategy:
//   The Adafruit library maintains its own internal buffer (also 1024 bytes).
//   Our Flush() copies the engine's FrameBuffer into the Adafruit buffer via
//   memcpy, then calls display.display() to trigger the I2C DMA transfer.
//
//   This means we carry two copies of the framebuffer in RAM:
//     - fb.data[1024]           — engine's buffer (drawn into by the game)
//     - Adafruit internal[1024] — library's buffer (sent to hardware via I2C)
//   Total SRAM cost: 2 KiB. Acceptable on the ESP32 (520 KiB available).
//
//   A future optimization could bypass the Adafruit buffer entirely and write
//   directly to the SSD1306 via I2C using Wire, saving 1 KiB of RAM and one
//   memcpy per frame.
// =============================================================================
#pragma once

#ifdef PLATFORM_ESP32

#include "IDisplay.h"
#include "FrameBuffer.h"
#include <Adafruit_SSD1306.h>

class DisplayESP32 final : public IDisplay {
public:
    static constexpr uint8_t I2C_ADDRESS = 0x3C;

    DisplayESP32() = default;

    bool Init()                       override;
    void Flush(const FrameBuffer& fb) override;
    bool IsRunning()            const override { return true; }
    void Shutdown()                   override;

private:
    Adafruit_SSD1306 m_display{DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1};
};

#endif // PLATFORM_ESP32
