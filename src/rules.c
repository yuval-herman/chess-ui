#include "rules.h"
#include "definitions.h"
#include "game.h"
#include "protocol.h"
#include "raylib.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Holds the move types possible in chess, can be used as a bitset as well
typedef enum {
  MOVE_TYPE_ILLEGAL    = 0,
  MOVE_TYPE_VERTICAL   = 1,
  MOVE_TYPE_HORIZONTAL = 2,
  MOVE_TYPE_DIAGONAL   = 4,
  MOVE_TYPE_KNIGHT     = 8,
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
  debug_print("Checking piece: %c at (%d,%d)", piece, cur.col + 1,             \
              cur.row + 1);                                                    \
  if (piece != '#')                                                            \
    return false;

// Check if all the cell are empty from src to dst, excluding src and dst
// themselves
bool is_way_free(Move_Type move_type, Cell src, Cell dst) {
  Cell cur = src;
  switch (move_type) {
    case MOVE_TYPE_HORIZONTAL: {
      debug_print("Testing horizontal movement");
      cur.col = min(src.col, dst.col);
      int end_col = max(src.col, dst.col);

      for (cur.col++; cur.col < end_col; cur.col++) {
        check_piece
      }
      return true;
    }
    case MOVE_TYPE_VERTICAL: {
      debug_print("Testing vertical movement");
      cur.row = min(src.row, dst.row);
      int end_row = max(src.row, dst.row);

      for (cur.row++; cur.row < end_row; cur.row++) {
        check_piece
      }
      return true;
    }
    case MOVE_TYPE_DIAGONAL: {
      debug_print("Testing diagonal movement");
      int col_dir = src.col < dst.col ? 1 : -1;
      int row_dir = src.row < dst.row ? 1 : -1;

      for (cur.col += col_dir, cur.row += row_dir; cur.col != dst.col;
           cur.col += col_dir, cur.row += row_dir) {
        check_piece
      }
      return true;
    }
    case MOVE_TYPE_KNIGHT: return true;
    case MOVE_TYPE_ILLEGAL:
      debug_print("Tried to test if way free for illegal move");
      exit(1);
  }
  debug_print("Unknown movement sent to is_way_free!");
  return false;
}

bitset get_piece_move_types(char piece) {
  switch (piece) {
  case 'b':
  case 'B':
    return MOVE_TYPE_DIAGONAL;
  case 'k':
  case 'K':
    return MOVE_TYPE_HORIZONTAL | MOVE_TYPE_VERTICAL | MOVE_TYPE_DIAGONAL;
  case 'n':
  case 'N':
    return MOVE_TYPE_KNIGHT;
  case 'p':
  case 'P':
    return MOVE_TYPE_VERTICAL;
  case 'q':
  case 'Q':
    return MOVE_TYPE_HORIZONTAL | MOVE_TYPE_VERTICAL | MOVE_TYPE_DIAGONAL;
  case 'r':
  case 'R':
    return MOVE_TYPE_HORIZONTAL | MOVE_TYPE_VERTICAL;
  default:
    debug_print("ERROR: received unknown piece type in %s", __FUNCTION__);
    exit(1);
  }
}

Move_Type get_move_move_type(bitset move_type, Cell src, Cell dst) {
  if (move_type & MOVE_TYPE_HORIZONTAL && src.row == dst.row)
    return MOVE_TYPE_HORIZONTAL;

  if (move_type & MOVE_TYPE_VERTICAL && src.col == dst.col)
    return MOVE_TYPE_VERTICAL;

  if (move_type & MOVE_TYPE_DIAGONAL &&
      abs(src.col - dst.col) == abs(src.row - dst.row))
    return MOVE_TYPE_DIAGONAL;

  if (move_type & MOVE_TYPE_KNIGHT) {
    int col_diff = abs(src.col - dst.col);
    int row_diff = abs(src.row - dst.row);
    if ((col_diff == 1 && row_diff == 2) || (col_diff == 2 && row_diff == 1))
      return MOVE_TYPE_KNIGHT;
  }

  return MOVE_TYPE_ILLEGAL;
}

void print_move_types(bitset move_types) {
  if (move_types & MOVE_TYPE_ILLEGAL) fprintf(stderr, " MOVE_TYPE_ILLEGAL");
  if (move_types & MOVE_TYPE_VERTICAL) fprintf(stderr, " MOVE_TYPE_VERTICAL");
  if (move_types & MOVE_TYPE_HORIZONTAL) fprintf(stderr, " MOVE_TYPE_HORIZONTAL");
  if (move_types & MOVE_TYPE_DIAGONAL) fprintf(stderr, " MOVE_TYPE_DIAGONAL");
  if (move_types & MOVE_TYPE_KNIGHT) fprintf(stderr, " MOVE_TYPE_KNIGHT");
  if (move_types == 0) fprintf(stderr, " MOVE_TYPE_ILLEGAL");
  fprintf(stderr, "\n");
}

Move_Codes is_move_legal(Move move) {
  if (is_move_stationary(move))
    return MOVE_STATIONARY;
  if (move.piece_moved == '#' || is_whites_turn() != is_piece_white(move.piece_moved))
    return MOVE_INVALID_SOURCE;
  if (is_move_OOB(move))
    return MOVE_OUT_OF_BOUNDS;

  char dst_piece = get_piece_at(move.dst);
  if (dst_piece != '#' && is_whites_turn() == is_piece_white(dst_piece))
    return MOVE_FRIENDLY_FIRE;
  bitset piece_move_type = get_piece_move_types(move.piece_moved);
  debug_print("%c movement type are:", move.piece_moved);
  print_move_types(piece_move_type);
  Move_Type move_move_type = get_move_move_type(piece_move_type, move.src, move.dst);
  debug_print("actual move preformed is:");
  print_move_types(move_move_type);
  if (move_move_type == MOVE_TYPE_ILLEGAL || !is_way_free(move_move_type, move.src, move.dst))
    return MOVE_ILLEGAL_PATTERN;

  return MOVE_VALID;
}

Cell get_random_move_cell(Cell src) {
  Cell options[64];
  int opt_count = 0;
  char piece = get_piece_at(src);
  if (piece == '#')
    return (Cell){.row = -1, .col = -1};

  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 8; c++) {
      Cell dst = {.row = r, .col = c};
      Move m = {.src = src, .dst = dst, .piece_moved = piece};
      if (is_move_legal(m) == MOVE_VALID) {
        options[opt_count++] = dst;
      }
    }
  }

  if (opt_count == 0)
    return (Cell){.row = -1, .col = -1};

  int idx = GetRandomValue(0, opt_count - 1);
  return options[idx];
}
