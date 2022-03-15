#include <SDL_stdinc.h>
#include <stdbool.h>

// compactly represents bool[N] for a large N
// Essentially wraps arr[N / 64] & (1 << (N % 64))
typedef struct Bitset {
  Uint32 length;
  Uint64 *data;
} Bitset;

Bitset *
Bitset_alloc(Uint32 length)
{
  Uint32 num_uints = 1 + length / 64;
  Bitset *ret = malloc(sizeof(Bitset));
  ret->length = num_uints;
  ret->data = calloc(num_uints, sizeof(Uint64));
  memset(ret->data, 0, num_uints * sizeof(Uint64));
  return ret;
}

void
Bitset_free(Bitset *bitset)
{
  free(bitset->data);
  free(bitset);
}

bool
Bitset_get(Bitset *bitset, Uint32 idx)
{
  if (idx > bitset->length * 64) {
    printf("Bitset_get error: out of range error: %d\n", idx);
    return false;
  } else {
    return bitset->data[idx / 64] & (1L << (idx % 64));
  }
}

void
Bitset_set(Bitset *bitset, Uint32 idx)
{
  if (idx > bitset->length * 64) {
    printf("Bitset_set error: out of range error: %d\n", idx);
  } else {
    bitset->data[idx / 64] |= (1L << (idx % 64));
  }
}

void
Bitset_clear(Bitset *bitset, Uint32 idx)
{
  if (idx > bitset->length * 64) {
    printf("Bitset_clear error: out of range error: %d\n", idx);
  } else {
    bitset->data[idx / 64] &= ~(1L << (idx % 64));
  }
}

void
Bitset_print(Bitset *bitset)
{
  for (int i = 0; i < bitset->length; i++) {
    for (int j = 0; j < 64; j++) {
      printf("%d", (bitset->data[i] & (1L << j)) != 0);
    }
  }
  printf("\n");
}
