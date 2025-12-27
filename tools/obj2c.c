/*
Kinda obsolete but we'll keep it
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256
#define MAX_VERTICES 10000
#define MAX_FACES 10000

typedef struct { float x, y, z; } Vec3;
typedef struct { int v1, v2, v3; } Face;

int parse_face_index(const char* token, int vertex_count) {
    int idx = 0;
    sscanf(token, "%d", &idx);
    if (idx < 0) idx = vertex_count + idx; // negative indices
    else idx = idx - 1; // OBJ 1-based
    return idx;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <file.obj>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    Vec3 vertices[MAX_VERTICES];
    Face faces[MAX_FACES];
    int vert_count = 0;
    int face_count = 0;

    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, file)) {
        if (strncmp(line, "v ", 2) == 0) {
            if (vert_count >= MAX_VERTICES) break;
            sscanf(line + 2, "%f %f %f", &vertices[vert_count].x, &vertices[vert_count].y, &vertices[vert_count].z);
            vert_count++;
        } else if (strncmp(line, "f ", 2) == 0) {
            if (face_count >= MAX_FACES) break;
            char* tokens[4] = {0};
            int n = 0;
            char* ptr = strtok(line + 2, " \t\r\n");
            while (ptr && n < 4) {
                tokens[n++] = ptr;
                ptr = strtok(NULL, " \t\r\n");
            }
            if (n < 3) continue; // invalid face

            int v[4];
            for (int i = 0; i < n; i++) {
                // Extract vertex index before any slashes
                char* slash = strchr(tokens[i], '/');
                if (slash) *slash = 0;
                v[i] = parse_face_index(tokens[i], vert_count);
            }

            // Always create at least one triangle
            if (face_count < MAX_FACES) {
                faces[face_count].v1 = v[0];
                faces[face_count].v2 = v[1];
                faces[face_count].v3 = v[2];
                face_count++;
            }

            // If quad, split into second triangle
            if (n == 4 && face_count < MAX_FACES) {
                faces[face_count].v1 = v[0];
                faces[face_count].v2 = v[2];
                faces[face_count].v3 = v[3];
                face_count++;
            }
        }
    }

    fclose(file);

    // Generate C code
    printf("// Generated from OBJ file: %s\n", filename);
    printf("#include <stddef.h>\n\n");

    printf("typedef struct { float x, y, z; } Vec3;\n");
    printf("typedef struct { int v1, v2, v3; } Face;\n\n");

    printf("Vec3 vertices[%d] = {\n", vert_count);
    for (int i = 0; i < vert_count; i++)
        printf("    { %ff, %ff, %ff },\n", vertices[i].x, vertices[i].y, vertices[i].z);
    printf("};\n\n");

    printf("Face faces[%d] = {\n", face_count);
    for (int i = 0; i < face_count; i++)
        printf("    { %d, %d, %d },\n", faces[i].v1, faces[i].v2, faces[i].v3);
    printf("};\n");

    printf("\nsize_t vertex_count = %d;\n", vert_count);
    printf("size_t face_count = %d;\n", face_count);

    return 0;
}
