// =============================================================================
// Bixim — Main Entry Point & Game Loop
// File   : src/main.cpp
// =============================================================================
// Game loop architecture: Fixed Timestep Accumulator
//
// Two independent tick domains:
//
//   LOGIC_HZ     = 10 Hz  → LOGIC_TICK_US = 100,000 µs per logic step
//   TARGET_FPS   = 30     → FRAME_BUDGET_US = 33,333 µs per render frame
//
// Core loop equations (executed every frame):
//
//   delta    = t_now - t_last_frame                  [µs elapsed this frame]
//   accum   += delta                                  [unprocessed time bank]
//
//   n_ticks  = min(accum / LOGIC_TICK_US, MAX_LOGIC_TICKS_PER_FRAME)
//   for i in [0, n_ticks):
//       UpdateLogic()                                 [advance FSM by one fixed step]
//   accum   -= n_ticks * LOGIC_TICK_US                [consume processed time]
//
//   RenderGraphics()                                  [draw current state]
//
//   frame_cost = t_after_render - t_now
//   if frame_cost < FRAME_BUDGET_US:
//       SleepMicroseconds(FRAME_BUDGET_US - frame_cost)   [cap to TARGET_FPS]
//
// Why MAX_LOGIC_TICKS_PER_FRAME?
//   If the host machine lags for N seconds (e.g., OS preemption, debugger
//   breakpoint), accum would accumulate N seconds of debt. Without the cap,
//   the loop would spend the next several frames doing nothing but logic with
//   no renders — the "spiral of death". Capping at 5 ticks means we accept
//   up to 500ms of lag catchup per frame and discard the rest, keeping the
//   game responsive at the cost of slight time compression after hard stalls.
// =============================================================================

// -----------------------------------------------------------------------------
// Platform-specific includes
// -----------------------------------------------------------------------------
#ifdef PLATFORM_PC
    #include <raylib.h>
    #include <cstdio>
    #include "hal/TimerPC.h"
#endif

#ifdef PLATFORM_ESP32
    #include <Arduino.h>
    #include <Wire.h>
    #include <Adafruit_GFX.h>
    #include <Adafruit_SSD1306.h>
    #include "hal/TimerESP32.h"
#endif

#include "hal/ITimer.h"

// =============================================================================
// Game Loop Constants
// =============================================================================

// Logic update rate. The FSM advances at exactly this rate, regardless of
// how fast or slow the host renders.
static constexpr uint32_t LOGIC_HZ               = 10;
static constexpr uint64_t LOGIC_TICK_US           = 1'000'000ULL / LOGIC_HZ; // 100,000 µs

// Render rate cap. On ESP32, the SSD1306 I2C bus will be the real bottleneck
// (typically ~20 FPS max at 400kHz I2C). On PC, Raylib enforces this via
// SetTargetFPS() and our manual sleep provides a secondary cap.
static constexpr uint32_t TARGET_FPS              = 30;
static constexpr uint64_t FRAME_BUDGET_US         = 1'000'000ULL / TARGET_FPS; // 33,333 µs

// Spiral-of-death protection: maximum logic ticks consumed in a single frame.
// At LOGIC_HZ=10, this caps lag catchup to 500ms per frame.
static constexpr uint32_t MAX_LOGIC_TICKS_PER_FRAME = 5;

// =============================================================================
// Forward declarations — game modules (to be implemented in future modules)
// =============================================================================

// Called once at startup to initialize all game subsystems.
void GameInit();

// Called at a fixed rate (LOGIC_HZ). Advances the FSM, reads input,
// decrements stats. Must NOT perform any rendering.
// Parameter: tick_count — total logic ticks elapsed since GameInit().
//            Use this to derive long-period events (e.g., hunger decay).
//            Example: if (tick_count % (LOGIC_HZ * 60) == 0) → 1 minute elapsed.
void UpdateLogic(uint64_t tick_count);

// Called once per render frame. Draws the current game state to the active
// display (Raylib window on PC, SSD1306 on ESP32).
// Must NOT modify any game state — read-only access to state is allowed.
void RenderGraphics();

// Called once at shutdown (PC only).
void GameShutdown();

// =============================================================================
// Placeholder implementations — replace with real game logic in future modules
// =============================================================================

void GameInit()
{
#ifdef PLATFORM_PC
    printf("[Bixim] GameInit()\n");
#else
    Serial.println(F("[Bixim] GameInit()"));
#endif
}

void UpdateLogic(uint64_t tick_count)
{
    // Suppress unused parameter warning until real logic is implemented.
    (void)tick_count;
}

void RenderGraphics()
{
    // Rendering stubs are in the platform-specific main() / loop() below.
}

void GameShutdown()
{
#ifdef PLATFORM_PC
    printf("[Bixim] GameShutdown()\n");
#endif
}

// =============================================================================
// PLATFORM: PC
// =============================================================================
#ifdef PLATFORM_PC

