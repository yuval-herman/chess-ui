#ifndef GAME_H
#define GAME_H
#include "definitions.h"

#define MOVE_REPR_LENGTH 7

void initGameState();
void set_board(char *board);
// Is it currently White's turn
bool is_whites_turn();
void set_whites_turn(bool turn);
bool is_white_up();
int make_chess_move(Move move);
char get_piece_at(Cell cell);
// Returns an array of move represantations, each MOVE_REPR_LENGTH characters long without null termination.
// Each call invalidates previous ones.
size_t get_moves_log(char moves_log[][MOVE_REPR_LENGTH], size_t max_moves);
size_t get_moves_count();

// This will overwrite the board state (returned from `get_piece_at`) and set the board immutable.
// To return to the current board and continue playing, call `reset_board`.
void show_board_at(size_t move_index);
// Resets the board to the last made move.
void reset_board();
// Returns true if the board is not the actuall game but a log history snapshot
bool is_viewing_history();

#endif // GAME_H
