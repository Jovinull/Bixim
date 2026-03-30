// =============================================================================
// Bixim - Main Entry Point
// =============================================================================
// This file serves as the unified entry point for both compilation targets.
// The preprocessor macro PLATFORM_PC is defined in platformio.ini [env:native].
// The preprocessor macro PLATFORM_ESP32 is defined in [env:esp32dev].
//
// Game logic shared between both platforms should live in separate modules
// under src/ and be included here without any platform guard.
// =============================================================================

// -----------------------------------------------------------------------------
// Platform-specific includes
// -----------------------------------------------------------------------------
#ifdef PLATFORM_PC
    #include <raylib.h>
    #include <cstdio>
#endif

#ifdef PLATFORM_ESP32
    #include <Arduino.h>
    #include <Wire.h>
    #include <Adafruit_GFX.h>
    #include <Adafruit_SSD1306.h>
#endif

// =============================================================================
// PLATFORM: PC (native)
// Entry point follows the standard C++ `int main()` signature.
// Raylib manages the window, input, and rendering loop.
// =============================================================================
#ifdef PLATFORM_PC

int main(void) {
    // Raylib window matching the SSD1306 resolution for 1:1 pixel mapping tests
    // Scale it up for readability during development
    const int SCALE  = 6;
    const int WIDTH  = 128 * SCALE;
    const int HEIGHT = 64  * SCALE;

    InitWindow(WIDTH, HEIGHT, "Bixim - Native Debug Build");
    SetTargetFPS(30);

    while (!WindowShouldClose()) {
        // --- Update phase (game logic will go here) ---

        // --- Draw phase ---
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("Bixim", WIDTH / 2 - MeasureText("Bixim", 30) / 2, 20, 30, WHITE);
        DrawText("Hello, World!", WIDTH / 2 - MeasureText("Hello, World!", 20) / 2, 70, 20, LIGHTGRAY);
        DrawText("[native build - PC]", 10, HEIGHT - 24, 12, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

#endif // PLATFORM_PC

// =============================================================================
// PLATFORM: ESP32 (arduino framework)
// Entry point follows the Arduino paradigm: setup() + loop().
// =============================================================================
#ifdef PLATFORM_ESP32

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1    // No dedicated reset pin; share MCU reset
#define OLED_ADDRESS  0x3C  // Most SSD1306 modules use 0x3C; try 0x3D if blank

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
    Serial.begin(115200);
    Serial.println(F("[Bixim] Booting..."));

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println(F("[Bixim] CRITICAL: SSD1306 init failed. Check wiring and I2C address."));
        // Halt execution - no point continuing without a display
        for (;;) {}
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("  Bixim"));
    display.println(F("  Hello, World!"));
    display.display();

    Serial.println(F("[Bixim] Display OK. Boot complete."));
}

void loop() {
    // Main game loop will be implemented here.
    // Avoid using delay() - prefer non-blocking timing with millis().
}

#endif // PLATFORM_ESP32
