// =============================================================================
// Bixim — Main Entry Point & Game Loop
// File   : src/main.cpp
// =============================================================================
// Game loop architecture: Fixed Timestep Accumulator
//
// Two independent tick domains:
//   LOGIC_HZ   = 10 Hz  → LOGIC_TICK_US = 100,000 µs per logic step
//   TARGET_FPS = 30     → FRAME_BUDGET_US = 33,333 µs per render frame
//
// Rendering pipeline:
//   Game logic draws into a FrameBuffer (1024 bytes, software).
//   At the end of each render frame, IDisplay::Flush() transfers the
//   buffer to the physical display (Raylib window on PC, SSD1306 on ESP32).
// =============================================================================

// -----------------------------------------------------------------------------
// Platform-specific includes
// -----------------------------------------------------------------------------
#ifdef PLATFORM_PC
    #include <cstdio>
    #include "hal/TimerPC.h"
    #include "hal/DisplayPC.h"
#endif

#ifdef PLATFORM_ESP32
    #include <Arduino.h>
    #include <Wire.h>
    #include "hal/TimerESP32.h"
    #include "hal/DisplayESP32.h"
#endif

#include "hal/ITimer.h"
#include "hal/IDisplay.h"
#include "hal/FrameBuffer.h"

// =============================================================================
// Game Loop Constants
// =============================================================================
static constexpr uint32_t LOGIC_HZ                  = 10;
static constexpr uint64_t LOGIC_TICK_US              = 1'000'000ULL / LOGIC_HZ;
static constexpr uint32_t TARGET_FPS                 = 30;
static constexpr uint64_t FRAME_BUDGET_US            = 1'000'000ULL / TARGET_FPS;
static constexpr uint32_t MAX_LOGIC_TICKS_PER_FRAME  = 5;

// =============================================================================
// Demo sprite data — 16x16 pixels, horizontal format (MSB = leftmost pixel)
// =============================================================================
// A simple face for display testing. Each row is 2 bytes (16 pixels).
// Replace this with real pet sprite data in the Sprite module.
static const uint8_t SPRITE_FACE[16 * 2] = {
    0b00111100, 0b00111100, // row  0
    0b01000010, 0b01000010, // row  1
    0b10100101, 0b10100101, // row  2  (eyes)
    0b10000001, 0b10000001, // row  3
    0b10000001, 0b10000001, // row  4
    0b10100101, 0b10100101, // row  5  (nostrils)
    0b10011001, 0b10011001, // row  6  (mouth)
    0b01000010, 0b01000010, // row  7
    0b00111100, 0b00111100, // row  8
    0b00000000, 0b00000000, // row  9
    0b00000000, 0b00000000, // row 10
    0b00000000, 0b00000000, // row 11
    0b00000000, 0b00000000, // row 12
    0b00000000, 0b00000000, // row 13
    0b00000000, 0b00000000, // row 14
    0b00000000, 0b00000000, // row 15
};

// Mask: all pixels inside the sprite bounding circle are opaque.
// Outside pixels are transparent (0) so the background shows through.
static const uint8_t MASK_FACE[16 * 2] = {
    0b00111100, 0b00111100,
    0b01111110, 0b01111110,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b11111111, 0b11111111,
    0b01111110, 0b01111110,
    0b00111100, 0b00111100,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
};

// =============================================================================
// Forward declarations — game modules
// =============================================================================
void GameInit(IDisplay& display, FrameBuffer& fb);
void UpdateLogic(uint64_t tick_count, FrameBuffer& fb);
void RenderGraphics(IDisplay& display, const FrameBuffer& fb);
void GameShutdown(IDisplay& display);

// =============================================================================
// Stub implementations
// =============================================================================
void GameInit(IDisplay& display, FrameBuffer& fb)
{
    if (!display.Init()) {
        // On ESP32 Init() already prints the error. On PC, Raylib opens the window.
        // If Init() fails on ESP32, halt here.
#ifdef PLATFORM_ESP32
        for (;;) {}
#endif
    }
    fb.Clear();
}

