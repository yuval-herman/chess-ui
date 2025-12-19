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

// Check if all the cell are empty from src to dst, excluding src and dst
// themselves
bool is_way_free(Cell src, Cell dst) {
  Cell cur = src;
  // Horizontal movement
  if (src.row == dst.row) {
    debug_print("Attempting horizontal movement");
    cur.col     = min(src.col,dst.col);
    int end_col = max(src.col,dst.col);

    for (cur.col++; cur.col < end_col; cur.col++) {
      char piece = get_piece_at(cur);
      debug_print("Checking piece: %c", piece);
      if (piece != '#')
        return false;
    }
    return true;
  }
  // Vectical movement
  else if (src.col == dst.col) {
    debug_print("Attempting vertical movement");
    cur.row     = min(src.row,dst.row);
    int end_row = max(src.row,dst.row);

    for (cur.row++; cur.row < end_row; cur.row++) {
      char piece = get_piece_at(cur);
      debug_print("Checking piece: %c", piece);
      if (piece != '#')
        return false;
    }
    return true;
  }
  // Diagonal movement 1 - downard line
  else if (src.col - src.row == dst.col - dst.row) {
    debug_print("Attempting diagonal 1 movement");
    cur = src.col < dst.col ? src : dst;
    int end_col = max(src.col, dst.col);

    for(cur.col++,cur.row++;cur.col<end_col;cur.col++,cur.row++) {
      char piece = get_piece_at(cur);
      debug_print("Checking piece: %c", piece);
      if (piece != '#')
        return false;
    }
    return true;
  }
  // Diagonal movement 2 - upwards line
  else if (7 - src.col - src.row == 7 - dst.col - dst.row) {
    debug_print("Attempting diagonal 2 movement");
    cur = src.col > dst.col ? src : dst;
    int end_col = min(src.col, dst.col);

    for(cur.col--,cur.row++;cur.col>end_col;cur.col--,cur.row++) {
      char piece = get_piece_at(cur);
      debug_print("Checking piece: %c", piece);
      if (piece != '#')
        return false;
    }
    return true;
  }
  debug_print("Unknown movement sent to is_way_free!");
  return false;
}

Move_Codes is_move_legal(Move move) {
  if (is_move_stationary(move))
    return MOVE_STATIONARY;
  if (move.piece_moved == '#' || is_whites_turn() != is_piece_white(move.piece_moved))
    return MOVE_INVALID_SOURCE;

  char dst_piece = get_piece_at(move.dst);
  if (dst_piece != '#' && is_whites_turn() == is_piece_white(dst_piece))
    return MOVE_FRIENDLY_FIRE;
  if (is_move_OOB(move))
    return MOVE_OUT_OF_BOUNDS;
  if(!is_way_free(move.src, move.dst))
    return MOVE_ILLEGAL_PATTERN;

  return false;
}
