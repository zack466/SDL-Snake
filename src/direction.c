#include <stdbool.h>

typedef enum Direction {
  None = 0,
  Left,
  Right,
  Up,
  Down,
} Direction;

bool
Direction_opposite(Direction a, Direction b)
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

// sets next_x to -1 if next position will be out of range (less than 0 or geq max_x/max_y)
void
Direction_nextpos(int *next_x, int *next_y, int x, int y, int max_x, int max_y, Direction direction)
{
  switch (direction) {
  case Left:
    if (x == 0) {
      *next_x = -1;
    } else {
      *next_x = x - 1;
      *next_y = y;
    }
    break;
  case Right:
    if (x == max_x - 1) {
      *next_x = -1;
    } else {
      *next_x = x + 1;
      *next_y = y;
    }
    break;
  case Up:
    if (y == 0) {
      *next_x = -1;
    } else {
      *next_x = x;
      *next_y = y - 1;
    }
    break;
  case Down:
    if (y == max_y - 1) {
      *next_x = -1;
    } else {
      *next_x = x;
      *next_y = y + 1;
    }
    break;
  case None:
    break;
  }
}

// enqueues a direction to a queue of size 2
// I'm using this so two consecutive keystrokes get properly handled, without
// allowing the player to kill themselves (if they turn in the opposite direction)
void
Direction_enqueue(Direction inputs[static 2], Direction dir)
{
  if (inputs[0] == None) {
    inputs[0] = dir;
  } else if (inputs[1] == None) {
    inputs[1] = dir;
  }
}

// dequeues a direction to a queue of size 2, only if new direction is not the opposite of the current direction
void
Direction_dequeue(Direction inputs[static 2], Direction *direction)
{
  if (inputs[0] == None && inputs[1] == None) {
    // nothing in input queue
    // keep direction the same
  } else if (inputs[0] != None && inputs[1] == None) {
    // one input in queue
    if (!Direction_opposite(*direction, inputs[0])) {
      *direction = inputs[0];
    }
    inputs[0] = None;
  } else if (inputs[0] != None && inputs[1] != None) {
    // two inputs in queue
    if (!Direction_opposite(*direction, inputs[0])) {
      *direction = inputs[0];
    }
    inputs[0] = inputs[1];
    inputs[1] = None;
  }
}
