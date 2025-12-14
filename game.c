#include "game.h"
#include "communication.h"
#include "definitions.h"
#include <assert.h>
#include <string.h>

typedef struct {
  Move *items;
  size_t count;
  size_t capacity;
} MovesDA;

typedef struct {
  char board[8][8];
  bool white_turn; // Is it currently White's turn
  MovesDA moves;
} GameState;

GameState STATE = {0};

char move_repr_buffer[4];

// Returns a move represantation the client can understand. Every call
// overwrites the previous one.
char *move_repr(Move move) {
  move_repr_buffer[0] = move.src.col + 'a';
  move_repr_buffer[1] = 7 - move.src.row + '1';
  move_repr_buffer[2] = move.dst.col + 'a';
  move_repr_buffer[3] = 7 - move.dst.row + '1';
  return move_repr_buffer;
}

void set_board(char board[8][8]) { memcpy(STATE.board, board, 8 * 8); }

bool is_whites_turn() { return STATE.white_turn; }
void set_whites_turn(bool turn) { STATE.white_turn = turn; }

void initGameState() {
  // default board, good for testing
  // char board[8][8] = {{'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
  //                     {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
  //                     {'#', '#', '#', '#', '#', '#', '#', '#'},
  //                     {'#', '#', '#', '#', '#', '#', '#', '#'},
  //                     {'#', '#', '#', '#', '#', '#', '#', '#'},
  //                     {'#', '#', '#', '#', '#', '#', '#', '#'},
  //                     {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
  //                     {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}};
  // set_board(board);

  STATE.moves.capacity = 1;
  STATE.moves.items = malloc(sizeof(STATE.moves.items[0]));
}

void make_chess_move(Move move) {
  MovesDA *moves = &STATE.moves;
  assert(moves->capacity); // Capacity should be initialized elsewhere
  if (moves->count == moves->capacity) {
    moves->capacity *= 2;
    moves->items = realloc(moves->items, moves->capacity * sizeof(moves->items[0]));
  }
  moves->items[moves->count++] = move;
  // update board
  char piece = STATE.board[move.src.row][move.src.col];
  STATE.board[move.src.row][move.src.col] = '#';
  STATE.board[move.dst.row][move.dst.col] = piece;
  pipe_send_message(move_repr(move));
}

char get_piece_at(Cell cell) {
  assert(cell.col >= 0 && cell.col <= 7);
  assert(cell.row >= 0 && cell.row <= 7);
  return STATE.board[cell.row][cell.col];
}

size_t get_moves_log(char moves_log[][6], size_t max_moves) {
  max_moves = max_moves < STATE.moves.count ? max_moves : STATE.moves.count;
  for (size_t i = 0; i < max_moves; i++) {
    char* move_repr_str = move_repr(STATE.moves.items[i]);
    moves_log[i][0] = move_repr_str[0];
    moves_log[i][1] = move_repr_str[1];
    moves_log[i][2] = ',';
    moves_log[i][3] = ' ';
    moves_log[i][4] = move_repr_str[2];
    moves_log[i][5] = move_repr_str[3];
  }
  return max_moves;
}

size_t get_moves_count() {
  return STATE.moves.count;
}
