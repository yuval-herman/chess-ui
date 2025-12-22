#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <stdio.h>

typedef unsigned int bitset;

typedef enum {
  MOVE_VALID = 0,
  MOVE_CHECK,
  MOVE_INVALID_SOURCE,
  MOVE_FRIENDLY_FIRE,
  MOVE_LEAVES_KING_EXPOSED,
  MOVE_OUT_OF_BOUNDS,
  MOVE_ILLEGAL_PATTERN,
  MOVE_STATIONARY,
  MOVE_CHECKMATE,
  MOVE_ERROR_UNKNOWN,
} Move_Codes;

typedef struct {
  int row, col;
} Cell;

typedef struct {
  Cell src;
  Cell dst;
  char piece_moved;
} Move;

#define debug_print(fmt, ...) fprintf(stderr, fmt " %s:%d\n", ##__VA_ARGS__, __FILE__, __LINE__)
#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

#endif // DEFINITIONS_H
