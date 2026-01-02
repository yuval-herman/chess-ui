#ifndef RULES_H
#define RULES_H
#include "definitions.h"
#include <stdbool.h>

Move_Codes rules_is_move_legal(Move move);
// This picks a random legal cell for the src cell to move into. If no such cells exists, an illegal cell (-1,-1) is returned.
Cell rules_get_random_move_cell(Cell src);


#endif // RULES_H
