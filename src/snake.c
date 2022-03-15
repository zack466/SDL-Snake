#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "bitset.h"
#include "ringbuf.h"
#include <time.h>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 800
#define PIXEL_SIZE 40
#define TICK_INTERVAL 10

#define HEIGHT WINDOW_HEIGHT / PIXEL_SIZE
#define WIDTH WINDOW_WIDTH / PIXEL_SIZE

#define NUM_PIXELS (HEIGHT * WIDTH)

enum Direction {
  Left,
  Right,
  Up,
  Down,
  None,
};

bool
opposite_direction(enum Direction a, enum Direction b)
{
  switch (a) {
  case Left:
    return b == Right;
  case Right:
    return b == Left;
  case Up:
    return b == Down;
  case Down:
    return b == Up;
  case None:
    return false;
  }
}

struct Snake {
  // contains the coordinates all of the squares contained by the snake, in order
  Ringbuf *buf_x;
  Ringbuf *buf_y;
};

#define RAINBOW_LEN 7

Uint8 Rainbow[RAINBOW_LEN][3] = {
  [0] = { 255, 0, 0 },        // red
  [1] = { 255, 165, 0 },      // orange
  [2] = { 220, 220, 0 },      // yellow
  [3] = { 0, 0x80, 0 },       // green
  [4] = { 0, 0, 0xff },       // blue
  [5] = { 0x4b, 0, 0x82 },    // indigo
  [6] = { 0xee, 0x82, 0xee }, // violet
};

// draws to the xth and yth pixels in pixel-space
void
RenderDrawPixel(SDL_Renderer *renderer, int pixel_size, int x, int y)
{
  SDL_RenderFillRect(renderer, &(SDL_Rect){ .x = x * pixel_size, .y = y * pixel_size, .w = pixel_size, .h = pixel_size });
}

// quantizes true_x and true_y to the pixel covering that space
void
RenderDrawPixelTrue(SDL_Renderer *renderer, int pixel_size, int true_x, int true_y)
{
  RenderDrawPixel(renderer, pixel_size, true_x / pixel_size, true_y / pixel_size);
}

void
TextureDrawPixel(SDL_Texture *texture, int pixel_size, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
  SDL_Rect target = (SDL_Rect){
    .x = pixel_size * x,
    .y = pixel_size * y,
    .h = pixel_size,
    .w = pixel_size,
  };
  int *pixels, pitch;
  SDL_LockTexture(texture, &target, (void **)&pixels, &pitch);
  // using ABGR8888 format
  for (int j = 0; j < pixel_size; ++j) {
    for (int i = 0; i < pixel_size; ++i) {
      Uint32 pixel_position = j * (pitch / sizeof(unsigned int)) + i;
      pixels[pixel_position] = (b << 16) | (g << 8) | r | 0xFF000000;
    }
  }
  SDL_UnlockTexture(texture);
}

void
TextureClear(SDL_Texture *texture, int w, int h, Uint8 r, Uint8 g, Uint8 b)
{
  SDL_Rect target = (SDL_Rect){
    .x = 0,
    .y = 0,
    .h = h,
    .w = w,
  };
  int *pixels, pitch;
  SDL_LockTexture(texture, &target, (void **)&pixels, &pitch);
  // using ABGR8888 format
  for (int i = 0; i < w * h; ++i) {
    pixels[i] = (b << 16) | (g << 8) | r | 0xFF000000;
  }
  SDL_UnlockTexture(texture);
}

void
NextPos(int res[static 2], int x, int y, enum Direction direction)
{
  switch (direction) {
  case Left:
    if (x == 0) {
      res[0] = -1;
    } else {
      res[0] = x - 1;
      res[1] = y;
    }
    break;
  case Right:
    if (x == WIDTH - 1) {
      res[0] = -1;
    } else {
      res[0] = x + 1;
      res[1] = y;
    }
    break;
  case Up:
    if (y == 0) {
      res[0] = -1;
    } else {
      res[0] = x;
      res[1] = y - 1;
    }
    break;
  case Down:
    if (y == HEIGHT - 1) {
      res[0] = -1;
    } else {
      res[0] = x;
      res[1] = y + 1;
    }
    break;
  case None:
    break;
  }
}

static Uint32 next_time;

