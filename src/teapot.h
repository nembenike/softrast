#ifndef TEAPOT_H
#define TEAPOT_H

#include "core/vec.h"
#include <stddef.h>

typedef struct { int v1, v2, v3; } Face;

extern Vec3 vertices[];
extern Face faces[];
extern size_t vertex_count;
extern size_t face_count;

#endif // TEAPOT_H
