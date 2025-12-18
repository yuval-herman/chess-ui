#include "game.h"
#include "definitions.h"
#include "protocol.h"
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
  Move move;
  char src_piece;
  char dst_piece;
} PieceMove;

typedef struct {
  PieceMove *items;
  size_t count;
  size_t capacity;
} MovesDA;

typedef struct {
  char board[8][8];
  bool white_up; // Whether white is on the top of the board
  bool white_turn; // Whether it is currently White's turn
  MovesDA moves;
  size_t showing_move;
} GameState;

GameState STATE = {0};

bool is_viewing_history() {
  return STATE.showing_move != STATE.moves.count;
}

void set_board(char *board) {
  memcpy(STATE.board, board, 8 * 8);
  int white_count = 0;
  int black_count = 0;
  for (int i = 0; i<8*4; i++) {
    if(isupper(board[i])) white_count++;
    else black_count++;
  }
  STATE.white_up = white_count >= black_count;
}

bool is_whites_turn() { return STATE.white_turn; }
void set_whites_turn(bool turn) { STATE.white_turn = turn; }
bool is_white_up() { return STATE.white_up; }

void initGameState() {
#ifdef UI_WORK
  // default board, good for testing
  char board[8][8] = {{'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
                      {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
                      {'#', '#', '#', '#', '#', '#', '#', '#'},
                      {'#', '#', '#', '#', '#', '#', '#', '#'},
                      {'#', '#', '#', '#', '#', '#', '#', '#'},
                      {'#', '#', '#', '#', '#', '#', '#', '#'},
                      {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
                      {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}};
  set_board((char*)board);
  set_whites_turn(true);
#endif

  STATE.moves.capacity = 1;
  STATE.moves.items = malloc(sizeof(STATE.moves.items[0]));
}

int make_chess_move(Move move) {
  assert(!is_viewing_history());
#ifndef UI_WORK
  int backend_code = get_move_code(move);
  if (!is_code_legal(backend_code))
    return backend_code;
#endif
  MovesDA *moves = &STATE.moves;
  assert(moves->capacity); // Capacity should be initialized elsewhere
  if (moves->count == moves->capacity) {
    moves->capacity *= 2;
    moves->items = realloc(moves->items, moves->capacity * sizeof(moves->items[0]));
  }
  moves->items[moves->count++] = (PieceMove){
      .move = move,
      .src_piece = STATE.board[move.src.row][move.src.col],
      .dst_piece = STATE.board[move.dst.row][move.dst.col],
  };
  STATE.showing_move = moves->count;
  // update board
  char piece = STATE.board[move.src.row][move.src.col];
  STATE.board[move.src.row][move.src.col] = '#';
  STATE.board[move.dst.row][move.dst.col] = piece;
  STATE.white_turn = !STATE.white_turn;
#ifndef UI_WORK
  return backend_code;
#else
  return 0;
#endif
}

char get_piece_at(Cell cell) {
  assert(cell.col >= 0 && cell.col <= 7);
  assert(cell.row >= 0 && cell.row <= 7);
  return STATE.board[cell.row][cell.col];
}

size_t get_moves_log(char moves_log[][MOVE_REPR_LENGTH], size_t max_moves) {
  max_moves = max_moves < STATE.moves.count ? max_moves : STATE.moves.count;
  for (size_t i = 0; i < max_moves; i++) {
    char* move_repr_str = move_repr(STATE.moves.items[i].move);
    moves_log[i][0] = STATE.moves.items[i].src_piece;
    moves_log[i][1] = move_repr_str[0];
    moves_log[i][2] = move_repr_str[1];
    moves_log[i][3] = ',';
    moves_log[i][4] = ' ';
    moves_log[i][5] = move_repr_str[2];
    moves_log[i][6] = move_repr_str[3];
  }
  return max_moves;
}

size_t get_moves_count() {
  return STATE.moves.count;
}

void make_move_backward(PieceMove move) {
  STATE.board[move.move.src.row][move.move.src.col] = move.src_piece;
  STATE.board[move.move.dst.row][move.move.dst.col] = move.dst_piece;
}

void make_move_forwards(PieceMove move) {
  if (move.dst_piece == '#')
    STATE.board[move.move.src.row][move.move.src.col] = move.dst_piece;
  else
    STATE.board[move.move.src.row][move.move.src.col] = '#';
  STATE.board[move.move.dst.row][move.move.dst.col] = move.src_piece;
}

void show_board_at(size_t move_index) {
  assert(move_index < STATE.moves.count);
  reset_board();
  for (size_t i = STATE.moves.count; i > move_index; i--) {
    make_move_backward(STATE.moves.items[i - 1]);
    STATE.showing_move = i - 1;
  }
}

void reset_board() {
  for (; STATE.showing_move < STATE.moves.count; STATE.showing_move++) {
    make_move_forwards(STATE.moves.items[STATE.showing_move]);
  }
}

size_t get_white_count() {
  size_t count = 0;
  for (size_t i = 0; i < 8 * 8; i++)
    if (islower(((char *)STATE.board)[i]))
      count++;
  return count;
}

size_t get_black_count() {
  size_t count = 0;
  for (size_t i = 0; i < 8 * 8; i++)
    if (isupper(((char *)STATE.board)[i]))
      count++;
  return count;
}
