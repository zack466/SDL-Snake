#include <SDL_stdinc.h>
#include <stdbool.h>

typedef struct Bitset {
} Bitset;

Bitset *Bitset_alloc(Uint32 length);

void Bitset_free(Bitset *bitset);

bool Bitset_get(Bitset *bitset, Uint32 idx);

void Bitset_set(Bitset *bitset, Uint32 idx);

void Bitset_clear(Bitset *bitset, Uint32 idx);

void Bitset_print(Bitset *bitset);
