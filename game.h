#include "definitions.h"

void initGameState();
void set_board(char board[8][8]);
// Is it currently White's turn
bool is_whites_turn();
void set_whites_turn(bool turn);
void make_chess_move(Move move);
char get_piece_at(Cell cell);
// Returns an array of move represantations, each 6 characters long without null termination.
// Each call invalidates previous ones.
size_t get_moves_log(char moves_log[][6], size_t max_moves);
size_t get_moves_count();
