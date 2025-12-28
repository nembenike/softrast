#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "renderer.h"
#include "../core/vec.h"
#include "../core/math.h"

struct Renderer {
    int width, height;
    uint32_t* framebuffer;
    float* zbuffer;
    SDL_Window* sdl_window;
    SDL_Renderer* sdl_renderer;
    SDL_Texture* texture;
    RendererWindingOrder winding_order;
};

Renderer* renderer_create(int width, int height, void* window_handle) {
    Renderer* r = malloc(sizeof(Renderer));
    if (!r) return NULL;

    r->width = width;
    r->height = height;
    r->framebuffer = malloc(width * height * sizeof(uint32_t));
    r->zbuffer = malloc(width * height * sizeof(float));
    r->sdl_renderer = NULL;
    r->texture = NULL;
    r->sdl_window = (SDL_Window*)window_handle;
    r->winding_order = RENDERER_WINDING_CCW;

    if (!r->framebuffer || !r->zbuffer) {
        renderer_destroy(r);
        return NULL;
    }

    r->sdl_renderer = SDL_CreateRenderer(r->sdl_window, -1, SDL_RENDERER_ACCELERATED);
    if (!r->sdl_renderer) {
        renderer_destroy(r);
        return NULL;
    }

    r->texture = SDL_CreateTexture(
        r->sdl_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    );

    if (!r->texture) {
        renderer_destroy(r);
        return NULL;
    }

    return r;
}

void renderer_destroy(Renderer* r) {
    if (!r) return;
    if (r->texture) SDL_DestroyTexture(r->texture);
    if (r->sdl_renderer) SDL_DestroyRenderer(r->sdl_renderer);
    free(r->framebuffer);
    free(r->zbuffer);
    free(r);
}

void renderer_clear(Renderer* r, uint32_t color) {
    int count = r->width * r->height;
    for (int i = 0; i < count; i++) {
        r->framebuffer[i] = color;
        r->zbuffer[i] = FLT_MAX;
    }
}

static float edge(Vec3 a, Vec3 b, float x, float y) {
    return (b.x - a.x) * (y - a.y) - (b.y - a.y) * (x - a.x);
}

void renderer_set_winding_order(Renderer* r, RendererWindingOrder order) {
    if (r) r->winding_order = order;
}

void renderer_draw_triangle(Renderer* r, Vec3 v0, Vec3 v1, Vec3 v2, uint32_t color) {
    if (r->winding_order == RENDERER_WINDING_CW) {
        Vec3 tmp = v1; v1 = v2; v2 = tmp;
    }

    int minX = (int)fmaxf(0.0f, fminf(fminf(v0.x, v1.x), v2.x));
    int minY = (int)fmaxf(0.0f, fminf(fminf(v0.y, v1.y), v2.y));
    int maxX = (int)fminf((float)(r->width - 1), fmaxf(fmaxf(v0.x, v1.x), v2.x));
    int maxY = (int)fminf((float)(r->height - 1), fmaxf(fmaxf(v0.y, v1.y), v2.y));

    float area = edge(v0, v1, v2.x, v2.y);
    if (fabsf(area) < 1e-6f) return;

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            float px = x + 0.5f;
            float py = y + 0.5f;

            float w0 = edge(v1, v2, px, py) / area;
            float w1 = edge(v2, v0, px, py) / area;
            float w2 = 1.0f - w0 - w1;

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float z = w0 * v0.z + w1 * v1.z + w2 * v2.z;
                int idx = y * r->width + x;
                if (z < r->zbuffer[idx]) {
                    r->zbuffer[idx] = z;
                    r->framebuffer[idx] = color;
                }
            }
        }
    }
}

