#include "clay.h"

void checkered_layout() {
  Clay_Color even_cell_color = {100, 100, 100, 255};
  Clay_Color odd_cell_color  = {125, 125, 125, 255};
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
              .backgroundColor = cell_color
        }) {}
      }
    }
  }
}

void main_layout() {
  CLAY(CLAY_ID("WindowContainer"), {
        .layout = {
          .layoutDirection = CLAY_TOP_TO_BOTTOM,
          .sizing = {
            .height = CLAY_SIZING_GROW(),
            .width = CLAY_SIZING_GROW(),
            },
          .padding = CLAY_PADDING_ALL(8),
          .childGap = 8,
        },
        .backgroundColor = {80, 80, 80, 255},
  }) {
    checkered_layout();
  }
}
