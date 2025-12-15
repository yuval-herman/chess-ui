#include "protocol.h"
#include "communication.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char move_repr_buffer[4];

bool protocol_init() {
  if (!pipe_init()) {
    printf("Failed creating named pipe, exiting...");
    exit(1);
  }

  char *pipe_msg = pipe_get_message();
  if (strlen(pipe_msg) != 65) {
    printf("Got unexpected message: %s\n", pipe_msg);
    return false;
  }
  set_board(pipe_msg);
  set_whites_turn(pipe_msg[64] == '0');
  return true;
}

void protocol_close() {
  if (!pipe_is_connected())
    return;
  pipe_send_message("quit");
  pipe_close();
}

const char* code2str(int code) {
  switch (code) {
  case 0: return "Valid move";
  case 1: return "Chess";
  case 2: return "Source cell is empty or the piece is not yours";
  case 3: return "Destination cell is occupied by your own piece";
  case 4: return "The move results in a chess";
  case 5: return "Invalid source or dest position";
  case 6: return "Piece cannot move that way";
  case 7: return "Source and destination cells are the same";
  case 8: return "Checkmate";
  default: return "Error code invalid";
  }
}

bool is_code_legal(int code) {
  return code == 0 || code == 1 || code == 8;
}

int get_move_code(Move move) {
  pipe_send_message(move_repr(move));
  char* backend_msg = pipe_get_message();
  return backend_msg[0] - '0';
}

char *move_repr(Move move) {
  move_repr_buffer[0] = move.src.col + 'a';
  move_repr_buffer[1] = 7 - move.src.row + '1';
  move_repr_buffer[2] = move.dst.col + 'a';
  move_repr_buffer[3] = 7 - move.dst.row + '1';
  return move_repr_buffer;
}