void renderer_draw_triangle_shaded(Renderer* r, Vec3 v0, Vec3 v1, Vec3 v2, uint32_t c0, uint32_t c1, uint32_t c2) {
    int minX = (int)fmaxf(0.0f, fminf(fminf(v0.x, v1.x), v2.x));
    int minY = (int)fmaxf(0.0f, fminf(fminf(v0.y, v1.y), v2.y));
    int maxX = (int)fminf((float)(r->width - 1), fmaxf(fmaxf(v0.x, v1.x), v2.x));
    int maxY = (int)fminf((float)(r->height - 1), fmaxf(fmaxf(v0.y, v1.y), v2.y));

    float area = edge(v0, v1, v2.x, v2.y);
    if (fabsf(area) < 1e-6f) return;

    float r0 = (float)((c0 >> 16) & 0xFF);
    float g0 = (float)((c0 >> 8) & 0xFF);
    float b0 = (float)(c0 & 0xFF);

    float r1 = (float)((c1 >> 16) & 0xFF);
    float g1 = (float)((c1 >> 8) & 0xFF);
    float b1 = (float)(c1 & 0xFF);

    float r2 = (float)((c2 >> 16) & 0xFF);
    float g2 = (float)((c2 >> 8) & 0xFF);
    float b2 = (float)(c2 & 0xFF);

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            float px = x + 0.5f;
            float py = y + 0.5f;

            float w0 = edge(v1, v2, px, py) / area;
            float w1 = edge(v2, v0, px, py) / area;
            float w2 = 1.0f - w0 - w1;

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float z = w0 * v0.z + w1 * v1.z + w2 * v2.z;
                int idx = y * r->width + x;
                if (z < r->zbuffer[idx]) {
                    r->zbuffer[idx] = z;

                    float rf = w0*r0 + w1*r1 + w2*r2;
                    float gf = w0*g0 + w1*g1 + w2*g2;
                    float bf = w0*b0 + w1*b1 + w2*b2;

                    uint32_t ri = (uint32_t)clampf(rf + 0.5f, 0.0f, 255.0f);
                    uint32_t gi = (uint32_t)clampf(gf + 0.5f, 0.0f, 255.0f);
                    uint32_t bi = (uint32_t)clampf(bf + 0.5f, 0.0f, 255.0f);

                    r->framebuffer[idx] = 0xFF000000 | (ri << 16) | (gi << 8) | bi;
                }
            }
        }
    }
}

Vec3 ndc_to_screen(Vec3 v, int width, int height) {
    return (Vec3){
        (v.x + 1.0f) * 0.5f * width,
        (1.0f - (v.y + 1.0f) * 0.5f) * height,
        v.z
    };
}

void renderer_draw_line(Renderer* r, Vec3 p0, Vec3 p1, uint32_t color) {
    int x0 = (int)clampf(p0.x, 0.0f, r->width-1.0f);
    int y0 = (int)clampf(p0.y, 0.0f, r->height-1.0f);
    int x1 = (int)clampf(p1.x, 0.0f, r->width-1.0f);
    int y1 = (int)clampf(p1.y, 0.0f, r->height-1.0f);
    float z0 = p0.z;
    float z1 = p1.z;

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int n = (dx > dy ? dx : dy) + 1;

    for (int i = 0; i < n; ++i) {
        if (x0 >= 0 && x0 < r->width && y0 >= 0 && y0 < r->height) {
            float t = n > 1 ? (float)i / (float)(n-1) : 0.0f;
            float z = lerpf(z0, z1, t);
            int idx = y0 * r->width + x0;
            if (z < r->zbuffer[idx]) {
                r->zbuffer[idx] = z;
                r->framebuffer[idx] = color;
            }
        }
        int e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 < dy)  { err += dx; y0 += sy; }
    }
}

void renderer_draw_rect(Renderer* r, int x, int y, int w, int h, uint32_t color) {
    if (!r || !r->framebuffer) return;
    if (w <= 0 || h <= 0) return;

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = x + w - 1;
    int y1 = y + h - 1;
    if (x1 >= r->width) x1 = r->width - 1;
    if (y1 >= r->height) y1 = r->height - 1;

    for (int yy = y0; yy <= y1; ++yy) {
        int base = yy * r->width;
        for (int xx = x0; xx <= x1; ++xx) {
            r->framebuffer[base + xx] = color;
        }
    }
}

void renderer_present(Renderer* r) {
    SDL_UpdateTexture(r->texture, NULL, r->framebuffer, r->width * sizeof(uint32_t));
    SDL_RenderClear(r->sdl_renderer);
    SDL_RenderCopy(r->sdl_renderer, r->texture, NULL, NULL);
    SDL_RenderPresent(r->sdl_renderer);
}
