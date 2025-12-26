#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "../platform/window.h"
#include "../platform/input.h"
#include "../platform/time.h"
#include "../renderer/renderer.h"

struct App {
    Window* window;
    Input input;
    Time time;
    Renderer* renderer;
};

App* app_create(int width, int height, const char* title) {
    App* app = malloc(sizeof(App));
    if (!app) return NULL;

    app->window = window_create(width, height, title);
    time_init(&app->time);
    memset(&app->input, 0, sizeof(Input));

    app->renderer = renderer_create(width, height, window_get_handle(app->window));
    return app;
}

void app_destroy(App* app) {
    if (!app) return;

    renderer_destroy(app->renderer);
    window_destroy(app->window);
    free(app);
}

void app_run(App* app) {
    while (!window_should_close(app->window)) {
        window_poll_events(app->window);
        input_update(&app->input);
        time_update(&app->time);

        renderer_clear(app->renderer, 0xFF000000);
        renderer_draw_triangle(app->renderer, (Vec3){100, 100, 0}, (Vec3){200, 100, 0}, (Vec3){150, 200, 0}, 0xFFFF0000);

        renderer_present(app->renderer);
    }
}
