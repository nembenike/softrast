#ifndef TIME_H
#define TIME_H

#include <stdint.h>

typedef struct {
    uint64_t ticks;
    float delta_seconds;
    float total_seconds;
} Time;

void time_init(Time* time);
void time_update(Time* time);

#endif // TIME_H