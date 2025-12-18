#include "rules.h"
#include "definitions.h"
#include "game.h"
#include "protocol.h"
#include <stdio.h>

bool is_move_OOB(Move move) {
  return 0 > move.src.col || move.src.col > 7 ||
         0 > move.src.row || move.src.row > 7 ||
         0 > move.dst.col || move.dst.col > 7 ||
         0 > move.dst.row || move.dst.row > 7;
}

bool is_move_stationary(Move move) {
  return move.src.col == move.dst.col && move.src.row == move.dst.row;
}

Move_Codes is_move_legal(Move move) {
  if (is_move_stationary(move))
    return MOVE_STATIONARY;
  if (is_whites_turn() != is_piece_white(move.piece_moved))
    return MOVE_INVALID_SOURCE;

  char dst_piece = get_piece_at(move.dst);
  if (dst_piece != '#' && is_whites_turn() == is_piece_white(dst_piece))
    return MOVE_FRIENDLY_FIRE;
  if (is_move_OOB(move))
    return MOVE_OUT_OF_BOUNDS;

  return false;
}
