#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <raylib.h>
#include "clay.h"

typedef struct {
  struct {
    struct {
      Texture2D w_king;
      Texture2D w_queen;
      Texture2D w_rook;
      Texture2D w_bishop;
      Texture2D w_knight;
      Texture2D w_pawn;
      Texture2D b_king;
      Texture2D b_queen;
      Texture2D b_rook;
      Texture2D b_bishop;
      Texture2D b_knight;
      Texture2D b_pawn;
    } chess_pieces;
  } textures;
  struct {
      Clay_Color background;
      Clay_Color board_background;
      Clay_Color light_background;
      Clay_Color even_cell;
      Clay_Color odd_cell;
      Clay_Color highlighted_cell;
  } colors;
} UIData;

typedef struct {
  int row, col;
} Cell;

typedef struct {
  Cell src;
  Cell dst;
} Move;

typedef struct {
  //TODO: proper DA managment soon TBD
  Move *items;
  size_t count;
  size_t capacity;
} MovesDA;

typedef struct {
  char board[8][8];
  Cell selected;
  MovesDA moves;
} GameState;

extern UIData UI;
extern GameState STATE;

#endif // DEFINITIONS_H
