#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "definitions.h"
#include <stdbool.h>

bool protocol_init();
bool protocol_has_started();
void protocol_close();
const char* code2str(int code);
bool is_code_legal(int code);
int get_move_code(Move move);
// Returns a move represantation the client can understand. Every call
// overwrites the previous one.
char *move_repr(Move move);
bool is_piece_white(char piece);

#endif // PROTOCOL_H
