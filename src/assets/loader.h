#ifndef ASSETS_LOADER_H
#define ASSETS_LOADER_H

#include <stddef.h>
#include <stdint.h>
#include "../core/vec.h"
#include "../assets/objloader.h"

// Loads `asset_name` from `pak_path`. On success returns 1 and fills out_* fields.
// On failure returns 0 and writes a short human message to out_message (if provided).
int assets_load_model_from_pak(const char* pak_path, const char* asset_name,
                               Vec3** out_vertices, Face** out_faces,
                               size_t* out_vertex_count, size_t* out_face_count,
                               char* out_message, size_t out_message_len);

#endif // ASSETS_LOADER_H