Uint32
time_left(void)
{
  Uint32 now;

  now = SDL_GetTicks();
  if (next_time <= now) {
    return 0;
  } else
    return next_time - now;
}

int
test()
{
  return EXIT_SUCCESS;
}

static SDL_Event event;
static SDL_Renderer *renderer;
static SDL_Window *window;
static SDL_Texture *screen;

static int color = 0;
static int color_offset = 0;

static Uint32 score = 0;

static bool game_over = false;
static bool score_displayed = false;

static Uint32 food_x;
static Uint32 food_y;

static Ringbuf *snake_x;
static Ringbuf *snake_y;
static Bitset *snake_pos;

void
place_food()
{
  do {
    food_x = rand() % WIDTH;
    food_y = rand() % HEIGHT;
    // printf("Placed food at %d,%d\n", food_x, food_y);
  } while (Bitset_get(snake_pos, food_x + food_y * WIDTH));
}

int
draw_pixel(Uint32 x, Uint32 y)
{
  TextureDrawPixel(screen, PIXEL_SIZE, x, y, Rainbow[color][0], Rainbow[color][1], Rainbow[color][2]);
  color = (color + 1) % RAINBOW_LEN;
  // SDL_SetRenderDrawColor(renderer, 0, 190, 0, 255);
  // RenderDrawPixel(renderer, PIXEL_SIZE, x, y);
  // TextureDrawPixel(screen, PIXEL_SIZE, x, y, 0, 190, 0);
  return 0;
}

