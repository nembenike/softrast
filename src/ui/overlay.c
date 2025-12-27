#include "overlay.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../core/vec.h"

// Shared glyph tables (3x5 font)
static const uint8_t glyph_digits[10][5] = {
    {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
    {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
    {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
    {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
    {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
    {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
    {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
    {0b111, 0b001, 0b010, 0b100, 0b100}, // 7
    {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
    {0b111, 0b101, 0b111, 0b001, 0b111}  // 9
};

static const uint8_t glyph_letters[26][5] = {
    {0b111,0b101,0b111,0b101,0b101}, // A
    {0b110,0b101,0b110,0b101,0b110}, // B
    {0b111,0b100,0b100,0b100,0b111}, // C
    {0b110,0b101,0b101,0b101,0b110}, // D
    {0b111,0b100,0b110,0b100,0b111}, // E
    {0b111,0b100,0b110,0b100,0b100}, // F
    {0b111,0b100,0b101,0b101,0b111}, // G
    {0b101,0b101,0b111,0b101,0b101}, // H
    {0b111,0b010,0b010,0b010,0b111}, // I
    {0b001,0b001,0b001,0b101,0b111}, // J
    {0b101,0b110,0b100,0b110,0b101}, // K (approx)
    {0b100,0b100,0b100,0b100,0b111}, // L
    {0b101,0b111,0b111,0b101,0b101}, // M (approx)
    {0b101,0b111,0b111,0b111,0b101}, // N (approx)
    {0b111,0b101,0b101,0b101,0b111}, // O
    {0b111,0b101,0b111,0b100,0b100}, // P
    {0b111,0b101,0b101,0b111,0b001}, // Q (approx)
    {0b110,0b101,0b110,0b101,0b101}, // R
    {0b111,0b100,0b111,0b001,0b111}, // S
    {0b111,0b010,0b010,0b010,0b010}, // T
    {0b101,0b101,0b101,0b101,0b111}, // U
    {0b101,0b101,0b101,0b010,0b010}, // V
    {0b101,0b101,0b111,0b111,0b101}, // W (approx)
    {0b101,0b010,0b010,0b010,0b101}, // X (approx)
    {0b101,0b101,0b111,0b010,0b010}, // Y (approx)
    {0b111,0b001,0b010,0b100,0b111}  // Z
};

// Punctuation/glyphs
static const uint8_t glyph_dot[5] = {0b000,0b000,0b000,0b000,0b010};
static const uint8_t glyph_slash[5] = {0b001,0b001,0b010,0b010,0b100};
static const uint8_t glyph_colon[5] = {0b000,0b010,0b000,0b010,0b000};
static const uint8_t glyph_dash[5] = {0b000,0b000,0b111,0b000,0b000};
static const uint8_t glyph_underscore[5] = {0b000,0b000,0b000,0b000,0b111};
static const uint8_t glyph_plus[5] = {0b000,0b010,0b111,0b010,0b000};
static const uint8_t glyph_comma[5] = {0b000,0b000,0b000,0b010,0b010};
static const uint8_t glyph_exclaim[5] = {0b010,0b010,0b010,0b000,0b010};
static const uint8_t glyph_question[5] = {0b111,0b001,0b010,0b000,0b010};

void overlay_draw_fps(Renderer* r, float delta_seconds) {
    if (!r) return;

    float dt = delta_seconds;
    if (dt <= 0.0f) dt = 1.0f/60.0f;
    int fps = (int)(1.0f / dt + 0.5f);
    if (fps < 0) fps = 0;

    char buf[16];
    int len = 0;
    int tmp = fps;
    if (tmp == 0) buf[len++] = '0';
    else {
        char rev[16]; int rlen = 0;
        while (tmp > 0 && rlen < 15) { rev[rlen++] = '0' + (tmp % 10); tmp /= 10; }
        for (int i = rlen-1; i >= 0; --i) buf[len++] = rev[i];
    }

    const int scale = 4;
    const int h = 5 * scale;
    const int margin = 4;
    const int spacing = 2;
    int digits_width = len * (3 * scale) + (len - 1) * spacing;
    int box_w = digits_width + margin * 2;
    int box_h = h + margin * 2;
    int x0 = 8;
    int y0 = 8;

    renderer_draw_rect(r, x0, y0, box_w, box_h, 0xFF202020);

        int cx = x0 + margin;
        for (int d = 0; d < len; ++d) {
            int digit = buf[d] - '0';
            for (int row = 0; row < 5; ++row) {
                uint8_t bits = glyph_digits[digit][row];
                for (int col = 0; col < 3; ++col) {
                    if (bits & (1 << (2 - col))) {
                        int px = cx + col * scale;
                        int py = y0 + margin + row * scale;
                        renderer_draw_rect(r, px, py, scale, scale, 0xFFFFFFFF);
                    }
                }
            }
            cx += 3 * scale + spacing;
        }
}

void overlay_draw_text(Renderer* r, const char* text, int x, int y, int scale, uint32_t color) {
    if (!r || !text) return;

    int cx = x;
    const int spacing = scale * 1;
    for (const char* p = text; *p; ++p) {
        char c = *p;
        if (c == ' ') { cx += (3 * scale) + spacing; continue; }
        if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';

        const uint8_t* glyph = NULL;
        if (c >= 'A' && c <= 'Z') {
            glyph = glyph_letters[c - 'A'];
        } else if (c >= '0' && c <= '9') {
            glyph = glyph_digits[c - '0'];
        } else {
            switch (c) {
                case '.': glyph = glyph_dot; break;
                case '/': glyph = glyph_slash; break;
                case ':': glyph = glyph_colon; break;
                case '-': glyph = glyph_dash; break;
                case '_': glyph = glyph_underscore; break;
                case '+': glyph = glyph_plus; break;
                case ',': glyph = glyph_comma; break;
                case '!': glyph = glyph_exclaim; break;
                case '?': glyph = glyph_question; break;
                default: glyph = NULL; break;
            }
        }
        if (!glyph) { cx += (3 * scale) + spacing; continue; }

        for (int row = 0; row < 5; ++row) {
            uint8_t bits = glyph[row];
            for (int col = 0; col < 3; ++col) {
                if (bits & (1 << (2 - col))) {
                    int px = cx + col * scale;
                    int py = y + row * scale;
                    renderer_draw_rect(r, px, py, scale, scale, color);
                }
            }
        }
        cx += (3 * scale) + spacing;
    }
}
