// =============================================================================
// Bixim — PC Timer Implementation (Declaration)
// File   : include/hal/TimerPC.h
// Platform: PLATFORM_PC (Windows / Linux / macOS via MSYS2 MinGW-w64)
// =============================================================================
// Uses std::chrono::steady_clock — the C++ standard monotonic clock.
//
// Why steady_clock and not system_clock?
//   system_clock tracks wall-clock time and can jump backwards when the user
//   adjusts the system time or when NTP syncs. steady_clock is guaranteed to
//   be monotonically increasing for the lifetime of the process, making it
//   safe for delta-time calculations.
//
// Why not Raylib's GetTime()?
//   GetTime() returns a double (seconds), which loses sub-microsecond
//   precision after ~9 hours of runtime due to floating-point mantissa
//   limits. uint64_t microseconds has no such drift within any sane session.
// =============================================================================
#pragma once

#ifdef PLATFORM_PC

#include "ITimer.h"
#include <chrono>

class TimerPC final : public ITimer {
public:
    // Records the origin timestamp at construction time.
    TimerPC();

    // Returns microseconds elapsed since this object was constructed.
    uint64_t GetMicroseconds() const override;

    // Sleeps for approximately the requested number of microseconds.
    // Uses std::this_thread::sleep_for — subject to OS scheduler granularity
    // (typically ~1ms on Windows without high-resolution timer adjustments).
    void SleepMicroseconds(uint64_t us) const override;

private:
    using Clock     = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    TimePoint m_origin;
};

#endif // PLATFORM_PC
