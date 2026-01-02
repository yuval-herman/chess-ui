#include "game.h"
#include "definitions.h"
#include "protocol.h"
#include "raylib.h"
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

bool game_is_viewing_history() {
  return STATE.showing_move != STATE.moves.count;
}

void game_set_board(char *board) {
  memcpy(STATE.board, board, 8 * 8);
  int white_count = 0;
  int black_count = 0;
  for (int i = 0; i<8*4; i++) {
    if (board[i]=='#') continue;
    if (protocol_is_piece_white(board[i])) white_count++;
    else black_count++;
  }
  STATE.white_up = white_count >= black_count;
}

bool game_is_whites_turn() { return STATE.white_turn; }
void game_set_whites_turn(bool turn) { STATE.white_turn = turn; }
bool game_is_white_up() { return STATE.white_up; }

void game_initGameState() {
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
  game_set_board((char*)board);
  game_set_whites_turn(true);
#endif

  STATE.moves.capacity = 1;
  STATE.moves.items = malloc(sizeof(STATE.moves.items[0]));
}

DataMove game_make_chess_move(Move move) {
  assert(!game_is_viewing_history());
#ifndef UI_WORK
  int backend_code = protocol_get_move_code(move);
#else
  int backend_code = 0;
#endif

  DataMove data_move = {
      .move = move,
      .src_piece = STATE.board[move.src.row][move.src.col],
      .dst_piece = STATE.board[move.dst.row][move.dst.col],
#ifdef TESTER_MODE // tester mode needs to report issues
  .backend_code = backend_code,
  .tester_code = rules_is_move_legal(move),
#else // normal operation mode fully depends on the backend
      .backend_code = backend_code,
      .tester_code = backend_code,
#endif
  };
  if (protocol_is_code_legal(backend_code)) {
    MovesDA *moves = &STATE.moves;
    assert(moves->capacity); // Capacity should be initialized elsewhere
    if (moves->count == moves->capacity) {
      moves->capacity *= 2;
      DataMove* new_arr = realloc(moves->items, moves->capacity * sizeof(moves->items[0]));
      assert(new_arr && "Memory allocation failed");
      moves->items = new_arr;
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

char game_get_piece_at(Cell cell) {
  assert(cell.col >= 0 && cell.col <= 7);
  assert(cell.row >= 0 && cell.row <= 7);
  return STATE.board[cell.row][cell.col];
}

DataMovesArr game_get_moves_log() {
  return (DataMovesArr){.items = STATE.moves.items, .count = STATE.moves.count};
}

size_t game_get_moves_count() {
  return STATE.moves.count;
}

static void game__make_move_backward(DataMove move) {
  STATE.board[move.move.src.row][move.move.src.col] = move.src_piece;
  STATE.board[move.move.dst.row][move.move.dst.col] = move.dst_piece;
}

static void game__make_move_forwards(DataMove move) {
  if (move.dst_piece == '#')
    STATE.board[move.move.src.row][move.move.src.col] = move.dst_piece;
  else
    STATE.board[move.move.src.row][move.move.src.col] = '#';
  STATE.board[move.move.dst.row][move.move.dst.col] = move.src_piece;
}

void game_show_board_at(size_t move_index) {
  assert(move_index < STATE.moves.count);
  game_reset_board();
  for (size_t i = STATE.moves.count; i > move_index; i--) {
    game__make_move_backward(STATE.moves.items[i - 1]);
    STATE.showing_move = i - 1;
  }
}

void game_reset_board() {
  for (; STATE.showing_move < STATE.moves.count; STATE.showing_move++) {
    game__make_move_forwards(STATE.moves.items[STATE.showing_move]);
  }
}

int game_get_white_count() {
  int count = 0;
  for (int i = 0; i < 8 * 8; i++) {
    char piece = ((char *)STATE.board)[i];
    if (piece != '#' && protocol_is_piece_white(piece))
      count++;
  }
  return count;
}

int game_get_black_count() {
  int count = 0;
  for (int i = 0; i < 8 * 8; i++) {
    char piece = ((char *)STATE.board)[i];
    if (piece != '#' && !protocol_is_piece_white(piece))
      count++;
  }
  return count;
}

Cell game_get_random_player_cell(bool white) {
  int piece_count = white ? game_get_white_count() : game_get_black_count();
  int selected_piece_index = GetRandomValue(1, piece_count);
  int count = 0;
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      char piece = STATE.board[row][col];
      if (piece != '#' && white == protocol_is_piece_white(piece)) count++;
      if (count == selected_piece_index) return (Cell){.col = col, .row = row};
    }
  }
  assert(false && "reached unreachable place");
}
