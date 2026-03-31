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
//
// Logic modules:
//   Pet      — FSM, stat decay, actions
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
#include "logic/Pet.h"

// =============================================================================
// Game Loop Constants
// =============================================================================
static constexpr uint32_t LOGIC_HZ                  = 10;
static constexpr uint64_t LOGIC_TICK_US              = 1'000'000ULL / LOGIC_HZ;
static constexpr uint32_t TARGET_FPS                 = 30;
static constexpr uint64_t FRAME_BUDGET_US            = 1'000'000ULL / TARGET_FPS;
static constexpr uint32_t MAX_LOGIC_TICKS_PER_FRAME  = 5;

// =============================================================================
// Sprite data — 16x16 pixels, horizontal format (MSB = leftmost pixel)
// =============================================================================
// Normal face
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

// Surprised / eating face
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

// Sick face — X eyes, wavy mouth
static const uint8_t SPRITE_FACE_SICK[16 * 2] = {
    0b00111100, 0b00000000,
    0b01000010, 0b00000000,
    0b10010101, 0b00000000, // X X eyes
    0b10101001, 0b00000000,
    0b10010001, 0b00000000,
    0b10000001, 0b00000000,
    0b10010101, 0b00000000, // wavy mouth
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

// Dead face — X eyes, flat mouth
static const uint8_t SPRITE_FACE_DEAD[16 * 2] = {
    0b00111100, 0b00000000,
    0b01000010, 0b00000000,
    0b10010101, 0b00000000, // X X eyes
    0b10101001, 0b00000000,
    0b10010001, 0b00000000,
    0b10000001, 0b00000000,
    0b10011001, 0b00000000, // flat mouth
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

// Sleeping face — closed eyes
static const uint8_t SPRITE_FACE_SLEEP[16 * 2] = {
    0b00111100, 0b00000000,
    0b01000010, 0b00000000,
    0b10000001, 0b00000000,
    0b10100101, 0b00000000, // closed eyes (lines)
    0b10000001, 0b00000000,
    0b10000001, 0b00000000,
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

// Egg / hatching sprite
static const uint8_t SPRITE_EGG[16 * 2] = {
    0b00011000, 0b00000000,
    0b00111100, 0b00000000,
    0b01111110, 0b00000000,
    0b01111110, 0b00000000,
    0b01111110, 0b00000000,
    0b01111110, 0b00000000,
    0b00111100, 0b00000000,
    0b00011000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
};

// Circular mask shared by all face sprites
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

// Mask for egg sprite
static const uint8_t MASK_EGG[16 * 2] = {
    0b00011000, 0b00000000,
    0b00111100, 0b00000000,
    0b01111110, 0b00000000,
    0b01111110, 0b00000000,
    0b01111110, 0b00000000,
    0b01111110, 0b00000000,
    0b00111100, 0b00000000,
    0b00011000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
};

// =============================================================================
// HUD Layout Constants
// =============================================================================
// Sprite position: right side of screen, vertically centered
static constexpr int SPRITE_X = 96;
static constexpr int SPRITE_Y = (64 - 16) / 2; // 24

// HUD bars: left column
// Each bar: 2px label row + 3px gap + 5px bar height
//   Bar area: x=0, w=88, h=5 (inner 3px tall)
//   Labels are drawn as single-letter abbreviations
static constexpr int BAR_X     = 8;   // left margin
static constexpr int BAR_W     = 76;  // bar width including 1px border each side
static constexpr int BAR_H     = 5;   // bar height including 1px border each side
static constexpr int BAR_GAP   = 9;   // vertical spacing between bars (label + bar + gap)

// Y positions for each stat row (label at Y, bar at Y+7)
static constexpr int ROW_HUNGER    = 1;
static constexpr int ROW_HAPPINESS = 1 + BAR_GAP + BAR_H;     // 15
static constexpr int ROW_ENERGY    = 1 + 2 * (BAR_GAP + BAR_H); // 29
static constexpr int ROW_HYGIENE   = 1 + 3 * (BAR_GAP + BAR_H); // 43

// =============================================================================
// Game State
// =============================================================================
static Pet g_pet;

// =============================================================================
// Forward declarations
// =============================================================================
void GameInit      (IDisplay& display, IInput& input, FrameBuffer& fb);
void UpdateLogic   (IInput& input, FrameBuffer& fb, uint64_t tick_count);
void RenderGraphics(IDisplay& display, const FrameBuffer& fb);
void GameShutdown  (IDisplay& display);

// =============================================================================
// Helpers
// =============================================================================

// Returns the sprite + mask pair appropriate for the current pet state.
static void selectSprite(const Pet& pet,
                          const uint8_t*& out_sprite,
                          const uint8_t*& out_mask)
{
    switch (pet.getState()) {
    case PetState::HATCHING:
        out_sprite = SPRITE_EGG;
        out_mask   = MASK_EGG;
        break;
    case PetState::EATING:
        out_sprite = SPRITE_FACE_B;
        out_mask   = MASK_FACE;
        break;
    case PetState::SLEEPING:
        out_sprite = SPRITE_FACE_SLEEP;
        out_mask   = MASK_FACE;
        break;
    case PetState::SICK:
        out_sprite = SPRITE_FACE_SICK;
        out_mask   = MASK_FACE;
        break;
    case PetState::DEAD:
        out_sprite = SPRITE_FACE_DEAD;
        out_mask   = MASK_FACE;
        break;
    default: // IDLE
        out_sprite = SPRITE_FACE_A;
        out_mask   = MASK_FACE;
        break;
    }
}

// Returns the null-terminated state label (max 8 chars, uppercase).
static const char* stateLabel(PetState s)
{
    switch (s) {
    case PetState::HATCHING: return "HATCH";
    case PetState::IDLE:     return "IDLE";
    case PetState::EATING:   return "EAT";
    case PetState::SLEEPING: return "SLEEP";
    case PetState::SICK:     return "SICK";
    case PetState::DEAD:     return "DEAD";
    default:                 return "?";
    }
}

// Draws the 4-bar HUD on the left side of the framebuffer.
// Layout (x=BAR_X, column of 4 stat rows):
//   "H" label + bar (hunger)
//   "P" label + bar (happiness)
//   "E" label + bar (energy)
//   "C" label + bar (hygiene / cleanliness)
static void drawHUD(FrameBuffer& fb, const Pet& pet)
{
    const PetStats& s = pet.getStats();

    // --- Hunger ---
    drawChar(fb, 0, ROW_HUNGER, 'H');
    drawBar(fb,
            BAR_X, ROW_HUNGER,
            BAR_W, BAR_H,
            s.hunger, 100);

    // --- Happiness ---
    drawChar(fb, 0, ROW_HAPPINESS, 'P');
    drawBar(fb,
            BAR_X, ROW_HAPPINESS,
            BAR_W, BAR_H,
            s.happiness, 100);

    // --- Energy ---
    drawChar(fb, 0, ROW_ENERGY, 'E');
    drawBar(fb,
            BAR_X, ROW_ENERGY,
            BAR_W, BAR_H,
            s.energy, 100);

    // --- Hygiene ---
    drawChar(fb, 0, ROW_HYGIENE, 'C');
    drawBar(fb,
            BAR_X, ROW_HYGIENE,
            BAR_W, BAR_H,
            s.hygiene, 100);

    // --- State label (bottom row) ---
    drawText(fb, 0, 57, stateLabel(pet.getState()));
}

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
    // --- 1. Sample input ---
    input.Update();

    // --- 2. Process button events ---

    // Button A — feed the pet
    if (input.IsPressed(Button::A)) {
        const bool accepted = g_pet.feed();
#ifdef PLATFORM_PC
        printf("[Input] A pressed — feed %s\n", accepted ? "accepted" : "ignored");
#else
        Serial.print(F("[Input] A — feed "));
        Serial.println(accepted ? F("ok") : F("ignored"));
#endif
    }

    // Button B — play with the pet
    if (input.IsPressed(Button::B)) {
        const bool accepted = g_pet.play();
#ifdef PLATFORM_PC
        printf("[Input] B pressed — play %s\n", accepted ? "accepted" : "ignored");
#else
        Serial.print(F("[Input] B — play "));
        Serial.println(accepted ? F("ok") : F("ignored"));
#endif
    }

    // Button C — toggle sleep / wake
    if (input.IsPressed(Button::C)) {
        bool accepted = g_pet.sleep();
        if (!accepted) accepted = g_pet.wake();
#ifdef PLATFORM_PC
        printf("[Input] C pressed — sleep/wake %s\n", accepted ? "accepted" : "ignored");
#else
        Serial.print(F("[Input] C — sleep/wake "));
        Serial.println(accepted ? F("ok") : F("ignored"));
#endif
    }

    // --- 3. Advance pet logic ---
    g_pet.tick();

    // --- 4. Draw frame ---
    fb.Clear();

    // Sprite
    const uint8_t* active_sprite = nullptr;
    const uint8_t* active_mask   = nullptr;
    selectSprite(g_pet, active_sprite, active_mask);
    drawSprite(fb, SPRITE_X, SPRITE_Y, active_sprite, active_mask, 16, 16);

    // HUD
    drawHUD(fb, g_pet);

    // Divider between HUD and sprite area
    for (int py = 0; py < DISPLAY_HEIGHT; ++py) {
        drawPixel(fb, 88, py, true);
    }

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
    Serial.println(F("[Bixim] Boot complete. A=feed B=play C=sleep/wake"));
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
