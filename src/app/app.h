#ifndef APP_H
#define APP_H

#include "../platform/window.h"
#include "../platform/input.h"
#include "../platform/time.h"

typedef struct App App;

App* app_create(int width, int height, const char* title);
void app_destroy(App* app);
void app_run(App* app);

#endif // APP_H
