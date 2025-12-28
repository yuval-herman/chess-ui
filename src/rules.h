#ifndef RULES_H
#define RULES_H
#include "definitions.h"
#include <stdbool.h>

Move_Codes is_move_legal(Move move);
// This picks a random move type and a random destination for this move type
Cell get_random_move_cell(Cell src);


#endif // RULES_H
