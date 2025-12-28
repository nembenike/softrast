#ifndef OVERLAY_HELPERS_H
#define OVERLAY_HELPERS_H

#include "../renderer/renderer.h"

int overlay_text_pixel_width(const char* text, int scale);
void overlay_draw_centered_message(Renderer* r, const char* text, int width, int height, int scale, uint32_t color);

#endif // OVERLAY_HELPERS_H
