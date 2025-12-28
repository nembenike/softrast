#ifndef UI_OVERLAY_H
#define UI_OVERLAY_H

#include "renderer/renderer.h"

void overlay_draw_fps(Renderer* r, float delta_seconds);
void overlay_draw_text(Renderer* r, const char* text, int x, int y, int scale, uint32_t color);

#endif // UI_OVERLAY_H
