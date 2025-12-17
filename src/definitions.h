#ifndef DEFINITIONS_H
#define DEFINITIONS_H

typedef struct {
  int row, col;
} Cell;

typedef struct {
  Cell src;
  Cell dst;
  char piece_moved;
} Move;

#endif // DEFINITIONS_H
