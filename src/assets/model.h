#ifndef MODEL_H
#define MODEL_H

#include "core/vec.h"
#include <stddef.h>

void normalize_model(Vec3* vertices, size_t vertex_count, float target_size);

#endif // MODEL_H
