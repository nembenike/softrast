#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "renderer.h"


struct Renderer {
    int width, height;
    uint32_t* framebuffer;

    SDL_Window* sdl_window;
    SDL_Renderer* sdl_renderer;
    SDL_Texture* texture;
};

Renderer* renderer_create(int width, int height, void* window_handle) {
    SDL_Window* window = (SDL_Window*)window_handle;
    Renderer* r = malloc(sizeof(Renderer));
    if (!r) return NULL;
    r->width = width;
    r->height = height;
    r->framebuffer = malloc(width*height*sizeof(uint32_t));
    r->sdl_window = window;
    r->sdl_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    r->texture = SDL_CreateTexture(r->sdl_renderer, SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING, width, height);
    return r;
}

void renderer_destroy(Renderer* r) {
    if (!r) return;
    SDL_DestroyTexture(r->texture);
    SDL_DestroyRenderer(r->sdl_renderer);
    free(r->framebuffer);
    free(r);
}

void renderer_clear(Renderer* r, uint32_t color) {
    for (int i = 0; i < r->width * r->height; i++)
        r->framebuffer[i] = color;
}

static void draw_pixel(Renderer* r, int x, int y, uint32_t color) {
    if (x < 0 || x >= r->width || y < 0 || y >= r->height) return;
    r->framebuffer[y * r->width + x] = color;
}

static float edge(Vec3 a, Vec3 b, float x, float y) {
    return (b.x - a.x)*(y - a.y) - (b.y - a.y)*(x - a.x);
}

void renderer_draw_triangle(Renderer* r, Vec3 v0, Vec3 v1, Vec3 v2, uint32_t color) {
    int minX = (int)fminf(fminf(v0.x,v1.x),v2.x);
    int minY = (int)fminf(fminf(v0.y,v1.y),v2.y);
    int maxX = (int)fmaxf(fmaxf(v0.x,v1.x),v2.x);
    int maxY = (int)fmaxf(fmaxf(v0.y,v1.y),v2.y);

    if (minX < 0) minX = 0; 
    if (minY < 0) minY = 0;
    if (maxX >= r->width) maxX = r->width-1;
    if (maxY >= r->height) maxY = r->height-1;

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            float w0 = edge(v1,v2,(float)x+0.5f,(float)y+0.5f);
            float w1 = edge(v2,v0,(float)x+0.5f,(float)y+0.5f);
            float w2 = edge(v0,v1,(float)x+0.5f,(float)y+0.5f);

            if (w0>=0 && w1>=0 && w2>=0)
                draw_pixel(r,x,y,color);
        }
    }
}

void renderer_present(Renderer* r) {
    SDL_UpdateTexture(r->texture, NULL, r->framebuffer, r->width * sizeof(uint32_t));
    SDL_RenderClear(r->sdl_renderer);
    SDL_RenderCopy(r->sdl_renderer, r->texture, NULL, NULL);
    SDL_RenderPresent(r->sdl_renderer);
}
