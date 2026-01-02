#include "protocol.h"
#include "communication.h"
#include "definitions.h"
#include "game.h"
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

char move_repr_buffer[5];

bool protocol_init() {
  if (!pipe_init()) {
    fprintf(stderr, "Failed creating named pipe, exiting...");
    return false;
  }
  return true;
}

bool protocol_has_started() {
  if(!pipe_is_connected() || !pipe_has_new_message()) return false;
  char *pipe_msg = pipe_get_message();
  if (strlen(pipe_msg) != 65) {
    fprintf(stderr, "Got unexpected message: %s\n", pipe_msg);
    return false;
  }
  game_set_board(pipe_msg);
  game_set_whites_turn(pipe_msg[64] == '0');
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
  case MOVE_OUT_OF_BOUNDS: return "Tried moving out of bounds";
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
  assert(move.src.col + 'a' <= CHAR_MAX);
  assert(7 - move.src.row + '1' <= CHAR_MAX);
  assert(move.dst.col + 'a' <= CHAR_MAX);
  assert(7 - move.dst.row + '1' <= CHAR_MAX);

  move_repr_buffer[0] = (char)(move.src.col + 'a');
  move_repr_buffer[1] = (char)(7 - move.src.row + '1');
  move_repr_buffer[2] = (char)(move.dst.col + 'a');
  move_repr_buffer[3] = (char)(7 - move.dst.row + '1');
  move_repr_buffer[4] = '\0';
  return move_repr_buffer;
}

bool is_piece_white(char piece) {
  if(piece=='#') debug_print("Trying to check if an empty piece is white!");
  return isupper(piece);
}