int main(void)
{
    // --- Window setup ---
    const int SCALE  = 6;
    const int WIDTH  = 128 * SCALE;
    const int HEIGHT = 64  * SCALE;

    InitWindow(WIDTH, HEIGHT, "Bixim - Native Debug Build");
    // SetTargetFPS is set to 0 (uncapped) because our loop manages timing
    // manually. Raylib's internal cap would interfere with our accumulator.
    SetTargetFPS(0);

    // --- Timer & loop state ---
    TimerPC timer;
    uint64_t t_last  = timer.GetMicroseconds();
    uint64_t accum   = 0;
    uint64_t tick_count = 0;

    GameInit();

    // =========================================================================
    // Main Game Loop — Fixed Timestep Accumulator
    // =========================================================================
    while (!WindowShouldClose()) {

        // --- 1. Measure delta time ---
        const uint64_t t_now  = timer.GetMicroseconds();
        uint64_t       delta  = t_now - t_last;
        t_last = t_now;

        // Guard: clamp delta to avoid enormous spikes from OS preemption,
        // debugger pauses or initial frame (where delta can be very large).
        // Maximum accepted delta = MAX_LOGIC_TICKS_PER_FRAME * LOGIC_TICK_US.
        const uint64_t MAX_DELTA = MAX_LOGIC_TICKS_PER_FRAME * LOGIC_TICK_US;
        if (delta > MAX_DELTA) {
            delta = MAX_DELTA;
        }

        // --- 2. Accumulate unprocessed time ---
        accum += delta;

        // --- 3. Logic update loop (fixed timestep) ---
        // Equation: consume LOGIC_TICK_US from accum for each UpdateLogic() call.
        // The loop runs 0 or more times per frame depending on elapsed time.
        uint32_t ticks_this_frame = 0;
        while (accum >= LOGIC_TICK_US && ticks_this_frame < MAX_LOGIC_TICKS_PER_FRAME) {
            UpdateLogic(tick_count);
            accum -= LOGIC_TICK_US;
            ++tick_count;
            ++ticks_this_frame;
        }

        // --- 4. Render (current state, not interpolated — future improvement) ---
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("Bixim", WIDTH / 2 - MeasureText("Bixim", 40) / 2, 16, 40, WHITE);

        // Debug overlay — remove in release builds
        char dbg[64];
        snprintf(dbg, sizeof(dbg), "tick: %llu  accum: %llums",
                 (unsigned long long)tick_count,
                 (unsigned long long)(accum / 1000));
        DrawText(dbg, 8, HEIGHT - 20, 10, DARKGRAY);

        EndDrawing();

        // --- 5. FPS cap: sleep for remaining frame budget ---
        // Equation: sleep_time = FRAME_BUDGET_US - frame_cost
        // where frame_cost = t_after_render - t_now (measured after EndDrawing)
        const uint64_t frame_cost = timer.GetMicroseconds() - t_now;
        if (frame_cost < FRAME_BUDGET_US) {
            timer.SleepMicroseconds(FRAME_BUDGET_US - frame_cost);
        }
    }

    GameShutdown();
    CloseWindow();
    return 0;
}

#endif // PLATFORM_PC

// =============================================================================
// PLATFORM: ESP32
// =============================================================================
#ifdef PLATFORM_ESP32

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Loop state — declared static so they persist across loop() calls.
static TimerESP32 timer;
static uint64_t   t_last    = 0;
static uint64_t   accum     = 0;
static uint64_t   tick_count = 0;

void setup()
{
    Serial.begin(115200);
    Serial.println(F("[Bixim] Booting..."));

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println(F("[Bixim] CRITICAL: SSD1306 init failed. Check wiring and I2C address."));
        for (;;) {}
    }

    display.clearDisplay();
    display.display();

    GameInit();

    t_last = timer.GetMicroseconds();
    Serial.println(F("[Bixim] Boot complete. Entering game loop."));
}

void loop()
{
    // =========================================================================
    // Main Game Loop — Fixed Timestep Accumulator (ESP32)
    // Same equations as the PC build. loop() is called by the Arduino runtime
    // in an infinite while(1), so no explicit loop construct is needed here.
    // =========================================================================

    // --- 1. Measure delta time ---
    const uint64_t t_now = timer.GetMicroseconds();
    uint64_t       delta = t_now - t_last;
    t_last = t_now;

    const uint64_t MAX_DELTA = MAX_LOGIC_TICKS_PER_FRAME * LOGIC_TICK_US;
    if (delta > MAX_DELTA) {
        delta = MAX_DELTA;
    }

    // --- 2. Accumulate ---
    accum += delta;

    // --- 3. Logic update loop ---
    uint32_t ticks_this_frame = 0;
    while (accum >= LOGIC_TICK_US && ticks_this_frame < MAX_LOGIC_TICKS_PER_FRAME) {
        UpdateLogic(tick_count);
        accum -= LOGIC_TICK_US;
        ++tick_count;
        ++ticks_this_frame;
    }

    // --- 4. Render ---
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(32, 28);
    display.println(F("Bixim"));
    display.display();

    // --- 5. FPS cap (soft — SSD1306 I2C transfer is the real bottleneck) ---
    const uint64_t frame_cost = timer.GetMicroseconds() - t_now;
    if (frame_cost < FRAME_BUDGET_US) {
        timer.SleepMicroseconds(FRAME_BUDGET_US - frame_cost);
    }
}

#endif // PLATFORM_ESP32
