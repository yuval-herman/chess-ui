#include "protocol.h"
#include "communication.h"
#include "definitions.h"
#include "game.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

char move_repr_buffer[4];

bool protocol_init() {
  if (!pipe_init()) {
    printf("Failed creating named pipe, exiting...");
    return false;
  }
  return true;
}

bool protocol_has_started() {
  if(!pipe_is_connected() || !pipe_has_new_message()) return false;
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
  case MOVE_VALID : return "Valid move";
  case MOVE_CHECK: return "Check";
  case MOVE_INVALID_SOURCE: return "Source cell empty or wrong color";
  case MOVE_FRIENDLY_FIRE: return "Destination cell is occupied by your own piece";
  case MOVE_LEAVES_KING_EXPOSED: return "The move results in a check against yourself";
  case MOVE_OUT_OF_BOUNDS: return "Invalid source or dest position";
  case MOVE_ILLEGAL_PATTERN: return "Piece cannot move that way";
  case MOVE_STATIONARY: return "Source and destination cells are the same";
  case MOVE_CHECKMATE: return "Checkmate";
  default: return "Error code invalid";
  }
}

bool is_code_legal(int code) {
  return code == MOVE_VALID || code == MOVE_CHECK || code == MOVE_CHECKMATE;
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

bool is_piece_white(char piece) {
  return isupper(piece);
}

