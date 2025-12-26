#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>

#ifndef LOG_LEVEL
#define LOG_LEVEL 3
#endif

// 0 = none, 1 = error, 2 = warn, 3 = info

#define LOG_INTERNAL(level, stream, tag, fmt, ...) \
    do { \
        if (LOG_LEVEL >= level) { \
            fprintf(stream, "[%s] %s:%d: " fmt "\n", \
                    tag, __FILE__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

#define LOG_INFO(fmt, ...)  LOG_INTERNAL(3, stdout, "INFO",  fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  LOG_INTERNAL(2, stdout, "WARN",  fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_INTERNAL(1, stderr, "ERROR", fmt, ##__VA_ARGS__)

#define LOG_FATAL(fmt, ...) \
    do { \
        LOG_INTERNAL(1, stderr, "FATAL", fmt, ##__VA_ARGS__); \
        abort(); \
    } while (0)

#endif // LOG_H
