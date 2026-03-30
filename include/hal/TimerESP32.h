// =============================================================================
// Bixim — ESP32 Timer Implementation (Declaration)
// File   : include/hal/TimerESP32.h
// Platform: PLATFORM_ESP32 (Arduino framework, ESP-IDF underneath)
// =============================================================================
// Uses esp_timer_get_time() from the ESP-IDF high-resolution timer API.
//
// esp_timer_get_time() characteristics:
//   - Returns int64_t microseconds since boot.
//   - Backed by a 64-bit hardware counter. Monotonic by design.
//   - Resolution: 1 µs. Accuracy: ±2 µs typical.
//   - Does NOT stop during light sleep. Stops during deep sleep.
//     (We will handle deep sleep wake corrections in a future module.)
//
// Why not millis()?
//   millis() returns uint32_t milliseconds, which wraps after ~49 days.
//   More importantly, 1ms granularity is too coarse for our 100ms logic tick
//   to remain accurate over long sessions.
// =============================================================================
#pragma once

#ifdef PLATFORM_ESP32

#include "ITimer.h"
#include <esp_timer.h>

class TimerESP32 final : public ITimer {
public:
    TimerESP32() = default;

    // Returns microseconds elapsed since ESP32 boot.
    // Casts int64_t to uint64_t — safe because esp_timer_get_time() is
    // always non-negative after boot.
    uint64_t GetMicroseconds() const override;

    // Yields using delayMicroseconds() from the Arduino framework.
    // Note: delayMicroseconds() busy-waits for values < ~20µs and uses
    // vTaskDelay for larger values. For values above 1ms, prefer the FreeRTOS
    // vTaskDelay(pdMS_TO_TICKS(ms)) directly for power efficiency.
    void SleepMicroseconds(uint64_t us) const override;
};

#endif // PLATFORM_ESP32
