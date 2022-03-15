#include <SDL.h>
#include <SDL_ttf.h>
#include "config.h"

// provides convenient ways to modify a texture as if it were a grid of pixels
// does not take ownership of texture
typedef struct Pixelgrid {
  SDL_Texture *texture;
  int pixel_size;
  int height;
  int width;
} Pixelgrid;

Pixelgrid *
Pixelgrid_alloc(SDL_Texture *texture, Uint32 pixel_size)
{
  Pixelgrid *p = malloc(sizeof(Pixelgrid));
  p->texture = texture;
  p->pixel_size = pixel_size;
  SDL_QueryTexture(texture, NULL, NULL, &p->width, &p->height); // get width/height for convenience
  return p;
}

void
Pixelgrid_free(Pixelgrid *grid)
{
  free(grid);
}

void
Pixelgrid_put(Pixelgrid *grid, Uint32 x, Uint32 y, SDL_Color color)
{
  const SDL_Rect target = (SDL_Rect){
    .x = grid->pixel_size * x,
    .y = grid->pixel_size * y,
    .h = grid->pixel_size,
    .w = grid->pixel_size,
  };
  int *pixels, pitch;
  SDL_LockTexture(grid->texture, &target, (void **)&pixels, &pitch);
  for (int j = 0; j < grid->pixel_size; ++j) {
    for (int i = 0; i < grid->pixel_size; ++i) {
      Uint32 pixel_position = j * (pitch / sizeof(Uint32)) + i;
      pixels[pixel_position] = (color.b << 16) | (color.g << 8) | color.r | 0xFF000000;
    }
    SDL_UnlockTexture(grid->texture);
  }
}

void
Pixelgrid_clear(Pixelgrid *grid, SDL_Color color)
{
  const SDL_Rect target = (SDL_Rect){
    .x = 0,
    .y = 0,
    .h = grid->height,
    .w = grid->width,
  };
  int *pixels, pitch;
  SDL_LockTexture(grid->texture, &target, (void **)&pixels, &pitch);
  for (int j = 0; j < grid->height; ++j) {
    for (int i = 0; i < grid->width; ++i) {
      Uint32 pixel_position = j * (pitch / sizeof(Uint32)) + i;
      pixels[pixel_position] = (color.b << 16) | (color.g << 8) | color.r | 0xFF000000;
    }
  }
  SDL_UnlockTexture(grid->texture);
}

SDL_Texture *
Pixelgrid_texture(Pixelgrid *grid)
{
  return grid->texture;
}
