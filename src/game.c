#include "game.h"
#include "definitions.h"
#include "protocol.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#ifdef TESTER_MODE
#include "rules.h"
#endif

typedef struct {
  DataMove *items;
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
    if (board[i]=='#') continue;
    if (is_piece_white(board[i])) white_count++;
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

DataMove make_chess_move(Move move) {
  assert(!is_viewing_history());
#ifndef UI_WORK
  int backend_code = get_move_code(move);
#else
  int backend_code = 0;
#endif

  DataMove data_move = {
      .move = move,
      .src_piece = STATE.board[move.src.row][move.src.col],
      .dst_piece = STATE.board[move.dst.row][move.dst.col],
#ifdef TESTER_MODE // tester mode needs to report issues
      .backend_code = backend_code,
      .tester_code = is_move_legal(move),
#else // normal operation mode fully depends on the backend
      .backend_code = backend_code,
      .tester_code = backend_code,
#endif
  };
  if (is_code_legal(backend_code)) {
    MovesDA *moves = &STATE.moves;
    assert(moves->capacity); // Capacity should be initialized elsewhere
    if (moves->count == moves->capacity) {
      moves->capacity *= 2;
      moves->items =
          realloc(moves->items, moves->capacity * sizeof(moves->items[0]));
    }
    moves->items[moves->count++] = data_move;
    STATE.showing_move = moves->count;
    // update board
    char piece = STATE.board[move.src.row][move.src.col];
    STATE.board[move.src.row][move.src.col] = '#';
    STATE.board[move.dst.row][move.dst.col] = piece;
    STATE.white_turn = !STATE.white_turn;
  }
  return data_move;
}

char get_piece_at(Cell cell) {
  assert(cell.col >= 0 && cell.col <= 7);
  assert(cell.row >= 0 && cell.row <= 7);
  return STATE.board[cell.row][cell.col];
}

DataMovesArr get_moves_log() {
  return (DataMovesArr){.items = STATE.moves.items, .count = STATE.moves.count};
}

size_t get_moves_count() {
  return STATE.moves.count;
}

void make_move_backward(DataMove move) {
  STATE.board[move.move.src.row][move.move.src.col] = move.src_piece;
  STATE.board[move.move.dst.row][move.move.dst.col] = move.dst_piece;
}

void make_move_forwards(DataMove move) {
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
  for (size_t i = 0; i < 8 * 8; i++) {
    char piece = ((char *)STATE.board)[i];
    if (piece != '#' && is_piece_white(piece))
      count++;
  }
  return count;
}

size_t get_black_count() {
  size_t count = 0;
  for (size_t i = 0; i < 8 * 8; i++) {
    char piece = ((char *)STATE.board)[i];
    if (piece != '#' && !is_piece_white(piece))
      count++;
  }
  return count;
}
