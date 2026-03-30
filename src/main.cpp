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
// HAL modules in use:
//   ITimer   — platform clock (TimerPC / TimerESP32)
//   IDisplay — framebuffer flush (DisplayPC / DisplayESP32)
//   IInput   — button state (InputPC / InputESP32)
// =============================================================================

// -----------------------------------------------------------------------------
// Platform-specific includes
// -----------------------------------------------------------------------------
#ifdef PLATFORM_PC
    #include <cstdio>
    #include "hal/TimerPC.h"
    #include "hal/DisplayPC.h"
    #include "hal/InputPC.h"
#endif

#ifdef PLATFORM_ESP32
    #include <Arduino.h>
    #include <Wire.h>
    #include "hal/TimerESP32.h"
    #include "hal/DisplayESP32.h"
    #include "hal/InputESP32.h"
#endif

#include "hal/ITimer.h"
#include "hal/IDisplay.h"
#include "hal/IInput.h"
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
// Sprite A — simple face (default)
static const uint8_t SPRITE_FACE_A[16 * 2] = {
    0b00111100, 0b00000000,
    0b01000010, 0b00000000,
    0b10100101, 0b00000000,
    0b10000001, 0b00000000,
    0b10000001, 0b00000000,
    0b10100101, 0b00000000,
    0b10011001, 0b00000000,
    0b01000010, 0b00000000,
    0b00111100, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
};

// Sprite B — surprised face (shown when Button B is pressed)
static const uint8_t SPRITE_FACE_B[16 * 2] = {
    0b00111100, 0b00000000,
    0b01000010, 0b00000000,
    0b10110101, 0b00000000,
    0b10000001, 0b00000000,
    0b10000001, 0b00000000,
    0b10001001, 0b00000000,
    0b10111101, 0b00000000,
    0b01000010, 0b00000000,
    0b00111100, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
};

// Shared circular mask for both sprites
static const uint8_t MASK_FACE[16 * 2] = {
    0b00111100, 0b00000000,
    0b01111110, 0b00000000,
    0b11111111, 0b00000000,
    0b11111111, 0b00000000,
    0b11111111, 0b00000000,
    0b11111111, 0b00000000,
    0b11111111, 0b00000000,
    0b01111110, 0b00000000,
    0b00111100, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
};

// =============================================================================
// Game state (shared between platforms via static storage)
// =============================================================================
static int  g_sprite_x       = (128 - 16) / 2;   // horizontal position, starts centered
static int  g_sprite_y       = (64  - 16) / 2;   // vertical position, stays centered
static bool g_use_sprite_b   = false;              // false = face A, true = face B

// Movement step in pixels per logic tick (10 Hz → 10 px/s at step=1)
static constexpr int MOVE_STEP = 3;

// =============================================================================
// Forward declarations
// =============================================================================
void GameInit   (IDisplay& display, IInput& input, FrameBuffer& fb);
void UpdateLogic(IInput& input, FrameBuffer& fb, uint64_t tick_count);
void RenderGraphics(IDisplay& display, const FrameBuffer& fb);
void GameShutdown(IDisplay& display);

// =============================================================================
// Implementation
// =============================================================================

void GameInit(IDisplay& display, IInput& input, FrameBuffer& fb)
{
    if (!display.Init()) {
#ifdef PLATFORM_ESP32
        for (;;) {}
#endif
    }
    input.Init();
    fb.Clear();
}

