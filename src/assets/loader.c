#include "loader.h"
#include "assets/pakloader.h"
#include "assets/objloader.h"
#include "core/log.h"
#include <string.h>
#include <stdlib.h>

int assets_load_model_from_pak(const char* pak_path, const char* asset_name,
                               Vec3** out_vertices, Face** out_faces,
                               size_t* out_vertex_count, size_t* out_face_count,
                               char* out_message, size_t out_message_len) {
    if (!pak_path || !asset_name) {
        if (out_message && out_message_len) snprintf(out_message, out_message_len, "invalid args");
        return 0;
    }

    PakFile pak = {0};
    if (!pak_open(&pak, pak_path)) {
        if (out_message && out_message_len) snprintf(out_message, out_message_len, "Failed to open %s", pak_path);
        return 0;
    }

    AssetEntry* e = pak_find(&pak, asset_name);
    if (!e) {
        if (out_message && out_message_len) snprintf(out_message, out_message_len, "%s not found", asset_name);
        pak_close(&pak);
        return 0;
    }

    uint8_t* data = pak_read_asset(&pak, e);
    if (!data) {
        if (out_message && out_message_len) snprintf(out_message, out_message_len, "Failed to read %s", asset_name);
        pak_close(&pak);
        return 0;
    }

    int ok = 0;
    if (obj_parse_from_memory(data, e->size, out_vertices, out_vertex_count, out_faces, out_face_count)) {
        LOG_INFO("Loaded %s: %zu vertices, %zu faces", asset_name, *out_vertex_count, *out_face_count);
        ok = 1;
    } else {
        if (out_message && out_message_len) snprintf(out_message, out_message_len, "Failed to parse %s", asset_name);
        ok = 0;
    }

    free(data);
    pak_close(&pak);
    return ok;
}
