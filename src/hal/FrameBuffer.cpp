// =============================================================================
// Bixim — FrameBuffer: Rendering Primitives Implementation
// File   : src/hal/FrameBuffer.cpp
// =============================================================================
#include "hal/FrameBuffer.h"
#include "hal/Font5x7.h"

// =============================================================================
// drawPixel
// =============================================================================
void drawPixel(FrameBuffer& fb, int x, int y, bool color)
{
    // Bounds check — silently discard out-of-range draws.
    // On the ESP32, an out-of-bounds write would corrupt adjacent memory.
    // On PC, it would write past the array. Either way, it must be rejected.
    if (x < 0 || x >= DISPLAY_WIDTH || y < 0 || y >= DISPLAY_HEIGHT) {
        return;
    }

    // --- Index calculation ---
    //
    // page         = y / 8
    //   Integer division. Maps the Y coordinate to one of the 8 horizontal
    //   pages. Page 0 covers Y=0..7, page 1 covers Y=8..15, etc.
    //
    // byte_index   = page * DISPLAY_WIDTH + x
    //              = (y / 8) * 128 + x
    //   Selects the byte in the flat array that owns this pixel.
    //
    // bit_position = y % 8
    //   The remainder selects which of the 8 bits within that byte represents
    //   this pixel. Bit 0 is the topmost row of the page; bit 7 is the bottom.
    //
    const uint16_t byte_index   = static_cast<uint16_t>((y / 8) * DISPLAY_WIDTH + x);
    const uint8_t  bit_position = static_cast<uint8_t>(y % 8);

    if (color) {
        // Set the bit: OR with a mask that has only the target bit as 1.
        //   Before: xxxxxxxx
        //   Mask  : 00000100  (bit_position = 2)
        //   After : xxxx1xxx  (target bit forced to 1, all others unchanged)
        fb.data[byte_index] |= static_cast<uint8_t>(1u << bit_position);
    } else {
        // Clear the bit: AND with the bitwise complement of the mask.
        //   Before: xxxxxxxx
        //   Mask  : 00000100  (bit_position = 2)
        //   ~Mask : 11111011
        //   After : xxxx0xxx  (target bit forced to 0, all others unchanged)
        fb.data[byte_index] &= static_cast<uint8_t>(~(1u << bit_position));
    }
}

// =============================================================================
// drawSprite
// =============================================================================
void drawSprite(FrameBuffer& fb,
                int x, int y,
                const uint8_t* sprite,
                const uint8_t* mask,
                int width, int height)
{
    // Number of bytes per sprite row. A 16-pixel-wide sprite needs 2 bytes.
    // A 9-pixel-wide sprite still needs 2 bytes (the last 7 bits are padding).
    //
    //   bytes_per_row = ceil(width / 8) = (width + 7) / 8
    //
    const int bytes_per_row = (width + 7) / 8;

    for (int sy = 0; sy < height; ++sy) {
        for (int sx = 0; sx < width; ++sx) {

            // --- Extract one bit from the sprite and mask arrays ---
            //
            // Sprites are stored horizontal, MSB-left:
            //   byte_in_row = sx / 8
            //     Which byte of this row owns this column.
            //
            //   bit_in_byte = 7 - (sx % 8)
            //     Within that byte, bit 7 (MSB) is the leftmost pixel.
            //     We subtract from 7 to convert "left-to-right column index"
            //     into "bit index from MSB".
            //     sx%8=0 → bit 7 (leftmost), sx%8=7 → bit 0 (rightmost).
            //
            const int sprite_byte = sy * bytes_per_row + (sx / 8);
            const int sprite_bit  = 7 - (sx % 8);

            // Extract the single pixel bit from sprite and mask.
            // >> shifts the target bit down to position 0, & 0x1 isolates it.
            const uint8_t sprite_pixel = (sprite[sprite_byte] >> sprite_bit) & 0x1u;
            const uint8_t mask_pixel   = (mask[sprite_byte]   >> sprite_bit) & 0x1u;

            // --- Compute destination coordinates in the framebuffer ---
            const int dest_x = x + sx;
            const int dest_y = y + sy;

            // Clip: skip pixels that fall outside the display boundaries.
            if (dest_x < 0 || dest_x >= DISPLAY_WIDTH  ||
                dest_y < 0 || dest_y >= DISPLAY_HEIGHT) {
                continue;
            }

            // --- Map destination (dest_x, dest_y) to buffer position ---
            const uint16_t byte_index   = static_cast<uint16_t>((dest_y / 8) * DISPLAY_WIDTH + dest_x);
            const uint8_t  bit_position = static_cast<uint8_t>(dest_y % 8);

            // --- Compositing operation (Porter-Duff Over, 1-bit alpha) ---
            //
            // fb[i] = (fb[i] & ~(mask_pixel << bit)) | ((sprite_pixel & mask_pixel) << bit)
            //
            // Step 1 — Punch a hole in the background where the sprite is opaque:
            //   fb[i] &= ~(mask_pixel << bit_position)
            //
            //   When mask_pixel = 0 (transparent):
            //     ~(0 << bit) = ~0x00 = 0xFF
            //     fb[i] &= 0xFF  →  no change. Background pixel is preserved.
            //
            //   When mask_pixel = 1 (opaque):
            //     ~(1 << bit)  →  a byte with all bits set except bit_position
            //     fb[i] &= ...  →  only the target bit is cleared. Hole punched.
            //
            fb.data[byte_index] &= static_cast<uint8_t>(~(mask_pixel << bit_position));

            // Step 2 — Paint the sprite pixel into the cleared position:
            //   fb[i] |= (sprite_pixel & mask_pixel) << bit_position
            //
            //   When mask_pixel = 0 (transparent):
            //     sprite_pixel & 0 = 0
            //     fb[i] |= 0  →  no change. Background pixel survives intact.
            //
            //   When mask_pixel = 1 (opaque):
            //     sprite_pixel & 1 = sprite_pixel
            //     The sprite's actual pixel value (0 or 1) is OR'd into the
            //     bit position that was cleared in Step 1.
            //     Result: background was cleared, sprite is painted.
            //
            fb.data[byte_index] |= static_cast<uint8_t>((sprite_pixel & mask_pixel) << bit_position);
        }
    }
}