int
main(int argc, char **argv)
{
  // set up SDL and open a window
  if (test() == EXIT_FAILURE)
    return EXIT_FAILURE;

  TTF_Init();
  SDL_Init(SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);

  screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);
  TextureClear(screen, WINDOW_WIDTH, WINDOW_HEIGHT, 255, 255, 255);

  TTF_Font *font = TTF_OpenFont("assets/PressStart2P-Regular.ttf", 24);
  if (font == NULL) {
    printf("Error: Font not found\n");
    return EXIT_FAILURE;
  }

  next_time = SDL_GetTicks() + TICK_INTERVAL;

  snake_x = Ringbuf_alloc(NUM_PIXELS);
  snake_y = Ringbuf_alloc(NUM_PIXELS);
  snake_pos = Bitset_alloc(NUM_PIXELS);
  // Bitset_set(snake_pos, 10 + 10 * WIDTH);
  Ringbuf_append(snake_x, 10);
  Ringbuf_append(snake_y, 10);

  place_food();

  enum Direction curr_direction = Right;

  Uint64 frame = 0;

  enum Direction inputs[2];
  inputs[0] = None;
  inputs[1] = None;

  while (1) {
    clock_t t1 = clock();
    frame++;

    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        break;
      } else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.scancode) {
        case SDL_SCANCODE_Q:
          goto exit;
          break;
        case SDL_SCANCODE_LEFT:
          // printf("left\n");
          if (inputs[0] == None) {
            inputs[0] = Left;
          } else if (inputs[1] == None) {
            inputs[1] = Left;
          }
          break;
        case SDL_SCANCODE_RIGHT:
          // printf("right\n");
          if (inputs[0] == None) {
            inputs[0] = Right;
          } else if (inputs[1] == None) {
            inputs[1] = Right;
          }
          break;
        case SDL_SCANCODE_DOWN:
          // printf("down\n");
          if (inputs[0] == None) {
            inputs[0] = Down;
          } else if (inputs[1] == None) {
            inputs[1] = Down;
          }
          break;
        case SDL_SCANCODE_UP:
          // printf("up\n");
          if (inputs[0] == None) {
            inputs[0] = Up;
          } else if (inputs[1] == None) {
            inputs[1] = Up;
          }
          break;
        default:
          break;
        }
      }
    }

    if (game_over) {
      if (score_displayed)
        continue;

      char buf[20] = "";
      sprintf(buf, "Final Score: %d", score);
      SDL_Surface *surfaceMessage = TTF_RenderText_Solid(font, buf, (SDL_Color){ 0, 0, 0 });
      SDL_Texture *Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
      SDL_Rect Message_rect;
      Message_rect.w = surfaceMessage->w;
      Message_rect.h = surfaceMessage->h;
      Message_rect.x = (WINDOW_WIDTH - surfaceMessage->w) / 2;
      Message_rect.y = (WINDOW_HEIGHT - surfaceMessage->h) / 2;
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_RenderClear(renderer);

      SDL_RenderCopy(renderer, screen, NULL, NULL);
      SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
      SDL_RenderPresent(renderer);

      SDL_FreeSurface(surfaceMessage);
      SDL_DestroyTexture(Message);
      score_displayed = true;
    }

    if (frame % 8 != 0) {
      SDL_Delay(time_left());
      next_time += TICK_INTERVAL;
      continue;
    }

    // use queued input
    // printf("input: %d %d\n", inputs[0], inputs[1]);
    if (inputs[0] == None && inputs[1] == None) {
      // nothing in input queue
      // just keep curr_direction the same
    } else if (inputs[0] != None && inputs[1] == None) {
      // one input in queue
      if (!opposite_direction(curr_direction, inputs[0])) {
        curr_direction = inputs[0];
      }
      inputs[0] = None;
    } else if (inputs[0] != None && inputs[1] != None) {
      // two inputs in queue
      if (!opposite_direction(curr_direction, inputs[0])) {
        curr_direction = inputs[0];
      }
      inputs[0] = inputs[1];
      inputs[1] = None;
    }

    int next_pos[2] = { -1, -1 };
    NextPos(next_pos, Ringbuf_head(snake_x), Ringbuf_head(snake_y), curr_direction);

    if (next_pos[0] == -1) {
      printf("you lose - hit wall\n");
      game_over = true;
      continue;
    }

    if (Bitset_get(snake_pos, next_pos[0] + next_pos[1] * WIDTH)) {
      printf("you lose, hit self at %d,%d\n", next_pos[0], next_pos[1]);
      game_over = true;
      continue;
    }

    SDL_RenderClear(renderer);

    // add next snake segment
    Ringbuf_append(snake_x, next_pos[0]);
    Ringbuf_append(snake_y, next_pos[1]);
    // something must have overwritten the memory in between here and here2.2
    Bitset_set(snake_pos, next_pos[0] + next_pos[1] * WIDTH);

    TextureDrawPixel(screen, PIXEL_SIZE, next_pos[0], next_pos[1], 0, 190, 0);

    // TextureDrawPixel(screen, PIXEL_SIZE, next_pos[0], next_pos[1], Rainbow[color][0], Rainbow[color][1], Rainbow[color][2]);

    // Ringbuf_foreach2(snake_x, snake_y, draw_pixel);
    // color = color_offset;
    // color_offset = (color_offset + RAINBOW_LEN - 1) % RAINBOW_LEN;

    if (Ringbuf_head(snake_x) == food_x && Ringbuf_head(snake_y) == food_y) {
      score++;
      place_food();
    } else {
      // remove tailing section
      int tail_x = Ringbuf_tail(snake_x);
      int tail_y = Ringbuf_tail(snake_y);
      Bitset_clear(snake_pos, tail_x + tail_y * WIDTH);
      TextureDrawPixel(screen, PIXEL_SIZE, tail_x, tail_y, 255, 255, 255);
      Ringbuf_pop(snake_x);
      Ringbuf_pop(snake_y);
    }

    // draw food
    TextureDrawPixel(screen, PIXEL_SIZE, food_x, food_y, 255, 0, 0);

    SDL_RenderCopy(renderer, screen, NULL, NULL);

    char buf[10] = "";
    sprintf(buf, "%d", score);
    SDL_Surface *surfaceMessage = TTF_RenderText_Solid(font, buf, (SDL_Color){ 0, 0, 0 });
    SDL_Texture *Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    SDL_Rect Message_rect;
    Message_rect.w = surfaceMessage->w;
    Message_rect.h = surfaceMessage->h;
    Message_rect.x = 0;
    Message_rect.y = 0;

    SDL_RenderCopy(renderer, Message, NULL, &Message_rect);

    SDL_RenderPresent(renderer);

    SDL_Delay(time_left());
    next_time += TICK_INTERVAL;
  }

exit:
  Ringbuf_free(snake_x);
  Ringbuf_free(snake_y);
  Bitset_free(snake_pos);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return EXIT_SUCCESS;
}
