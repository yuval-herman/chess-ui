#ifndef DEFINITIONS_H
#define DEFINITIONS_H

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

#endif // DEFINITIONS_H
