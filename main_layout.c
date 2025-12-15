#include "clay.h"
#include "definitions.h"
#include "game.h"
#include "protocol.h"
#include "raylib.h"
#include <assert.h>
#include <string.h>
#include <time.h>

// Average chess games have around 30-40 moves.
// The moves log shows moves in this format:
// "pe2, b8" MOVE_REPR_LENGTH chars
// I allow up to 200 moves per game, if you want longer games, adjust this accordingly.
#define MAX_LOGS 200
#define BANNER_TIMEOUT 120
char move_log_buffer[MAX_LOGS][MOVE_REPR_LENGTH];

typedef struct {
  Cell selected;
  int backend_code; // last code received from the backend
  int banner_timeout;
} UIState;

UIState UI_STATE = {
    .selected = {-1, -1},
    .backend_code = -1,
    .banner_timeout = -1,
};

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

void handle_banner_hover(Clay_ElementId element_id, Clay_PointerData pointer_data, void* user_data) {
  (void)element_id;
  (void)user_data;
  if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    UI_STATE.banner_timeout = -1;
  }
}

void handle_board_cell_hover(Clay_ElementId element_id,
                             Clay_PointerData pointer_data, void *user_data) {
  (void)user_data;
  if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    int col = element_id.offset % 8;
    int row = (element_id.offset - col) / 8;
    if (UI_STATE.selected.col >= 0) {
      UI_STATE.backend_code = make_chess_move((Move){
          .src = {UI_STATE.selected.row, UI_STATE.selected.col},
          .dst = {row, col},
      });
      if (!is_code_legal(UI_STATE.backend_code))
        UI_STATE.banner_timeout = BANNER_TIMEOUT;

      UI_STATE.selected.col = -1;
      UI_STATE.selected.row = -1;
    } else {
      UI_STATE.selected.col = col;
      UI_STATE.selected.row = row;
    }
  }
}

void turn_indicator(bool is_white) {
  Clay_Color color = {0};
  if (is_whites_turn()) {
    if (is_white)
      color = UI.colors.turn_indicator;
  } else if (!is_white) {
    color = UI.colors.turn_indicator;
  }
  CLAY(CLAY_IDI("TurnIndicator", (int)is_white), {
    .layout = {
      .sizing = {
        .width = CLAY_SIZING_GROW(),
        .height = CLAY_SIZING_FIXED(10)
      }
    },
    .backgroundColor = color
   }) {}
}

void illegal_move_banner() {
  if (is_code_legal(UI_STATE.backend_code) || UI_STATE.banner_timeout < 0)
    return;
  UI_STATE.banner_timeout--;
  CLAY(CLAY_ID("IllegalMoveBanner"), {
       .layout = {
         .sizing = {
           .width = CLAY_SIZING_GROW(),
           .height = CLAY_SIZING_GROW()
         },
         .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}
       },
      .backgroundColor = UI.colors.banner_background,
      .floating = {.attachTo = CLAY_ATTACH_TO_PARENT}
     }) {
    Clay_OnHover(handle_banner_hover, NULL);
    const char *message = code2str(UI_STATE.backend_code);
    Clay_String clay_str = {
        .isStaticallyAllocated = true,
        .chars = message,
        .length = strlen(message),
    };
    CLAY_TEXT(clay_str, CLAY_TEXT_CONFIG({
                            .fontSize = 60,
                            .textColor = (Clay_Color){255, 255, 255, 255},
                        }));
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
  }) {
    turn_indicator(is_white_up());
    for (int row = 0; row < 8; row++) {
      CLAY(CLAY_IDI("RowContainer", row), {
            .layout = {
              .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
              .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_GROW(),
              },
              .childGap = 8,
            }
        }) {
        for (int col = 0; col < 8; col++) {
          Clay_Color cell_color = (row + col) % 2 ? UI.colors.odd_cell : UI.colors.even_cell;
          bool cell_selected = row == UI_STATE.selected.row && col == UI_STATE.selected.col;
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
              .image = { .imageData = char2tex(get_piece_at((Cell){row, col})) },
              .aspectRatio = {1}
            }) {}
          }
        }
      }
    }
    turn_indicator(!is_white_up());
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
    illegal_move_banner();
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
          .clip = {.vertical = true, .childOffset = Clay_GetScrollOffset()},
          .backgroundColor = UI.colors.light_background
        }
      ) {
        size_t move_count = get_moves_log(move_log_buffer, MAX_LOGS);
        for (size_t i = 0; i < move_count; i++) {
          // For the actuall string we only show the second character onward, the first is the piece sign.
          Clay_String log_line = {.isStaticallyAllocated = true,
                                  .length = MOVE_REPR_LENGTH - 1,
                                  .chars = move_log_buffer[i] + 1};
          CLAY(CLAY_IDI("MoveContainer", i), {
               .layout = {
                .padding = CLAY_PADDING_ALL(8),
                .sizing = {.width = CLAY_SIZING_GROW()},
                .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}
                },
               .backgroundColor = UI.colors.board_background
             }) {
            CLAY_TEXT(log_line, CLAY_TEXT_CONFIG({.fontSize = 32, .textColor = (Clay_Color){0, 0, 0, 255}}));
              CLAY(CLAY_IDI("LogLineIcon", i), {
              .layout = {.sizing =
                             {
                                 .height = CLAY_SIZING_FIXED(30),
                                 .width = CLAY_SIZING_FIXED(30),
                             }},
              .image = {.imageData = char2tex(move_log_buffer[i][0])},
              .aspectRatio = {
                1
              }}) {}
          }
        }
      }
    }
  }
}
