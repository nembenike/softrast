#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <stddef.h>
#include <stdint.h>
#include "core/vec.h"

typedef struct { int v1, v2, v3; } Face;


int obj_parse_from_memory(const uint8_t* data, size_t size,
                          Vec3** out_vertices, size_t* out_vertex_count,
                          Face** out_faces, size_t* out_face_count);

void obj_free_mesh(Vec3* vertices, Face* faces);

#endif // OBJLOADER_H
