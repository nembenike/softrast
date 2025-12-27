#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "renderer.h"
#include "../core/vec.h"

struct Renderer {
    int width, height;
    uint32_t* framebuffer;
    float* zbuffer;

    SDL_Window* sdl_window;
    SDL_Renderer* sdl_renderer;
    SDL_Texture* texture;
};

Renderer* renderer_create(int width, int height, void* window_handle) {
    Renderer* r = malloc(sizeof(Renderer));
    if (!r) return NULL;

    r->width = width;
    r->height = height;
    r->framebuffer = NULL;
    r->zbuffer = NULL;
    r->sdl_renderer = NULL;
    r->texture = NULL;
    r->sdl_window = (SDL_Window*)window_handle;

    r->framebuffer = malloc(width * height * sizeof(uint32_t));
    r->zbuffer = malloc(width * height * sizeof(float));
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

void renderer_draw_triangle(Renderer* r, Vec3 v0, Vec3 v1, Vec3 v2, uint32_t color) {
    int minX = (int)fmaxf(0.0f, fminf(fminf(v0.x, v1.x), v2.x));
    int minY = (int)fmaxf(0.0f, fminf(fminf(v0.y, v1.y), v2.y));
    int maxX = (int)fminf(r->width-1.0f, fmaxf(fmaxf(v0.x, v1.x), v2.x));
    int maxY = (int)fminf(r->height-1.0f, fmaxf(fmaxf(v0.y, v1.y), v2.y));

    float area = edge(v0, v1, v2.x, v2.y);
    if (fabsf(area) < 1e-6f) return;

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            float px = x + 0.5f;
            float py = y + 0.5f;

            float w0 = edge(v1, v2, px, py) / area;
            float w1 = edge(v2, v0, px, py) / area;
            float w2 = edge(v0, v1, px, py) / area;

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

void renderer_draw_line(Renderer* r, Vec3 p0, Vec3 p1, uint32_t color) {
    int x0 = (int)roundf(p0.x);
    int y0 = (int)roundf(p0.y);
    int x1 = (int)roundf(p1.x);
    int y1 = (int)roundf(p1.y);

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int steps = dx > dy ? dx : dy;
    if (steps == 0) {
        if (x0 >= 0 && x0 < r->width && y0 >= 0 && y0 < r->height) {
            int idx = y0 * r->width + x0;
            if (p0.z < r->zbuffer[idx]) {
                r->zbuffer[idx] = p0.z;
                r->framebuffer[idx] = color;
            }
        }
        return;
    }

    for (int i = 0; i <= steps; ++i) {
        float t = (float)i / (float)steps;
        int x = (int)roundf(x0 + (x1 - x0) * t);
        int y = (int)roundf(y0 + (y1 - y0) * t);
        float z = p0.z * (1.0f - t) + p1.z * t;

        if (x < 0 || x >= r->width || y < 0 || y >= r->height) continue;
        int idx = y * r->width + x;
        if (z < r->zbuffer[idx]) {
            r->zbuffer[idx] = z;
            r->framebuffer[idx] = color;
        }
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

void renderer_draw_triangle_shaded(Renderer* r, Vec3 v0, Vec3 v1, Vec3 v2, uint32_t c0, uint32_t c1, uint32_t c2) {
    int minX = (int)fmaxf(0.0f, fminf(fminf(v0.x, v1.x), v2.x));
    int minY = (int)fmaxf(0.0f, fminf(fminf(v0.y, v1.y), v2.y));
    int maxX = (int)fminf(r->width-1.0f, fmaxf(fmaxf(v0.x, v1.x), v2.x));
    int maxY = (int)fminf(r->height-1.0f, fmaxf(fmaxf(v0.y, v1.y), v2.y));

    float area = edge(v0, v1, v2.x, v2.y);
    if (fabsf(area) < 1e-6f) return;

    // unpack colors to float channels 0..255
    float r0 = (float)((c0 >> 16) & 0xFF);
    float g0 = (float)((c0 >> 8) & 0xFF);
    float b0 = (float)(c0 & 0xFF);
    float r1 = (float)((c1 >> 16) & 0xFF);
    float g1 = (float)((c1 >> 8) & 0xFF);
    float b1 = (float)(c1 & 0xFF);
    float r2c = (float)((c2 >> 16) & 0xFF);
    float g2c = (float)((c2 >> 8) & 0xFF);
    float b2c = (float)(c2 & 0xFF);

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            float px = x + 0.5f;
            float py = y + 0.5f;

            float w0 = edge(v1, v2, px, py) / area;
            float w1 = edge(v2, v0, px, py) / area;
            float w2 = edge(v0, v1, px, py) / area;

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float z = w0 * v0.z + w1 * v1.z + w2 * v2.z;
                int idx = y * r->width + x;

                if (z < r->zbuffer[idx]) {
                    r->zbuffer[idx] = z;

                    float rf = w0 * r0 + w1 * r1 + w2 * r2c;
                    float gf = w0 * g0 + w1 * g1 + w2 * g2c;
                    float bf = w0 * b0 + w1 * b1 + w2 * b2c;

                    uint32_t ri = (uint32_t)(rf + 0.5f) & 0xFF;
                    uint32_t gi = (uint32_t)(gf + 0.5f) & 0xFF;
                    uint32_t bi = (uint32_t)(bf + 0.5f) & 0xFF;
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

void renderer_present(Renderer* r) {
    SDL_UpdateTexture(r->texture, NULL, r->framebuffer, r->width * sizeof(uint32_t));
    SDL_RenderClear(r->sdl_renderer);
    SDL_RenderCopy(r->sdl_renderer, r->texture, NULL, NULL);
    SDL_RenderPresent(r->sdl_renderer);
}
