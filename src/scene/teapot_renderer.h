#ifndef TEAPOT_RENDERER_H
#define TEAPOT_RENDERER_H

#include "../core/vec.h"
#include "../assets/objloader.h"
#include "../core/mat.h"
#include "../renderer/renderer.h"

typedef struct TeapotRenderer TeapotRenderer;

TeapotRenderer* teapot_renderer_create(const Vec3* vertices, const Face* faces, size_t vertex_count, size_t face_count);
void teapot_renderer_destroy(TeapotRenderer* t);

int teapot_renderer_update(TeapotRenderer* t, Mat4 model, Mat4 view, Mat4 proj, Vec3 camera_pos, int width, int height);
void teapot_renderer_draw(TeapotRenderer* t, Renderer* r, int wireframe_pref);

#endif // TEAPOT_RENDERER_H
