// =============================================================================
// Bixim — HAL Timer Interface
// File   : include/hal/ITimer.h
// =============================================================================
// Abstract interface for all platform-specific timer implementations.
//
// Design contract:
//   - GetMicroseconds() MUST return a monotonically increasing value.
//     It must never go backwards. It is allowed to wrap after ~584,000 years
//     (uint64_t overflow at 1 µs resolution).
//   - The zero point is arbitrary (boot time, program start, etc.).
//     Only differences between two calls carry meaning.
//   - SleepMicroseconds() is a best-effort hint to the scheduler. The actual
//     sleep may be longer due to OS jitter. The game loop must NOT depend on
//     this function for correctness — it is only used for FPS capping.
// =============================================================================
#pragma once

#include <cstdint>

class ITimer {
public:
    virtual ~ITimer() = default;

    // Returns elapsed time since an arbitrary epoch, in microseconds.
    // Must be monotonic. Thread-safe reads are not required for this project.
    virtual uint64_t GetMicroseconds() const = 0;

    // Yields the CPU for approximately the requested duration.
    // Used only for render FPS capping — never for logic timing correctness.
    virtual void SleepMicroseconds(uint64_t us) const = 0;
};
