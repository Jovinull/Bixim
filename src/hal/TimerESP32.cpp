// =============================================================================
// Bixim — ESP32 Timer Implementation (Definition)
// File   : src/hal/TimerESP32.cpp
// Platform: PLATFORM_ESP32
// =============================================================================
#ifdef PLATFORM_ESP32

#include "hal/TimerESP32.h"
#include <Arduino.h>

uint64_t TimerESP32::GetMicroseconds() const
{
    // esp_timer_get_time() returns int64_t. It is always >= 0 after boot,
    // so the cast to uint64_t is safe and preserves the full 64-bit range.
    return static_cast<uint64_t>(esp_timer_get_time());
}

void TimerESP32::SleepMicroseconds(uint64_t us) const
{
    // For values >= 1000 µs (1 ms), delegate to vTaskDelay for power savings.
    // For values < 1000 µs, delayMicroseconds busy-waits with a tight loop.
    //
    // In the Bixim game loop, SleepMicroseconds is only called to cap the
    // render rate. On ESP32 the SSD1306 I2C transfer itself limits throughput
    // to well below 30 FPS, so this path is rarely exercised in practice.
    if (us >= 1000) {
        vTaskDelay(pdMS_TO_TICKS(us / 1000));
    } else {
        delayMicroseconds(static_cast<uint32_t>(us));
    }
}

#endif // PLATFORM_ESP32
