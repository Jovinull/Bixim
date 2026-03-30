// =============================================================================
// Bixim — FrameBuffer: Data Structure and Rendering Primitives
// File   : include/hal/FrameBuffer.h
// =============================================================================
// The FrameBuffer is the central data structure of the rendering pipeline.
// It is a software representation of the SSD1306 VRAM, kept in the MCU's RAM.
// All drawing operations are performed on this buffer. When a frame is
// complete, IDisplay::Flush() transfers it to the physical hardware.
//
// Memory Layout — SSD1306 Page Format (Page-Major, Column-Minor):
//
//   The display is divided into 8 horizontal pages of 8 rows each.
//   Each byte in the buffer represents 8 vertically-stacked pixels in one
//   column, with bit 0 at the top of the page and bit 7 at the bottom.
//
//   Page 0 → rows Y = 0..7
//   Page 1 → rows Y = 8..15
//   ...
//   Page 7 → rows Y = 56..63
//
//   Buffer layout:
//     fb.data[0]           = col 0, page 0 (Y=0..7)
//     fb.data[1]           = col 1, page 0 (Y=0..7)
//     ...
//     fb.data[127]         = col 127, page 0 (Y=0..7)
//     fb.data[128]         = col 0,   page 1 (Y=8..15)
//     ...
//     fb.data[1023]        = col 127, page 7 (Y=56..63)
//
//   Index formula for pixel (x, y):
//     byte_index   = (y / 8) * DISPLAY_WIDTH + x
//     bit_position = y % 8
//
// Sprite Format (horizontal, MSB-left):
//   Sprites are stored row-major. Each row occupies ceil(width/8) bytes.
//   Within each byte, bit 7 (MSB) is the leftmost pixel of that chunk.
//
//   Example — 16x16 sprite (2 bytes per row, 32 bytes total):
//     sprite[0] = row 0, cols 0..7   (bit7=col0, bit0=col7)
//     sprite[1] = row 0, cols 8..15  (bit7=col8, bit0=col15)
//     sprite[2] = row 1, cols 0..7
//     ...
//
//   This format matches most sprite editor tools and is converted
//   on-the-fly to the page-major buffer format inside drawSprite().
// =============================================================================
#pragma once

#include <cstdint>
#include <cstring>

// =============================================================================
// Display Constants
// =============================================================================
static constexpr int DISPLAY_WIDTH    = 128;
static constexpr int DISPLAY_HEIGHT   = 64;
static constexpr int DISPLAY_PAGES    = DISPLAY_HEIGHT / 8;          // 8
static constexpr int FRAMEBUFFER_SIZE = DISPLAY_WIDTH * DISPLAY_PAGES; // 1024 bytes

// =============================================================================
// FrameBuffer Data Structure
// =============================================================================
struct FrameBuffer {
    // 1024 bytes — 1 bit per pixel, page-major layout as described above.
    // Initialized to all zeros (all pixels off / black) by the constructor.
    uint8_t data[FRAMEBUFFER_SIZE];

    FrameBuffer() { Clear(); }

    // Sets all pixels to off (black).
    void Clear() {
        memset(data, 0x00, FRAMEBUFFER_SIZE);
    }

    // Sets all pixels to on (white). Useful for testing display connectivity.
    void Fill() {
        memset(data, 0xFF, FRAMEBUFFER_SIZE);
    }
};

// =============================================================================
// Rendering Primitives
// =============================================================================

// -----------------------------------------------------------------------------
// drawPixel — Set or clear a single pixel in the framebuffer.
//
// Parameters:
//   fb    : reference to the target framebuffer
//   x     : horizontal coordinate [0, DISPLAY_WIDTH-1]
//   y     : vertical coordinate   [0, DISPLAY_HEIGHT-1]  (0 = top)
//   color : true = white (pixel on), false = black (pixel off)
//
// Bitwise operations:
//   byte_index   = (y / 8) * 128 + x
//                  └─ integer division selects the page (0..7)
//                                   └─ column offset within the page
//
//   bit_position = y % 8
//                  └─ which bit within the byte (0=top, 7=bottom of page)
//
//   To set   (color=1): fb.data[byte_index] |=  (1 << bit_position)
//     OR with a mask that has only the target bit set — all others unchanged.
//
//   To clear (color=0): fb.data[byte_index] &= ~(1 << bit_position)
//     AND with the bitwise complement — only the target bit is forced to 0.
// -----------------------------------------------------------------------------
void drawPixel(FrameBuffer& fb, int x, int y, bool color);

// -----------------------------------------------------------------------------
// drawSprite — Composite a sprite over the framebuffer with 1-bit transparency.
//
// Parameters:
//   fb     : reference to the target framebuffer (background + output)
//   x, y   : top-left corner of the sprite in display coordinates
//   sprite : pixel data, horizontal format, MSB=leftmost (see layout above)
//   mask   : transparency data, same format as sprite
//            1 = opaque pixel (drawn), 0 = transparent (background shows through)
//   width  : sprite width  in pixels (not necessarily a multiple of 8)
//   height : sprite height in pixels
//
// Compositing formula (applied per pixel):
//
//   For each pixel (sx, sy) in the sprite:
//
//     sprite_bit = extract bit from sprite[] at (sx, sy)
//     mask_bit   = extract bit from mask[]   at (sx, sy)
//
//     dest_x = x + sx
//     dest_y = y + sy
//
//     byte_index   = (dest_y / 8) * 128 + dest_x
//     bit_position = dest_y % 8
//
//     // Step 1: Clear the destination bit (punch a hole in the background)
//     //         only where the mask is opaque (mask_bit == 1).
//     //   fb[byte_index] &= ~(mask_bit << bit_position)
//     //   When mask_bit=0, ~(0 << bit) = ~0 = 0xFF → AND leaves bit unchanged.
//     //   When mask_bit=1, ~(1 << bit)       → AND clears exactly that bit.
//
//     // Step 2: Paint the sprite pixel into the cleared bit.
//     //   fb[byte_index] |= (sprite_bit & mask_bit) << bit_position
//     //   When mask_bit=0, sprite_bit & 0 = 0 → OR changes nothing.
//     //   When mask_bit=1, sprite_bit is painted as-is.
//
// Combined into one read-modify-write per pixel:
//   fb[i] = (fb[i] & ~(mask_bit << bit)) | ((sprite_bit & mask_bit) << bit)
//
// Note on performance: this iterates pixel-by-pixel for correctness and
// clarity. A byte-aligned sprite optimization (processing 8 pixels per
// iteration) can be added later if profiling on the ESP32 indicates a
// bottleneck — premature optimization is avoided here.
// -----------------------------------------------------------------------------
void drawSprite(FrameBuffer& fb,
                int x, int y,
                const uint8_t* sprite,
                const uint8_t* mask,
                int width, int height);