void UpdateLogic(uint64_t tick_count, FrameBuffer& fb)
{
    fb.Clear();

    // Draw a static checkerboard border as background.
    for (int x = 0; x < DISPLAY_WIDTH; ++x) {
        drawPixel(fb, x, 0,                 true);
        drawPixel(fb, x, DISPLAY_HEIGHT - 1, true);
    }
    for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
        drawPixel(fb, 0,                   y, true);
        drawPixel(fb, DISPLAY_WIDTH - 1,   y, true);
    }

    // Animate the sprite position horizontally using tick_count.
    // The face bounces between X=10 and X=102 (128 - 16 - 10).
    const int range  = DISPLAY_WIDTH - 16 - 20; // 92
    const int period = LOGIC_HZ * 4;             // 4 seconds per cycle
    const int phase  = static_cast<int>(tick_count % (period * 2));
    const int offset = (phase < period) ? phase : (period * 2 - phase);
    const int sprite_x = 10 + (offset * range / period);
    const int sprite_y = (DISPLAY_HEIGHT - 16) / 2; // vertically centered

    drawSprite(fb, sprite_x, sprite_y,
               SPRITE_FACE, MASK_FACE, 16, 16);
}

void RenderGraphics(IDisplay& display, const FrameBuffer& fb)
{
    display.Flush(fb);
}

void GameShutdown(IDisplay& display)
{
    display.Shutdown();
}

// =============================================================================
// PLATFORM: PC
// =============================================================================
#ifdef PLATFORM_PC

int main(void)
{
    TimerPC    timer;
    DisplayPC  display;
    FrameBuffer fb;

    GameInit(display, fb);

    uint64_t t_last    = timer.GetMicroseconds();
    uint64_t accum     = 0;
    uint64_t tick_count = 0;

    while (display.IsRunning()) {
        // --- 1. Delta time ---
        const uint64_t t_now  = timer.GetMicroseconds();
        uint64_t delta = t_now - t_last;
        t_last = t_now;

        const uint64_t MAX_DELTA = MAX_LOGIC_TICKS_PER_FRAME * LOGIC_TICK_US;
        if (delta > MAX_DELTA) delta = MAX_DELTA;

        // --- 2. Accumulate ---
        accum += delta;

        // --- 3. Fixed-rate logic ticks ---
        uint32_t ticks_this_frame = 0;
        while (accum >= LOGIC_TICK_US && ticks_this_frame < MAX_LOGIC_TICKS_PER_FRAME) {
            UpdateLogic(tick_count, fb);
            accum -= LOGIC_TICK_US;
            ++tick_count;
            ++ticks_this_frame;
        }

        // --- 4. Render ---
        RenderGraphics(display, fb);

        // --- 5. FPS cap ---
        const uint64_t frame_cost = timer.GetMicroseconds() - t_now;
        if (frame_cost < FRAME_BUDGET_US) {
            timer.SleepMicroseconds(FRAME_BUDGET_US - frame_cost);
        }
    }

    GameShutdown(display);
    return 0;
}

#endif // PLATFORM_PC

// =============================================================================
// PLATFORM: ESP32
// =============================================================================
#ifdef PLATFORM_ESP32

static TimerESP32   timer;
static DisplayESP32 display;
static FrameBuffer  fb;

static uint64_t t_last     = 0;
static uint64_t accum      = 0;
static uint64_t tick_count = 0;

void setup()
{
    Serial.begin(115200);
    Serial.println(F("[Bixim] Booting..."));
    GameInit(display, fb);
    t_last = timer.GetMicroseconds();
    Serial.println(F("[Bixim] Boot complete."));
}

void loop()
{
    const uint64_t t_now  = timer.GetMicroseconds();
    uint64_t delta = t_now - t_last;
    t_last = t_now;

    const uint64_t MAX_DELTA = MAX_LOGIC_TICKS_PER_FRAME * LOGIC_TICK_US;
    if (delta > MAX_DELTA) delta = MAX_DELTA;

    accum += delta;

    uint32_t ticks_this_frame = 0;
    while (accum >= LOGIC_TICK_US && ticks_this_frame < MAX_LOGIC_TICKS_PER_FRAME) {
        UpdateLogic(tick_count, fb);
        accum -= LOGIC_TICK_US;
        ++tick_count;
        ++ticks_this_frame;
    }

    RenderGraphics(display, fb);

    const uint64_t frame_cost = timer.GetMicroseconds() - t_now;
    if (frame_cost < FRAME_BUDGET_US) {
        timer.SleepMicroseconds(FRAME_BUDGET_US - frame_cost);
    }
}

#endif // PLATFORM_ESP32
