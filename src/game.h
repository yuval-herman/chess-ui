#ifndef GAME_H
#define GAME_H
#include "definitions.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct {
  Move move;
  char src_piece;
  char dst_piece;
  int backend_code;
  int tester_code;
} DataMove;

typedef struct {
  const DataMove * items;
  const size_t count;
} DataMovesArr;

void initGameState();
void set_board(char *board);
// Is it currently White's turn
bool is_whites_turn();
void set_whites_turn(bool turn);
bool is_white_up();
DataMove make_chess_move(Move move);
char get_piece_at(Cell cell);
DataMovesArr get_moves_log();
size_t get_moves_count();

size_t get_white_count();
size_t get_black_count();

// This will overwrite the board state (returned from `get_piece_at`) and set the board immutable.
// To return to the current board and continue playing, call `reset_board`.
void show_board_at(size_t move_index);
// Resets the board to the last made move.
void reset_board();
// Returns true if the board is not the actuall game but a log history snapshot
bool is_viewing_history();

#endif // GAME_H
