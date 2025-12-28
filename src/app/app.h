#ifndef APP_H
#define APP_H

#include "platform/input.h"
#include "renderer/renderer.h"
#include "core/camera.h"
#include "platform/time.h"
#include "platform/window.h"

typedef struct App App;

App* app_create(int width, int height, const char* title);
void app_destroy(App* app);
void app_run(App* app);

#endif
