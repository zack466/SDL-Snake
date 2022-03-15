#include <stdbool.h>

typedef enum Direction {
  Direction_None = 0,
  Direction_Left,
  Direction_Right,
  Direction_Up,
  Direction_Down,
} Direction;

bool Direction_opposite(Direction a, Direction b);

void Direction_nextpos(int *next_x, int *next_y, int x, int y, int max_x, int max_y, Direction direction);

void Direction_enqueue(Direction inputs[static 2], Direction dir);

void Direction_dequeue(Direction inputs[static 2], Direction *direction);
