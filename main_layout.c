#include "clay.h"
#include "definitions.h"
#include <assert.h>
#include <raylib.h>
#include <string.h>
#include <time.h>

// Average chess games have around 30-40 moves.
// The moves log shows moves in this format:
// "e2, b8" 6 chars
// I allow up to 200 moves per game, if you want longer games, adjust this accordingly.
char move_log_buffer[200][6];

// Returns a chess piece texture for the received char.
// Returns white variant for white==true, black otherwise.
Texture2D* char2tex(char c) {
  switch (c) {
    case 'b': return &UI.textures.chess_pieces.b_bishop;
    case 'B': return &UI.textures.chess_pieces.w_bishop;
    case 'k': return &UI.textures.chess_pieces.b_king;
    case 'K': return &UI.textures.chess_pieces.w_king;
    case 'n': return &UI.textures.chess_pieces.b_knight;
    case 'N': return &UI.textures.chess_pieces.w_knight;
    case 'p': return &UI.textures.chess_pieces.b_pawn;
    case 'P': return &UI.textures.chess_pieces.w_pawn;
    case 'q': return &UI.textures.chess_pieces.b_queen;
    case 'Q': return &UI.textures.chess_pieces.w_queen;
    case 'r': return &UI.textures.chess_pieces.b_rook;
    case 'R': return &UI.textures.chess_pieces.w_rook;
    default: return NULL;
  }
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

void handle_board_cell_hover(Clay_ElementId element_id,
                             Clay_PointerData pointer_data, void *user_data) {
  (void)user_data;
  if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    int col = element_id.offset % 8;
    int row = (element_id.offset - col) / 8;
    if (STATE.selected.col >= 0) {
      make_chess_move((Move){
          .src = {STATE.selected.row, STATE.selected.col},
          .dst = {row, col},
      });


      STATE.selected.col = -1;
      STATE.selected.row = -1;
    } else {
      STATE.selected.col = col;
      STATE.selected.row = row;
    }
  }
}

void board_layout() {
  CLAY(CLAY_ID("BoardContainer"), {
       .layout = {
          .layoutDirection = CLAY_TOP_TO_BOTTOM,
          .sizing = {
            .width = CLAY_SIZING_GROW(),
            .height = CLAY_SIZING_GROW(),
          },
          .childGap = 8,
          .padding = CLAY_PADDING_ALL(8),
      },
      .backgroundColor = UI.colors.board_background,
      .aspectRatio = {.aspectRatio = 1}
    }) {
    for (int row = 0; row < 8; row++) {
      CLAY_AUTO_ID({
            .layout = {
              .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_GROW(),
              },
              .childGap = 8,
            }
        }) {
        for (int col = 0; col < 8; col++) {
          Clay_Color cell_color = (row + col) % 2 ? UI.colors.odd_cell : UI.colors.even_cell;
          bool cell_selected = row == STATE.selected.row && col == STATE.selected.col;
          CLAY(CLAY_IDI("cell", col + row * 8),{
                .layout = {
                  .sizing = {
                    .width = CLAY_SIZING_GROW(),
                    .height = CLAY_SIZING_GROW(),
                    }
                  },
                .cornerRadius = CLAY_CORNER_RADIUS(4),
                .backgroundColor = (Clay_Hovered() || cell_selected) ? UI.colors.highlighted_cell : cell_color,
                .aspectRatio = {.aspectRatio = 1}
          }) {
            Clay_OnHover(handle_board_cell_hover, NULL);
            CLAY_AUTO_ID({
              .layout = {
                .sizing = {
                  .height = CLAY_SIZING_GROW(0),
                  .width = CLAY_SIZING_GROW(0),
                }
              },
              .image = { .imageData = char2tex(STATE.board[row][col]) },
              .aspectRatio = {1}
            }) {}
          }
        }
      }
    }
  }
}

void main_layout() {
  CLAY(CLAY_ID("WindowContainer"), {
        .layout = {
          .sizing = {
            .height = CLAY_SIZING_GROW(),
            .width = CLAY_SIZING_GROW(),
            },
          .padding = CLAY_PADDING_ALL(8),
          .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER},
          .childGap = 8
        },
        .backgroundColor = UI.colors.background,
  }) {
    board_layout();
    CLAY(CLAY_ID("InfoPanel"), {
      .layout = {
        .sizing = {
          .width = CLAY_SIZING_GROW(50, 500),
          .height = CLAY_SIZING_GROW(),
        },
        .layoutDirection = CLAY_TOP_TO_BOTTOM,
        .padding = CLAY_PADDING_ALL(8)
      },
      .backgroundColor = UI.colors.board_background
    }) {
      CLAY_TEXT(CLAY_STRING("Move history:"), CLAY_TEXT_CONFIG({.fontSize = 32, .textColor = (Clay_Color){255, 255, 255, 255}}));
      CLAY(CLAY_ID("MoveHistoryPanel"), {
          .layout = {
            .sizing = {
              .width = CLAY_SIZING_GROW(),
              .height = CLAY_SIZING_GROW(),
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = 8,
            .padding = CLAY_PADDING_ALL(8)
          },
          .backgroundColor = UI.colors.light_background
        }
      ) {
        for (size_t i = 0; i < STATE.moves.count; i++) {
          Move move = STATE.moves.items[i];
          move_log_buffer[i][0] = move.src.col + 'a';
          move_log_buffer[i][1] = move.src.row + '1';
          move_log_buffer[i][2] = ',';
          move_log_buffer[i][3] = ' ';
          move_log_buffer[i][4] = move.dst.col + 'a';
          move_log_buffer[i][5] = move.dst.row + '1';

          Clay_String log_line = {.isStaticallyAllocated = true, .length = 6, .chars = move_log_buffer[i]};
          CLAY(CLAY_IDI("MoveContainer", i), {
               .layout = {
                .padding = CLAY_PADDING_ALL(8),
                .sizing = {.width = CLAY_SIZING_GROW()}
                },
               .backgroundColor = UI.colors.board_background
             }) {
            CLAY_TEXT(log_line, CLAY_TEXT_CONFIG({.fontSize = 32, .textColor = (Clay_Color){0, 0, 0, 255}}));
          }
        }
      }
    }
  }
}
