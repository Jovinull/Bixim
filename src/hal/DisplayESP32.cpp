// =============================================================================
// Bixim — ESP32 Display Driver (Definition)
// File   : src/hal/DisplayESP32.cpp
// Platform: PLATFORM_ESP32
// =============================================================================
#ifdef PLATFORM_ESP32

#include "hal/DisplayESP32.h"
#include <Arduino.h>
#include <string.h>

bool DisplayESP32::Init()
{
    if (!m_display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS)) {
        Serial.println(F("[Bixim] CRITICAL: SSD1306 init failed. Check I2C wiring and address."));
        return false;
    }

    // The Adafruit library draws an Adafruit logo on startup.
    // Clear it immediately so the engine starts with a blank screen.
    m_display.clearDisplay();
    m_display.display();

    Serial.println(F("[Bixim] SSD1306 display initialized."));
    return true;
}

void DisplayESP32::Flush(const FrameBuffer& fb)
{
    // The Adafruit SSD1306 library exposes its internal buffer via getBuffer().
    // Both buffers use the same SSD1306 page-major layout, so a direct memcpy
    // is valid — no format conversion is required.
    //
    // After the copy, display() triggers the I2C transfer (DMA on ESP32),
    // sending all 1024 bytes to the OLED controller in one transaction.
    memcpy(m_display.getBuffer(), fb.data, FRAMEBUFFER_SIZE);
    m_display.display();
}

void DisplayESP32::Shutdown()
{
    // No hardware shutdown sequence required for the SSD1306.
    // Power is cut at the hardware level via the slide switch.
}

#endif // PLATFORM_ESP32
