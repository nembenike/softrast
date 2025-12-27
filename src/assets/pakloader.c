#include "pakloader.h"
#include <stdlib.h>
#include <string.h>

int pak_open(PakFile *pak, const char *filename) {
    pak->file = fopen(filename, "rb");
    if (!pak->file) return 0;

    if (fread(&pak->asset_count, sizeof(uint32_t), 1, pak->file) != 1) return 0;

    pak->entries = malloc(sizeof(AssetEntry) * pak->asset_count);
    if (!pak->entries) return 0;

    if (fread(pak->entries, sizeof(AssetEntry), pak->asset_count, pak->file) != pak->asset_count)
        return 0;

    return 1;
}

void pak_close(PakFile *pak) {
    if (!pak) return;
    if (pak->file) fclose(pak->file);
    if (pak->entries) free(pak->entries);
    pak->file = NULL;
    pak->entries = NULL;
    pak->asset_count = 0;
}

AssetEntry* pak_find(PakFile *pak, const char *name) {
    if (!pak || !pak->entries) return NULL;
    for (uint32_t i = 0; i < pak->asset_count; i++) {
        if (strcmp(pak->entries[i].name, name) == 0) return &pak->entries[i];
    }
    return NULL;
}

uint8_t* pak_read_asset(PakFile *pak, AssetEntry *entry) {
    if (!pak || !pak->file || !entry) return NULL;
    uint8_t *data = malloc(entry->size);
    if (!data) return NULL;

    fseek(pak->file, entry->offset, SEEK_SET);
    if (fread(data, 1, entry->size, pak->file) != entry->size) {
        free(data);
        return NULL;
    }

    return data;
}
