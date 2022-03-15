#include <SDL.h>
#include <SDL_ttf.h>

typedef struct Pixelgrid {
} Pixelgrid;

Pixelgrid *Pixelgrid_alloc(SDL_Texture *texture, Uint32 pixel_size);

void Pixelgrid_put(Pixelgrid *, Uint32 x, Uint32 y, SDL_Color color);

void Pixelgrid_clear(Pixelgrid *, SDL_Color color);

SDL_Texture *Pixelgrid_texture(Pixelgrid *grid);

void Pixelgrid_free(Pixelgrid *);
