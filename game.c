#include "game.h"
#include "definitions.h"
#include <string.h>
#include <assert.h>

typedef struct {
  Move *items;
  size_t count;
  size_t capacity;
} MovesDA;

typedef struct {
  char board[8][8];
  MovesDA moves;
} GameState;

GameState STATE = {0};

void initGameState() {
  char board[8][8] = {{'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
                      {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
                      {'#', '#', '#', '#', '#', '#', '#', '#'},
                      {'#', '#', '#', '#', '#', '#', '#', '#'},
                      {'#', '#', '#', '#', '#', '#', '#', '#'},
                      {'#', '#', '#', '#', '#', '#', '#', '#'},
                      {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
                      {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}};
  memcpy(STATE.board, board, 8 * 8);
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
}

char get_piece_at(Cell cell) {
  assert(cell.col >= 0 && cell.col <= 7);
  assert(cell.row >= 0 && cell.row <= 7);
  return STATE.board[cell.row][cell.col];
}

size_t get_moves_log(char moves_log[][6], size_t max_moves) {
  max_moves = max_moves < STATE.moves.count ? max_moves : STATE.moves.count;
  for (size_t i = 0; i < max_moves; i++) {
    Move move = STATE.moves.items[i];
    moves_log[i][0] = move.src.col + 'a';
    moves_log[i][1] = move.src.row + '1';
    moves_log[i][2] = ',';
    moves_log[i][3] = ' ';
    moves_log[i][4] = move.dst.col + 'a';
    moves_log[i][5] = move.dst.row + '1';
  }
  return max_moves;
}

size_t get_moves_count() {
  return STATE.moves.count;
}
