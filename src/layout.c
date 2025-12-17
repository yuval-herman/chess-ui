#include "clay.h"
#include "definitions.h"
#include "game.h"
#include "packed_files.h"
#include "protocol.h"
#include "raylib.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct {
  struct {
    struct {
      Texture2D w_king;
      Texture2D w_queen;
      Texture2D w_rook;
      Texture2D w_bishop;
      Texture2D w_knight;
      Texture2D w_pawn;
      Texture2D b_king;
      Texture2D b_queen;
      Texture2D b_rook;
      Texture2D b_bishop;
      Texture2D b_knight;
      Texture2D b_pawn;
    } chess_pieces;
  } textures;
  struct {
      Clay_Color background;
      Clay_Color board_background;
      Clay_Color light_background;
      Clay_Color even_cell;
      Clay_Color odd_cell;
      Clay_Color highlighted_cell;
      Clay_Color turn_indicator;
      Clay_Color banner_background;
  } colors;
  struct {
    Cell selected;
    int backend_code; // last code received from the backend
    int banner_timeout;
    bool move_log_hover;
  } state;
} UIData;

// Average chess games have around 30-40 moves.
// The moves log shows moves in this format:
// "pe2, b8" MOVE_REPR_LENGTH chars
// I allow up to 200 moves per game, if you want longer games, adjust this accordingly.
#define MAX_LOGS 200
#define BANNER_TIMEOUT 120
char move_log_buffer[MAX_LOGS][MOVE_REPR_LENGTH];
UIData UI = {0};

void initUIData() {
  Image b_bishop = LoadImageFromMemory(".png", bd, bd_size);
  UI.textures.chess_pieces.b_bishop = LoadTextureFromImage(b_bishop);
  UnloadImage(b_bishop);
  Image w_bishop = LoadImageFromMemory(".png", bl, bl_size);
  UI.textures.chess_pieces.w_bishop = LoadTextureFromImage(w_bishop);
  UnloadImage(w_bishop);
  Image b_king = LoadImageFromMemory(".png", kd, kd_size);
  UI.textures.chess_pieces.b_king   = LoadTextureFromImage(b_king);
  UnloadImage(b_king);
  Image w_king = LoadImageFromMemory(".png", kl, kl_size);
  UI.textures.chess_pieces.w_king   = LoadTextureFromImage(w_king);
  UnloadImage(w_king);
  Image b_knight = LoadImageFromMemory(".png", kd, kd_size);
  UI.textures.chess_pieces.b_knight = LoadTextureFromImage(b_knight);
  UnloadImage(b_knight);
  Image w_knight = LoadImageFromMemory(".png", kl, kl_size);
  UI.textures.chess_pieces.w_knight = LoadTextureFromImage(w_knight);
  UnloadImage(w_knight);
  Image b_pawn = LoadImageFromMemory(".png", pd, pd_size);
  UI.textures.chess_pieces.b_pawn   = LoadTextureFromImage(b_pawn);
  UnloadImage(b_pawn);
  Image w_pawn = LoadImageFromMemory(".png", pl, pl_size);
  UI.textures.chess_pieces.w_pawn   = LoadTextureFromImage(w_pawn);
  UnloadImage(w_pawn);
  Image b_queen = LoadImageFromMemory(".png", qd, qd_size);
  UI.textures.chess_pieces.b_queen  = LoadTextureFromImage(b_queen);
  UnloadImage(b_queen);
  Image w_queen = LoadImageFromMemory(".png", ql, ql_size);
  UI.textures.chess_pieces.w_queen  = LoadTextureFromImage(w_queen);
  UnloadImage(w_queen);
  Image b_rook = LoadImageFromMemory(".png", rd, rd_size);
  UI.textures.chess_pieces.b_rook   = LoadTextureFromImage(b_rook);
  UnloadImage(b_rook);
  Image w_rook = LoadImageFromMemory(".png", rl, rl_size);
  UI.textures.chess_pieces.w_rook   = LoadTextureFromImage(w_rook);
  UnloadImage(w_rook);

  UI.colors.background        = (Clay_Color){80, 80, 80, 255};
  UI.colors.light_background  = (Clay_Color){150, 150, 150, 255};
  UI.colors.board_background  = (Clay_Color){112, 112, 112, 255};
  UI.colors.even_cell         = (Clay_Color){100, 100, 100, 255};
  UI.colors.odd_cell          = (Clay_Color){125, 125, 125, 255};
  UI.colors.highlighted_cell  = (Clay_Color){125, 125, 100, 255};
  UI.colors.turn_indicator    = (Clay_Color){125, 125, 100, 255};
  UI.colors.banner_background = (Clay_Color){200, 125, 125, 175};

  UI.state.selected       = (Cell){-1, -1};
  UI.state.backend_code   = -1;
  UI.state.banner_timeout = -1;
  UI.state.move_log_hover = false;
}

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

void handle_moe_log_hover(Clay_ElementId element_id, Clay_PointerData pointer_data, void* user_data) {
  (void)pointer_data;
  (void)user_data;
  // TODO: actually require clicking the log entry
  // if(pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
  UI.state.move_log_hover = true;
  show_board_at(element_id.offset);
}

void handle_banner_hover(Clay_ElementId element_id, Clay_PointerData pointer_data, void* user_data) {
  (void)element_id;
  (void)user_data;
  if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    UI.state.banner_timeout = -1;
  }
}

void handle_board_cell_hover(Clay_ElementId element_id,
                             Clay_PointerData pointer_data, void *user_data) {
  (void)user_data;
  if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    int col = element_id.offset % 8;
    int row = (element_id.offset - col) / 8;
    if (UI.state.selected.col >= 0) {
      UI.state.backend_code = make_chess_move((Move){
          .src = {UI.state.selected.row, UI.state.selected.col},
          .dst = {row, col},
      });
      if (!is_code_legal(UI.state.backend_code))
        UI.state.banner_timeout = BANNER_TIMEOUT;

      UI.state.selected.col = -1;
      UI.state.selected.row = -1;
    } else {
      UI.state.selected.col = col;
      UI.state.selected.row = row;
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
  if (is_code_legal(UI.state.backend_code) || UI.state.banner_timeout < 0)
    return;
  UI.state.banner_timeout--;
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
    const char *message = code2str(UI.state.backend_code);
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
          bool cell_selected = row == UI.state.selected.row && col == UI.state.selected.col;
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

void info_panel() {
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
             .backgroundColor = Clay_Hovered() ? UI.colors.highlighted_cell : UI.colors.board_background
           }) {
          UI.state.move_log_hover = false;
          Clay_OnHover(handle_moe_log_hover, NULL);
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

void main_layout() {
  if(is_viewing_history() && !UI.state.move_log_hover) reset_board();
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
    info_panel();
  }
}
