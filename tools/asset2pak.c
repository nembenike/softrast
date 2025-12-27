#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef enum {
    ASSET_OBJ = 0,
    ASSET_TEXTURE,
    ASSET_SOUND,
    ASSET_UNKNOWN
} AssetType;

typedef struct {
    char name[64];
    uint32_t type;
    uint32_t offset;
    uint32_t size;
} AssetEntry;

// Infer asset type from file extension
AssetType get_asset_type(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return ASSET_UNKNOWN;
    if (strcmp(ext, ".obj") == 0) return ASSET_OBJ;
    if (strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0) return ASSET_TEXTURE;
    if (strcmp(ext, ".wav") == 0 || strcmp(ext, ".ogg") == 0) return ASSET_SOUND;
    return ASSET_UNKNOWN;
}

// Human-readable type string
const char *asset_type_str(AssetType type) {
    switch (type) {
        case ASSET_OBJ: return "OBJ";
        case ASSET_TEXTURE: return "TEXTURE";
        case ASSET_SOUND: return "SOUND";
        default: return "UNKNOWN";
    }
}

int pack_assets(const char **files, int n, const char *outPak) {
    FILE *pak = fopen(outPak, "wb");
    if (!pak) {
        fprintf(stderr, "Failed to open pak file for writing.\n");
        return -1;
    }

    uint32_t headerSize = sizeof(uint32_t) + n * sizeof(AssetEntry);
    fseek(pak, headerSize, SEEK_SET);

    AssetEntry *entries = malloc(n * sizeof(AssetEntry));
    uint32_t offset = headerSize;
    uint32_t totalDataSize = 0;

    for (int i = 0; i < n; i++) {
        FILE *f = fopen(files[i], "rb");
        if (!f) {
            fprintf(stderr, "Failed to open asset file: %s\n", files[i]);
            free(entries);
            fclose(pak);
            return -2;
        }

        fseek(f, 0, SEEK_END);
        uint32_t size = ftell(f);
        fseek(f, 0, SEEK_SET);

        uint8_t *data = malloc(size);
        fread(data, 1, size, f);
        fclose(f);

        fwrite(data, 1, size, pak);
        free(data);

        // Fill TOC entry
        const char *nameOnly = strrchr(files[i], '/');
        if (!nameOnly) nameOnly = files[i];
        else nameOnly++; // skip '/'

        strncpy(entries[i].name, nameOnly, 63);
        entries[i].name[63] = 0;

        entries[i].type = get_asset_type(files[i]);
        entries[i].offset = offset;
        entries[i].size = size;

        offset += size;
        totalDataSize += size;

        printf("Packed asset: %s (%s), size: %u bytes\n",
               entries[i].name, asset_type_str(entries[i].type), size);
    }

    // Write TOC at the start
    fseek(pak, 0, SEEK_SET);
    fwrite(&n, sizeof(uint32_t), 1, pak);
    fwrite(entries, sizeof(AssetEntry), n, pak);

    free(entries);
    fclose(pak);

    printf("Total pak size (including TOC): %u bytes\n", totalDataSize + headerSize);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <outPak> <asset1> <asset2> [...]\n", argv[0]);
        return 1;
    }

    const char *outPak = argv[1];
    int assetCount = argc - 2;
    const char **files = (const char **)&argv[2];

    return pack_assets(files, assetCount, outPak);
}

