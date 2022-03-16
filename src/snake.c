#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

#include "bitset.h"
#include "ringbuf.h"
#include "pixelgrid.h"
#include "direction.h"
#include "config.h"

#define RGB(R, G, B) \
  (SDL_Color) { .r = R, .g = G, .b = B, .a = 255 }

static Uint32 score = 0;

static bool game_over = false;
static bool score_displayed = false;
static bool game_started = false;

static Uint32 food_x;
static Uint32 food_y;

void
place_food(Bitset *snake_pos)
{
  do {
    food_x = rand() % WIDTH;
    food_y = rand() % HEIGHT;
    // printf("Placed food at %d,%d\n", food_x, food_y);
  } while (Bitset_get(snake_pos, food_x + food_y * WIDTH));
}

void
render_text(SDL_Renderer *renderer, TTF_Font *font, const char str[], int x, int y, bool centered)
{
  SDL_Surface *surface = TTF_RenderText_Solid(font, str, RGB(0, 0, 0));
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_Rect target;

  if (centered) {
    target = (SDL_Rect){
      .w = surface->w,
      .h = surface->h,
      .x = (x - surface->w) / 2,
      .y = (y - surface->h) / 2,
    };
  } else {
    target = (SDL_Rect){
      .w = surface->w,
      .h = surface->h,
      .x = x,
      .y = y,
    };
  }

  SDL_RenderCopy(renderer, texture, NULL, &target);

  SDL_FreeSurface(surface);
  SDL_DestroyTexture(texture);
}

int
main(int argc, char **argv)
{
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);

  TTF_Init();
  TTF_Font *font = TTF_OpenFont("assets/PressStart2P-Regular.ttf", 24);
  if (font == NULL) {
    printf("Error: Font not found\n");
    return EXIT_FAILURE;
  }

  srand(clock());

  Uint32 now, next_time = SDL_GetTicks() + TICK_INTERVAL;

  SDL_Texture *grid_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);
  Pixelgrid *grid = Pixelgrid_alloc(grid_texture, PIXEL_SIZE);
  Pixelgrid_clear(grid, RGB(255, 255, 255));

  // contains the coordinates all of the squares contained by the snake, in order
  Ringbuf *snake_x = Ringbuf_alloc(NUM_PIXELS);
  Ringbuf *snake_y = Ringbuf_alloc(NUM_PIXELS);
  //
  // constant time access to whether a particular square is occupied by a snake
  Bitset *snake_pos = Bitset_alloc(NUM_PIXELS);

  Ringbuf_append(snake_x, 10);
  Ringbuf_append(snake_y, 10);
  Bitset_set(snake_pos, 10 + 10 * WIDTH);

  place_food(snake_pos);

  Direction curr_direction = Direction_Right;

  Uint64 frame = 0;

  Direction inputs[2];
  inputs[0] = Direction_None;
  inputs[1] = Direction_None;

  SDL_Event event;

  while (1) {
    frame++;

    // handle input
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        break;
      } else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.scancode) {
        case SDL_SCANCODE_Q:
          goto exit;
        case SDL_SCANCODE_LEFT:
        case SDL_SCANCODE_A:
          Direction_enqueue(inputs, Direction_Left);
          game_started = true;
          break;
        case SDL_SCANCODE_RIGHT:
        case SDL_SCANCODE_D:
          Direction_enqueue(inputs, Direction_Right);
          game_started = true;
          break;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
          Direction_enqueue(inputs, Direction_Down);
          game_started = true;
          break;
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_W:
          Direction_enqueue(inputs, Direction_Up);
          game_started = true;
          break;
        default:
          break;
        }
      }
    }

    // only move snake forward every SNAKE_SPEED frames
    if (frame % SNAKE_SPEED != 0) {
      goto end_frame;
    }

    // loop and display final score if haven't yet
    if (game_over) {
      if (score_displayed)
        continue;

      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, Pixelgrid_texture(grid), NULL, NULL);
      if (score == WIDTH * HEIGHT) {
        // near impossible btw
        render_text(renderer, font, "You won!!", WINDOW_WIDTH, WINDOW_HEIGHT, true);
      } else {
        char msg[25] = "";
        sprintf(msg, "Final Score: %d", score);
        render_text(renderer, font, msg, WINDOW_WIDTH, WINDOW_HEIGHT, true);
      }
      score_displayed = true;
      SDL_RenderPresent(renderer);

      continue;
    }

    if (!game_started) {
      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, Pixelgrid_texture(grid), NULL, NULL);
      render_text(renderer, font, "WASD or Arrow Keys to start", WINDOW_WIDTH, WINDOW_HEIGHT, true);
      SDL_RenderPresent(renderer);
      continue;
    }

    // handle queued input
    Direction_dequeue(inputs, &curr_direction);
    int next_x, next_y;
    Direction_nextpos(&next_x, &next_y, Ringbuf_head(snake_x), Ringbuf_head(snake_y), WIDTH, HEIGHT, curr_direction);

    // check for hit wall
    if (next_x == -1) {
      printf("You lose - hit wall\n");
      game_over = true;
      continue;
    }

    // check for hit self
    if (Bitset_get(snake_pos, next_x + next_y * WIDTH)) {
      printf("You lose, hit self at %d,%d\n", next_x, next_y);
      game_over = true;
      continue;
    }

    // add next snake segment
    Ringbuf_append(snake_x, next_x);
    Ringbuf_append(snake_y, next_y);
    Bitset_set(snake_pos, next_x + next_y * WIDTH);
    Pixelgrid_put(grid, next_x, next_y, RGB(0, 190, 0));

    if (Ringbuf_head(snake_x) == food_x && Ringbuf_head(snake_y) == food_y) {
      // just hit a food square
      score++;
      place_food(snake_pos);
    } else {
      // remove tailing section
      int tail_x = Ringbuf_tail(snake_x);
      int tail_y = Ringbuf_tail(snake_y);
      Bitset_clear(snake_pos, tail_x + tail_y * WIDTH);
      Pixelgrid_put(grid, tail_x, tail_y, RGB(255, 255, 255));
      Ringbuf_pop(snake_x);
      Ringbuf_pop(snake_y);
    }

    // draw food
    Pixelgrid_put(grid, food_x, food_y, RGB(255, 0, 0));

    // render grid
    SDL_RenderCopy(renderer, Pixelgrid_texture(grid), NULL, NULL);

    // render score in top left
    char buf[10] = "";
    sprintf(buf, "%d", score);
    render_text(renderer, font, buf, 10, 10, false);

    // apply render changes
    SDL_RenderPresent(renderer);

    // for a constant framerate
  end_frame:
    now = SDL_GetTicks();
    if (next_time > now) {
      SDL_Delay(next_time - now);
    }
    next_time += TICK_INTERVAL;
  }

exit:
  Ringbuf_free(snake_x);
  Ringbuf_free(snake_y);
  Bitset_free(snake_pos);
  Pixelgrid_free(grid);

  TTF_CloseFont(font);
  SDL_DestroyTexture(grid_texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return EXIT_SUCCESS;
}
