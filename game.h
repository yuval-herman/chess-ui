#include "definitions.h"

void initGameState();
void make_chess_move(Move move);
char get_piece_at(Cell cell);
// Returns an array of move represantations, each 6 characters long without null termination.
// Each call invalidates previous ones.
size_t get_moves_log(char moves_log[][6], size_t max_moves);
size_t get_moves_count();
