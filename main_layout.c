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

void board_layout(ChessTextures *chess_pieces, char board[8][8]) {
  Clay_Color even_cell_color = {100, 100, 100, 255};
  Clay_Color odd_cell_color  = {125, 125, 125, 255};
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
          CLAY_AUTO_ID({
                .layout = {
                  .sizing = {
                    .width = CLAY_SIZING_GROW(),
                    .height = CLAY_SIZING_GROW(),
                    }
                  },
                .cornerRadius = CLAY_CORNER_RADIUS(4),
                .backgroundColor = cell_color,
                .aspectRatio = {.aspectRatio = 1}
          }) {
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
