#include "clay.h"
#include "definitions.h"
#include <raylib.h>

void board_layout(ChessTextures *chess_pieces) {
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
              .image = { .imageData = &chess_pieces->b_bishop },
              .aspectRatio = {1}
            }) {}
          }
        }
      }
    }
  }
}

void main_layout(ChessTextures *chess_pieces) {
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
    board_layout(chess_pieces);
  }
}
