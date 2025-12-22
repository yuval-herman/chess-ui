#include "rules.h"
#include "definitions.h"
#include "game.h"
#include "protocol.h"
#include <stdlib.h>

// Holds the move types possible in chess, can be used as a bitset as well
typedef enum {
  MOVE_TYPE_VERTICAL   = 1,
  MOVE_TYPE_HORIZONTAL = 2,
  // Diagonal movement 1 - downard line
  MOVE_TYPE_DIAGONAL_1 = 4,
  // Diagonal movement 2 - upwards line
  MOVE_TYPE_DIAGONAL_2 = 8,
  MOVE_TYPE_KNIGHT     = 16,
} Move_Type;

bool is_move_OOB(Move move) {
  return 0 > move.src.col || move.src.col > 7 ||
         0 > move.src.row || move.src.row > 7 ||
         0 > move.dst.col || move.dst.col > 7 ||
         0 > move.dst.row || move.dst.row > 7;
}

bool is_move_stationary(Move move) {
  return move.src.col == move.dst.col && move.src.row == move.dst.row;
}

// Helper macro for is_way_free
#define check_piece                                                            \
  char piece = get_piece_at(cur);                                              \
  debug_print("Checking piece: %c", piece);                                    \
  if (piece != '#')                                                            \
    return false;


// Check if all the cell are empty from src to dst, excluding src and dst
// themselves
bool is_way_free(bitset move_type, Cell src, Cell dst) {
  Cell cur = src;
  if (move_type & MOVE_TYPE_HORIZONTAL) {
    debug_print("Testing horizontal movement");
    cur.col     = min(src.col,dst.col);
    int end_col = max(src.col,dst.col);

    for (cur.col++; cur.col < end_col; cur.col++) {
      check_piece
    }
    return true;
  }
  if (move_type & MOVE_TYPE_VERTICAL) {
    debug_print("Testing vertical movement");
    cur.row     = min(src.row,dst.row);
    int end_row = max(src.row,dst.row);

    for (cur.row++; cur.row < end_row; cur.row++) {
      check_piece
    }
    return true;
  }
  if (move_type & MOVE_TYPE_DIAGONAL_1) {
    debug_print("Testing diagonal 1 movement");
    cur = src.col < dst.col ? src : dst;
    int end_col = max(src.col, dst.col);

    for(cur.col++,cur.row++;cur.col<end_col;cur.col++,cur.row++) {
      check_piece
    }
    return true;
  }
  if (move_type & MOVE_TYPE_DIAGONAL_2) {
    debug_print("Testing diagonal 2 movement");
    cur = src.col > dst.col ? src : dst;
    int end_col = min(src.col, dst.col);

    for(cur.col--,cur.row++;cur.col>end_col;cur.col--,cur.row++) {
      check_piece
    }
    return true;
  }
  debug_print("Unknown movement sent to is_way_free!");
  return false;
}

bitset get_piece_move_types(char piece) {
  switch (piece) {
  case 'b':
  case 'B':
    return MOVE_TYPE_DIAGONAL_1 | MOVE_TYPE_DIAGONAL_2;
  case 'k':
  case 'K':
    return MOVE_TYPE_HORIZONTAL | MOVE_TYPE_VERTICAL | MOVE_TYPE_DIAGONAL_1 |
           MOVE_TYPE_DIAGONAL_2;
  case 'n':
  case 'N':
    return MOVE_TYPE_KNIGHT;
  case 'p':
  case 'P':
    return MOVE_TYPE_VERTICAL;
  case 'q':
  case 'Q':
    return MOVE_TYPE_HORIZONTAL | MOVE_TYPE_VERTICAL | MOVE_TYPE_DIAGONAL_1 |
           MOVE_TYPE_DIAGONAL_2;
  case 'r':
  case 'R':
    return MOVE_TYPE_HORIZONTAL | MOVE_TYPE_VERTICAL;
  default:
    debug_print("ERROR: received unknown piece type in %s", __FUNCTION__);
    exit(1);
  }
}

bool is_move_type_valid(bitset move_type, Cell src, Cell dst) {
  if (!(move_type & MOVE_TYPE_HORIZONTAL) && src.row == dst.row) {
    debug_print("Attmpted horizontal move with a piece incapable of it");
    return false;
  }
  if (!(move_type & MOVE_TYPE_VERTICAL) && src.col == dst.col) {
    debug_print("Attmpted vertical move with a piece incapable of it");
    return false;
  }
  if ((!(move_type & MOVE_TYPE_DIAGONAL_1) &&
       src.col - src.row == dst.col - dst.row) ||
      (!(move_type & MOVE_TYPE_DIAGONAL_2) &&
       7 - src.col - src.row == 7 - dst.col - dst.row)) {
    debug_print("Attmpted diagonal move with a piece incapable of it");
    return false;
  }

  return true;
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
  bitset move_type = get_piece_move_types(move.piece_moved);
  if (!is_move_type_valid(move_type, move.src, move.dst) ||
      !is_way_free(move_type, move.src, move.dst))
    return MOVE_ILLEGAL_PATTERN;

  return false;
}