// =============================================================================
// drawChar
// =============================================================================
void drawChar(FrameBuffer& fb, int x, int y, char c)
{
    // Bounds check: if the character origin is entirely off-screen, skip it.
    if (x + FONT_CHAR_W < 0 || x >= DISPLAY_WIDTH  ||
        y + FONT_CHAR_H < 0 || y >= DISPLAY_HEIGHT) {
        return;
    }

    // Map ASCII to font table index. Characters outside the range render as space.
    const uint8_t ascii = static_cast<uint8_t>(c);
    const int idx = (ascii >= FONT_FIRST_CHAR && ascii <= FONT_LAST_CHAR)
                    ? (ascii - FONT_FIRST_CHAR)
                    : 0; // 0 = space

    // Iterate over the 7 rows of the glyph.
    for (int row = 0; row < FONT_CHAR_H; ++row) {
        const uint8_t row_data = FONT_5X7[idx][row];

        // Each row byte encodes 5 pixels (bits 4..0, MSB = leftmost).
        // Iterate columns right-to-left in bit significance.
        for (int col = 0; col < FONT_CHAR_W; ++col) {
            // bit4 = col0, bit3 = col1, ..., bit0 = col4
            // Shift: (4 - col) puts the target bit into position 0.
            const bool pixel_on = ((row_data >> (FONT_CHAR_W - 1 - col)) & 0x1u) != 0;
            drawPixel(fb, x + col, y + row, pixel_on);
        }
    }
}

// =============================================================================
// drawText
// =============================================================================
void drawText(FrameBuffer& fb, int x, int y, const char* text)
{
    if (!text) return;
    int cursor_x = x;
    while (*text != '\0') {
        drawChar(fb, cursor_x, y, *text);
        cursor_x += FONT_ADVANCE; // advance by glyph width + 1px gap
        ++text;
    }
}

// =============================================================================
// drawRect
// =============================================================================
void drawRect(FrameBuffer& fb, int x, int y, int w, int h, bool color)
{
    for (int px = x; px < x + w; ++px) {
        drawPixel(fb, px, y,           color); // top edge
        drawPixel(fb, px, y + h - 1,   color); // bottom edge
    }
    for (int py = y; py < y + h; ++py) {
        drawPixel(fb, x,           py, color); // left edge
        drawPixel(fb, x + w - 1,   py, color); // right edge
    }
}

// =============================================================================
// drawBar
// =============================================================================
void drawBar(FrameBuffer& fb, int x, int y, int w, int h,
             uint8_t value, uint8_t max_value)
{
    // Draw outer border.
    drawRect(fb, x, y, w, h, true);

    if (max_value == 0) return; // guard against division by zero

    // Interior dimensions (border is 1px on each side).
    const int inner_w = w - 2;
    const int inner_h = h - 2;

    // Equation:
    //   fill_px = (value * inner_w) / max_value
    //   Integer division truncates — acceptable for visual display.
    //   At inner_w=80 and max=100, each pixel represents 1.25% of the stat.
    const int fill_px = (static_cast<int>(value) * inner_w) / static_cast<int>(max_value);

    // Fill the interior left-to-right up to fill_px.
    for (int py = y + 1; py < y + 1 + inner_h; ++py) {
        for (int px = x + 1; px < x + 1 + fill_px; ++px) {
            drawPixel(fb, px, py, true);
        }
    }
}