void UpdateLogic(IInput& input, FrameBuffer& fb, uint64_t tick_count)
{
    // --- 1. Sample input (must be first in UpdateLogic) ---
    input.Update();

    // --- 2. Process button events ---

    // Button A — move sprite left
    if (input.IsHeld(Button::A)) {
        g_sprite_x -= MOVE_STEP;
        // Clamp to display bounds
        if (g_sprite_x < 0) g_sprite_x = 0;

#ifdef PLATFORM_PC
        printf("[Input] A held — sprite_x = %d\n", g_sprite_x);
#else
        Serial.print(F("[Input] A held — sprite_x = "));
        Serial.println(g_sprite_x);
#endif
    }

    // Button C — move sprite right
    if (input.IsHeld(Button::C)) {
        g_sprite_x += MOVE_STEP;
        // Clamp: sprite is 16px wide
        if (g_sprite_x > DISPLAY_WIDTH - 16) g_sprite_x = DISPLAY_WIDTH - 16;

#ifdef PLATFORM_PC
        printf("[Input] C held — sprite_x = %d\n", g_sprite_x);
#else
        Serial.print(F("[Input] C held — sprite_x = "));
        Serial.println(g_sprite_x);
#endif
    }

    // Button B — toggle between sprite A and B (fires once per press)
    if (input.IsPressed(Button::B)) {
        g_use_sprite_b = !g_use_sprite_b;

#ifdef PLATFORM_PC
        printf("[Input] B pressed — sprite = %s\n", g_use_sprite_b ? "B" : "A");
#else
        Serial.print(F("[Input] B pressed — sprite = "));
        Serial.println(g_use_sprite_b ? F("B") : F("A"));
#endif
    }

    // --- 3. Draw frame ---
    fb.Clear();

    // Border
    for (int x = 0; x < DISPLAY_WIDTH;  ++x) {
        drawPixel(fb, x, 0,                  true);
        drawPixel(fb, x, DISPLAY_HEIGHT - 1, true);
    }
    for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
        drawPixel(fb, 0,                  y, true);
        drawPixel(fb, DISPLAY_WIDTH - 1,  y, true);
    }

    // Sprite
    const uint8_t* active_sprite = g_use_sprite_b ? SPRITE_FACE_B : SPRITE_FACE_A;
    drawSprite(fb, g_sprite_x, g_sprite_y, active_sprite, MASK_FACE, 16, 16);

    // Suppress unused parameter warning on configurations where tick_count
    // is not yet used for animation.
    (void)tick_count;
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
    TimerPC     timer;
    DisplayPC   display;
    InputPC     input;
    FrameBuffer fb;

    GameInit(display, input, fb);

    uint64_t t_last     = timer.GetMicroseconds();
    uint64_t accum      = 0;
    uint64_t tick_count = 0;

    while (display.IsRunning()) {
        // --- 1. Delta time ---
        const uint64_t t_now  = timer.GetMicroseconds();
        uint64_t       delta  = t_now - t_last;
        t_last = t_now;

        const uint64_t MAX_DELTA = MAX_LOGIC_TICKS_PER_FRAME * LOGIC_TICK_US;
        if (delta > MAX_DELTA) delta = MAX_DELTA;

        // --- 2. Accumulate ---
        accum += delta;

        // --- 3. Fixed-rate logic ticks ---
        uint32_t ticks_this_frame = 0;
        while (accum >= LOGIC_TICK_US && ticks_this_frame < MAX_LOGIC_TICKS_PER_FRAME) {
            UpdateLogic(input, fb, tick_count);
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
static InputESP32   input;
static FrameBuffer  fb;

static uint64_t t_last     = 0;
static uint64_t accum      = 0;
static uint64_t tick_count = 0;

void setup()
{
    Serial.begin(115200);
    Serial.println(F("[Bixim] Booting..."));
    GameInit(display, input, fb);
    t_last = timer.GetMicroseconds();
    Serial.println(F("[Bixim] Boot complete. Buttons: A=GPIO25 B=GPIO26 C=GPIO27"));
}

void loop()
{
    const uint64_t t_now  = timer.GetMicroseconds();
    uint64_t       delta  = t_now - t_last;
    t_last = t_now;

    const uint64_t MAX_DELTA = MAX_LOGIC_TICKS_PER_FRAME * LOGIC_TICK_US;
    if (delta > MAX_DELTA) delta = MAX_DELTA;

    accum += delta;

    uint32_t ticks_this_frame = 0;
    while (accum >= LOGIC_TICK_US && ticks_this_frame < MAX_LOGIC_TICKS_PER_FRAME) {
        UpdateLogic(input, fb, tick_count);
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
