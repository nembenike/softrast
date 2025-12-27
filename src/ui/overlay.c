#include "overlay.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../core/vec.h"

void overlay_draw_fps(Renderer* r, float delta_seconds) {
    if (!r) return;

    static const uint8_t digits[10][5] = {
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
            uint8_t bits = digits[digit][row];
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
