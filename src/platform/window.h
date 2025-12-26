#ifndef WINDOW_H
#define WINDOW_H

typedef struct Window Window;

Window* window_create(int width, int height, const char* title);
void window_destroy(Window* w);
void window_poll_events(Window* w);
int window_should_close(Window* w);

void* window_get_handle(Window* w);

#endif
