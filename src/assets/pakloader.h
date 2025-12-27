#ifndef PAKLOADER_H
#define PAKLOADER_H

#include <stdint.h>
#include <stdio.h>

#define MAX_ASSET_NAME 64

typedef enum {
    ASSET_OBJ = 0,
    ASSET_TEXTURE,
    ASSET_SOUND,
    ASSET_UNKNOWN
} AssetType;

typedef struct {
    char name[MAX_ASSET_NAME];
    AssetType type;
    uint32_t offset;
    uint32_t size;
} AssetEntry;

typedef struct {
    FILE *file;
    uint32_t asset_count;
    AssetEntry *entries;
} PakFile;

int pak_open(PakFile *pak, const char *filename);

void pak_close(PakFile *pak);

AssetEntry* pak_find(PakFile *pak, const char *name);

uint8_t* pak_read_asset(PakFile *pak, AssetEntry *entry);

#endif // PAKLOADER_H