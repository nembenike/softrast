#include "objloader.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static int parse_face_index(const char* token, int vertex_count) {
	int idx = 0;
	if (sscanf(token, "%d", &idx) != 1) return -1;
	if (idx < 0) idx = vertex_count + idx; // negative indices
	else idx = idx - 1; // OBJ 1-based
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
		// skip leading whitespace
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
				verts[verts_count].x = x;
				verts[verts_count].y = y;
				verts[verts_count].z = z;
				verts_count++;
			}
		} else if (line[0] == 'f' && line[1] == ' ') {
			// tokenize face indices (space-separated), take only vertex index before '/'
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
				// truncate at first '/'
				char* slash = strchr(tok, '/');
				if (slash) *slash = '\0';
				toks[n++] = tok;
			}

			if (n >= 3) {
				int v[4];
				for (int i = 0; i < n && i < 4; ++i) {
					v[i] = parse_face_index(toks[i], (int)verts_count);
				}

				// append triangle
				if (faces_count + 1 > faces_cap) {
					size_t nc = faces_cap ? faces_cap * 2 : 512;
					Face* nf = realloc(faces, nc * sizeof(Face));
					if (!nf) {
						for (int i=0;i<n;i++) free(toks[i]);
						goto fail;
					}
					faces = nf; faces_cap = nc;
				}
				faces[faces_count].v1 = v[0];
				faces[faces_count].v2 = v[1];
				faces[faces_count].v3 = v[2];
				faces_count++;

				// if quad, make second triangle
				if (n == 4) {
					if (faces_count + 1 > faces_cap) {
						size_t nc = faces_cap ? faces_cap * 2 : 512;
						Face* nf = realloc(faces, nc * sizeof(Face));
						if (!nf) {
							for (int i=0;i<n;i++) free(toks[i]);
							goto fail;
						}
						faces = nf; faces_cap = nc;
					}
					faces[faces_count].v1 = v[0];
					faces[faces_count].v2 = v[2];
					faces[faces_count].v3 = v[3];
					faces_count++;
				}
			}

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
	if (buf) free(buf);
	if (verts) free(verts);
	if (faces) free(faces);
	return 0;
}

void obj_free_mesh(Vec3* vertices, Face* faces) {
	if (vertices) free(vertices);
	if (faces) free(faces);
}
