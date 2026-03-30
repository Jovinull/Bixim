// =============================================================================
// Bixim — PC Display Driver (Definition)
// File   : src/hal/DisplayPC.cpp
// Platform: PLATFORM_PC
// =============================================================================
#ifdef PLATFORM_PC

#include "hal/DisplayPC.h"
#include <raylib.h>

static constexpr int WIN_WIDTH  = DISPLAY_WIDTH  * DisplayPC::PIXEL_SCALE;
static constexpr int WIN_HEIGHT = DISPLAY_HEIGHT * DisplayPC::PIXEL_SCALE;

static const Color PIXEL_ON  = WHITE;
static const Color PIXEL_OFF = BLACK;

bool DisplayPC::Init()
{
    InitWindow(WIN_WIDTH, WIN_HEIGHT, "Bixim - Native Debug Build");
    SetTargetFPS(0); // Game loop manages timing manually via ITimer
    return !WindowShouldClose();
}

void DisplayPC::Flush(const FrameBuffer& fb)
{
    BeginDrawing();
    ClearBackground(PIXEL_OFF);

    // Iterate over all 1024 bytes in the framebuffer.
    // For each byte, unpack the 8 vertical pixels it represents.
    for (int page = 0; page < DISPLAY_PAGES; ++page) {
        for (int col = 0; col < DISPLAY_WIDTH; ++col) {

            // byte_index = page * 128 + col
            // This byte holds pixels for column=col, rows Y=(page*8)..(page*8+7)
            const uint8_t byte_val = fb.data[page * DISPLAY_WIDTH + col];

            for (int bit = 0; bit < 8; ++bit) {
                // Extract the pixel value for row (page*8 + bit).
                //   (byte_val >> bit) & 1
                //   Shifts the target bit down to position 0 and masks it.
                //   bit=0 → topmost row of the page, bit=7 → bottommost.
                const bool pixel_on = ((byte_val >> bit) & 0x1u) != 0;

                if (pixel_on) {
                    const int screen_x = col * PIXEL_SCALE;
                    const int screen_y = (page * 8 + bit) * PIXEL_SCALE;
                    DrawRectangle(screen_x, screen_y, PIXEL_SCALE, PIXEL_SCALE, PIXEL_ON);
                }
                // Off pixels are already covered by ClearBackground(BLACK).
            }
        }
    }

    EndDrawing();
}

bool DisplayPC::IsRunning() const
{
    return !WindowShouldClose();
}

void DisplayPC::Shutdown()
{
    CloseWindow();
}

#endif // PLATFORM_PC
