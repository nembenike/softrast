#include "arena.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static size_t align_forward(size_t ptr, size_t align) {
    size_t mod = ptr & (align - 1);
    if (mod) ptr += align - mod;
    return ptr;
}

void arena_init(Arena* arena, size_t size) {
    arena->memory = (uint8_t*)malloc(size);
    assert(arena->memory && "Arena allocation failed");
    arena->size = size;
    arena->used = 0;
}

void arena_free(Arena* arena) {
    free(arena->memory);
    arena->memory = NULL;
    arena->size = 0;
    arena->used = 0;
}

void* arena_alloc(Arena* arena, size_t size) {
    return arena_alloc_aligned(arena, size, 8);
}

void* arena_alloc_aligned(Arena* arena, size_t size, size_t alignment) {
    size_t current = (size_t)(arena->memory + arena->used);
    size_t aligned = align_forward(current, alignment);
    size_t offset = aligned - (size_t)arena->memory;

    if (offset + size > arena->size) {
        assert(0 && "Arena out of memory");
        return NULL;
    }

    arena->used = offset + size;
    return arena->memory + offset;
}

void* arena_alloc_zero(Arena* arena, size_t size) {
    void* ptr = arena_alloc(arena, size);
    if (ptr) memset(ptr, 0, size);
    return ptr;
}

void arena_reset(Arena* arena) {
    arena->used = 0;
}
