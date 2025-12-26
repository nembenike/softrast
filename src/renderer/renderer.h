// renderer.h
#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>
#include "../core/vec.h"

typedef struct Renderer Renderer;

Renderer* renderer_create(int width, int height, void* window_handle); // opaque
void renderer_destroy(Renderer* r);

void renderer_clear(Renderer* r, uint32_t color);
void renderer_present(Renderer* r);
void renderer_draw_triangle(Renderer* r, Vec3 v0, Vec3 v1, Vec3 v2, uint32_t color);

#endif // RENDERER_H
