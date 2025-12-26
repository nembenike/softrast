#include "app/app.h"

int main(void) {
    App* app = app_create(800, 600, "Software Rasterizer");
    if (!app) return 1;

    app_run(app);

    app_destroy(app);
    return 0;
}
