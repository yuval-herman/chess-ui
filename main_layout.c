#include "clay.h"

void checkers_layout() {
  for (int col = 0; col < 8; col++) {
    CLAY_AUTO_ID({
          .layout = {
            .sizing = {
              .width = CLAY_SIZING_GROW(),
              .height = CLAY_SIZING_GROW(),
            },
            .childGap = 8,
          },
          .backgroundColor = {125, 125, 125, 255}
      }) {
      for (int row = 0; row < 8; row++) {
        CLAY_AUTO_ID({
              .layout = {
                .sizing = {
                  .width = CLAY_SIZING_GROW(),
                  .height = CLAY_SIZING_GROW(),
                  }
                },
              .cornerRadius = CLAY_CORNER_RADIUS(4),
              .backgroundColor = {255, 0, 0, 255}
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
        .backgroundColor = {125, 125, 125, 255},
  }) {
    checkers_layout();
  }
}
