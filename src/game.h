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

void game_initGameState();
void game_set_board(char *board);
// Is it currently White's turn
bool game_is_whites_turn();
void game_set_whites_turn(bool turn);
bool game_is_white_up();
DataMove game_make_chess_move(Move move);
char game_get_piece_at(Cell cell);
DataMovesArr game_get_moves_log();
size_t game_get_moves_count();

int game_get_white_count();
int game_get_black_count();

// This will overwrite the board state (returned from `get_piece_at`) and set the board immutable.
// To return to the current board and continue playing, call `reset_board`.
void game_show_board_at(size_t move_index);
// Resets the board to the last made move.
void game_reset_board();
// Returns true if the board is not the actuall game but a log history snapshot
bool game_is_viewing_history();

// Returns a random cell for the requested.
Cell game_get_random_player_cell(bool white);

#endif // GAME_H
