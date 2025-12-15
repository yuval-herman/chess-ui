#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "definitions.h"
#include <stdbool.h>

bool protocol_init();
void protocol_close();
const char* code2str(int code);
bool is_code_legal(int code);
int get_move_code(Move move);
// Returns a move represantation the client can understand. Every call
// overwrites the previous one.
char *move_repr(Move move);

#endif // PROTOCOL_H
