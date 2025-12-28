#include "profiler.h"
#include <stdio.h>

static double frame_time = 0.0, draw_time = 0.0, present_time = 0.0;
static int frame_count = 0;

void profiler_init(void) {
    frame_time = draw_time = present_time = 0.0;
    frame_count = 0;
}

void profiler_record_draw(double dt) {
    draw_time += dt;
}

void profiler_record_present(double dt) {
    present_time += dt;
}

void profiler_frame_end(void) {
    frame_count++;
    if (frame_count % 300 == 0) {
        printf("[PROFILE] avg draw: %.4f ms, present: %.4f ms\n",
               (draw_time / frame_count) * 1000.0,
               (present_time / frame_count) * 1000.0);
    }
}
