// =============================================================================
// Bixim — PC Timer Implementation (Definition)
// File   : src/hal/TimerPC.cpp
// Platform: PLATFORM_PC
// =============================================================================
#ifdef PLATFORM_PC

#include "hal/TimerPC.h"
#include <thread>

TimerPC::TimerPC()
    : m_origin(Clock::now())
{
}

uint64_t TimerPC::GetMicroseconds() const
{
    // Equation:
    //   elapsed = t_now - m_origin       [std::chrono::duration]
    //   result  = elapsed cast to µs     [uint64_t]
    //
    // duration_cast truncates towards zero — acceptable for timing purposes
    // since we always compute differences, not absolute values.
    const TimePoint now = Clock::now();
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(now - m_origin).count()
    );
}

void TimerPC::SleepMicroseconds(uint64_t us) const
{
    // std::this_thread::sleep_for is the most portable sleep on Windows/Linux.
    // On Windows without timeBeginPeriod(1), the minimum sleep granularity is
    // ~15.6ms. This means our FPS cap may not be perfectly tight at 30 FPS.
    // For a debug/development build this is acceptable. A production Windows
    // port could call timeBeginPeriod(1) at startup to improve this.
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

#endif // PLATFORM_PC
