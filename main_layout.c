#include "clay.h"
#include "definitions.h"
#include <raylib.h>

// Returns a chess piece texture for the received char.
// Returns white variant for white==true, black otherwise.
Texture2D* char2tex(ChessTextures *tex, char c) {
  switch (c) {
    case 'b': return &tex->b_bishop;
    case 'B': return &tex->w_bishop;
    case 'k': return &tex->b_king;
    case 'K': return &tex->w_king;
    case 'n': return &tex->b_knight;
    case 'N': return &tex->w_knight;
    case 'p': return &tex->b_pawn;
    case 'P': return &tex->w_pawn;
    case 'q': return &tex->b_queen;
    case 'Q': return &tex->w_queen;
    case 'r': return &tex->b_rook;
    case 'R': return &tex->w_rook;
    default: return NULL;
  }
}

typedef struct {
  int *selected_row;
  int *selected_col;
} BoardHoverData;

void handle_board_cell_hover(Clay_ElementId element_id,
                             Clay_PointerData pointer_data, void *user_data) {
  BoardHoverData *click_data = user_data;
  if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    int col = element_id.offset % 8;
    int row = (element_id.offset - col) / 8;
    *click_data->selected_col = col;
    *click_data->selected_row = row;
  }
}

void board_layout(ChessTextures *chess_pieces, char board[8][8]) {
  Clay_Color even_cell_color = {100, 100, 100, 255};
  Clay_Color odd_cell_color  = {125, 125, 125, 255};
  static int selected_row = -1;
  static int selected_col = -1;
  static BoardHoverData click_data = {&selected_row, &selected_col};

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
      .backgroundColor = {112, 112, 112, 255},
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
          Clay_Color cell_color = (row + col) % 2 ? odd_cell_color : even_cell_color;
          CLAY(CLAY_IDI("cell", col + row * 8),{
                .layout = {
                  .sizing = {
                    .width = CLAY_SIZING_GROW(),
                    .height = CLAY_SIZING_GROW(),
                    }
                  },
                .cornerRadius = CLAY_CORNER_RADIUS(4),
                .backgroundColor = (Clay_Hovered() || (row == selected_row && col == selected_col)) ? (Clay_Color){125,125,100,255} : cell_color,
                .aspectRatio = {.aspectRatio = 1}
          }) {
            Clay_OnHover(handle_board_cell_hover, &click_data);
            CLAY_AUTO_ID({
              .layout = {
                .sizing = {
                  .height = CLAY_SIZING_GROW(0),
                  .width = CLAY_SIZING_GROW(0),
                }
              },
              .image = { .imageData = char2tex(chess_pieces, board[row][col]) },
              .aspectRatio = {1}
            }) {}
          }
        }
      }
    }
  }
}

void main_layout(ChessTextures *chess_pieces, char board[8][8]) {
  CLAY(CLAY_ID("WindowContainer"), {
        .layout = {
          .sizing = {
            .height = CLAY_SIZING_GROW(),
            .width = CLAY_SIZING_GROW(),
            },
          .padding = CLAY_PADDING_ALL(8),
        },
        .backgroundColor = {80, 80, 80, 255},
  }) {
    board_layout(chess_pieces, board);
  }
}
