#include "clay.h"
#include "definitions.h"
#include <raylib.h>
#include <time.h>

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

void handle_board_cell_hover(Clay_ElementId element_id,
                             Clay_PointerData pointer_data, void *user_data) {
  (void)user_data;
  if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    int col = element_id.offset % 8;
    int row = (element_id.offset - col) / 8;
    if (STATE.selected.col >= 0) {
      char piece = STATE.board[STATE.selected.row][STATE.selected.col];
      STATE.board[STATE.selected.row][STATE.selected.col] = '#';
      STATE.board[row][col] = piece;

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
      CLAY(CLAY_ID("MoveHistoryPanle"), {
          .layout = {
            .sizing = {
              .width = CLAY_SIZING_GROW(),
              .height = CLAY_SIZING_GROW(),
            }
          },
          .backgroundColor = UI.colors.light_background
        }
      );
    }
  }
}
