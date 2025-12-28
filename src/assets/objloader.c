#include "objloader.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static int parse_face_index(const char* token, int vertex_count) {
    int idx = 0;
    if (sscanf(token, "%d", &idx) != 1) return -1;
    if (idx < 0) idx = vertex_count + idx;
    else idx = idx - 1;
    if (idx < 0 || idx >= vertex_count) return -1;
    return idx;
}

int obj_parse_from_memory(const uint8_t* data, size_t size,
                          Vec3** out_vertices, size_t* out_vertex_count,
                          Face** out_faces, size_t* out_face_count) {
    if (!data || size == 0 || !out_vertices || !out_vertex_count || !out_faces || !out_face_count) return 0;

    char* buf = malloc(size + 1);
    if (!buf) return 0;
    memcpy(buf, data, size);
    buf[size] = '\0';

    Vec3* verts = NULL;
    size_t verts_cap = 0;
    size_t verts_count = 0;

    Face* faces = NULL;
    size_t faces_cap = 0;
    size_t faces_count = 0;

    char* line = strtok(buf, "\r\n");
    while (line) {
        while (*line && isspace((unsigned char)*line)) ++line;
        if (line[0] == 'v' && line[1] == ' ') {
            float x, y, z;
            if (sscanf(line + 2, "%f %f %f", &x, &y, &z) == 3) {
                if (verts_count + 1 > verts_cap) {
                    size_t nc = verts_cap ? verts_cap * 2 : 256;
                    Vec3* nv = realloc(verts, nc * sizeof(Vec3));
                    if (!nv) goto fail;
                    verts = nv; verts_cap = nc;
                }
                verts[verts_count++] = (Vec3){x, y, z};
            }
        } else if (line[0] == 'f' && line[1] == ' ') {
            const char* p = line + 2;
            char* toks[8];
            int n = 0;
            while (*p && n < 8) {
                while (*p && isspace((unsigned char)*p)) ++p;
                if (!*p) break;
                const char* start = p;
                while (*p && !isspace((unsigned char)*p)) ++p;
                int len = (int)(p - start);
                char* tok = malloc(len + 1);
                if (!tok) goto fail;
                memcpy(tok, start, len);
                tok[len] = '\0';
                char* slash = strchr(tok, '/');
                if (slash) *slash = '\0';
                toks[n++] = tok;
            }

            if (n >= 3) {
                int v[4];
                int valid = 1;
                for (int i = 0; i < n && i < 4; ++i) {
                    v[i] = parse_face_index(toks[i], (int)verts_count);
                    if (v[i] < 0) valid = 0;
                }
                if (valid) {
                    if (faces_count + 1 > faces_cap) {
                        size_t nc = faces_cap ? faces_cap * 2 : 512;
                        Face* nf = realloc(faces, nc * sizeof(Face));
                        if (!nf) goto fail_free_toks;
                        faces = nf; faces_cap = nc;
                    }
                    faces[faces_count++] = (Face){v[0], v[1], v[2]};
                    if (n == 4) {
                        if (faces_count + 1 > faces_cap) {
                            size_t nc = faces_cap ? faces_cap * 2 : 512;
                            Face* nf = realloc(faces, nc * sizeof(Face));
                            if (!nf) goto fail_free_toks;
                            faces = nf; faces_cap = nc;
                        }
                        faces[faces_count++] = (Face){v[0], v[2], v[3]};
                    }
                }
            }

        fail_free_toks:
            for (int i = 0; i < n; ++i) free(toks[i]);
        }
        line = strtok(NULL, "\r\n");
    }

    free(buf);
    *out_vertices = verts;
    *out_vertex_count = verts_count;
    *out_faces = faces;
    *out_face_count = faces_count;
    return 1;

fail:
    free(buf);
    if (verts) free(verts);
    if (faces) free(faces);
    return 0;
}

void obj_free_mesh(Vec3* vertices, Face* faces) {
    if (vertices) free(vertices);
    if (faces) free(faces);
}
