#include "overlay_helpers.h"
#include "overlay.h"
#include <string.h>

int overlay_text_pixel_width(const char* text, int scale) {
    if (!text) return 0;
    int len = (int)strlen(text);
    if (len <= 0) return 0;
    int spacing = scale;
    return len * (3 * scale) + (len - 1) * spacing;
}

void overlay_draw_centered_message(Renderer* r, const char* text, int width, int height, int scale, uint32_t color) {
    if (!text) return;
    int w = overlay_text_pixel_width(text, scale);
    int x = width/2 - w/2;
    if (x < 0) x = 0;
    overlay_draw_text(r, text, x, height/2 - 8, scale, color);
}
