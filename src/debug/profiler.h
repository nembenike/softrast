#ifndef PROFILER_H
#define PROFILER_H

void profiler_init(void);
void profiler_record_draw(double dt);
void profiler_record_present(double dt);
void profiler_frame_end(void);

#endif // PROFILER_H
