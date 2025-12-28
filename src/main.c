#include "app/app.h"

int main(void) {
    App* app = app_create(1280, 720, "i am a teapot");
    if (!app) return 1;

    app_run(app);

    app_destroy(app);
    return 0;
}
