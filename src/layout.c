#include "clay.h"
#include "definitions.h"
#include "game.h"
#include "packed_files.h"
#include "protocol.h"
#include "raylib.h"
#include "rules.h"
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define RANDOM_TESTS 1000
#define RANDOM_TESTS_100 100
#define RANDOM_TESTS_500 500

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
    int tester_code;  // last code from inner tester
    int banner_timeout;
    bool move_log_hover;
    int random_moves_left; // amount of random moves left to perform in tester mode
    bool testing_menu_open;
  } state;
  char* moves_log_buffer;
  size_t moves_log_buffer_length;
} UIData;


#define BANNER_TIMEOUT 120
#define MOVE_REPR_LENGTH 6

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
  Image b_knight = LoadImageFromMemory(".png", nd, nd_size);
  UI.textures.chess_pieces.b_knight = LoadTextureFromImage(b_knight);
  UnloadImage(b_knight);
  Image w_knight = LoadImageFromMemory(".png", nl, nl_size);
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
  UI.state.random_moves_left = 0;
  UI.state.testing_menu_open = false;

  UI.moves_log_buffer_length = 100;
  UI.moves_log_buffer = malloc(100);
}

// Returns a chess piece texture for the received char.
// Returns white variant for white==true, black otherwise.
Texture2D* piece2tex(char piece) {
  switch (piece) {
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

void do_move(Cell src, Cell dst) {
  DataMove move = game_make_chess_move((Move){
    .src = src,
    .dst = dst,
    .piece_moved = game_get_piece_at(src),
  });
  UI.state.backend_code = move.backend_code;
  UI.state.tester_code = move.tester_code;

  if (!is_code_legal(UI.state.backend_code)) UI.state.banner_timeout = BANNER_TIMEOUT;
}

void handle_test_button_hover(Clay_ElementId element_id, Clay_PointerData pointer_data,
                          void *user_data) {
  (void)element_id;
  (void)user_data;
  if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    UI.state.testing_menu_open = !UI.state.testing_menu_open;
  }
}

void handle_test_option_100(Clay_ElementId element_id, Clay_PointerData pointer_data, void* user_data) {
  (void)element_id;
  (void)user_data;
  if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    UI.state.random_moves_left = RANDOM_TESTS_100;
    UI.state.testing_menu_open = false;
  }
}

void handle_test_option_500(Clay_ElementId element_id, Clay_PointerData pointer_data, void* user_data) {
  (void)element_id;
  (void)user_data;
  if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    UI.state.random_moves_left = RANDOM_TESTS_500;
    UI.state.testing_menu_open = false;
  }
}

void handle_test_option_1000(Clay_ElementId element_id, Clay_PointerData pointer_data, void* user_data) {
  (void)element_id;
  (void)user_data;
  if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    UI.state.random_moves_left = RANDOM_TESTS;
    UI.state.testing_menu_open = false;
  }
}

void handle_move_log_hover(Clay_ElementId element_id, Clay_PointerData pointer_data, void* user_data) {
  (void)pointer_data;
  (void)user_data;
  // Don't allow viewing history while testing moves
  if (UI.state.random_moves_left > 0) return;
  // TODO: actually require clicking the log entry
  // if(pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
  UI.state.move_log_hover = true;
  game_show_board_at(element_id.offset);
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
    unsigned int col = element_id.offset % 8;
    unsigned int row = (element_id.offset - col) / 8;
    assert(col <= INT_MAX && row <= INT_MAX);

    if (UI.state.selected.col >= 0) {
      Cell selected_cell = {UI.state.selected.row, UI.state.selected.col};
      Cell dst_cell = {(int)row, (int)col};

      do_move(selected_cell, dst_cell);
      UI.state.selected.col = -1;
      UI.state.selected.row = -1;
    } else {
      UI.state.selected.col = (int)col;
      UI.state.selected.row = (int)row;
    }
  }
}

void get_move_repr(char *buffer, Move move) {
  char* repr = move_repr(move);
  
  buffer[0] = repr[0];
  buffer[1] = repr[1];
  buffer[2] = ',';
  buffer[3] = ' ';
  buffer[4] = repr[2];
  buffer[5] = repr[3];
}

void turn_indicator(bool is_white) {
  Clay_Color color = {0};
    if (game_is_whites_turn()) {
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
        .length = (int)strlen(message),
    };
    CLAY_TEXT(clay_str, CLAY_TEXT_CONFIG({
                            .fontSize = 60,
                            .textColor = (Clay_Color){255, 255, 255, 255},
                        }));
  }
}

void board_cell(int row, int col) {
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
      .image = { .imageData = piece2tex(game_get_piece_at((Cell){.row = row, .col = col})) },
      .aspectRatio = {1}
    }) {}
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
    turn_indicator(game_is_white_up());
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
          board_cell(row, col);
        }
      }
    }
    turn_indicator(!game_is_white_up());
  }
}

void info_panel() {
  static char* piece_count_buffer[6];
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
      CLAY(CLAY_ID("PieceCountsContainer"),
           {.layout = {.padding = CLAY_PADDING_ALL(8),
                       .sizing = {.width = CLAY_SIZING_GROW()},
                       .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}},
            }) {
        CLAY(CLAY_ID("BlackPieceIcon"),
             {.layout = {.sizing =
                             {
                                 .height = CLAY_SIZING_FIXED(30),
                                 .width = CLAY_SIZING_FIXED(30),
                             }},
              .image = {.imageData = &UI.textures.chess_pieces.b_pawn},
              .aspectRatio = {1}}) {}
        char* t_buffer = (char*)piece_count_buffer;
        snprintf(t_buffer, 3, "%u", game_get_white_count());
        Clay_String white_count_str = {.length = 2, .isStaticallyAllocated = true, .chars = t_buffer};
        CLAY_TEXT(white_count_str, CLAY_TEXT_CONFIG({.fontSize = 32, .textColor = (Clay_Color){0, 0, 0, 255}}));
        CLAY(CLAY_ID("WhitePieceIcon"),
             {.layout = {.sizing =
                             {
                                 .height = CLAY_SIZING_FIXED(30),
                                 .width = CLAY_SIZING_FIXED(30),
                             }},
              .image = {.imageData = &UI.textures.chess_pieces.w_pawn},
              .aspectRatio = {1}}) {}
        t_buffer = (char *)piece_count_buffer + 3;
        snprintf(t_buffer, 3, "%u", game_get_black_count());
        Clay_String black_count_str = {.length = 2, .isStaticallyAllocated = true, .chars = t_buffer};
        CLAY_TEXT(black_count_str, CLAY_TEXT_CONFIG({.fontSize = 32, .textColor = (Clay_Color){255, 255, 255, 255}}));
        CLAY(CLAY_ID("TestButton"),
             {
               .layout =
                   {
                     .sizing =
                         {
                           .height = CLAY_SIZING_FIT(),
                           .width = CLAY_SIZING_FIT(),
                         },
                     .padding = CLAY_PADDING_ALL(4),
                   },
               .backgroundColor = Clay_Hovered() ? (Clay_Color){125, 125, 125, 255}
                                                 : (Clay_Color){255, 255, 255, 255},
             }) {
          Clay_OnHover(handle_test_button_hover, NULL);
          CLAY_TEXT(CLAY_STRING("Test"), CLAY_TEXT_CONFIG({.fontSize = 32, .textColor = (Clay_Color){0, 0, 0, 255}}));
        }

        if (UI.state.testing_menu_open) {
          CLAY(CLAY_ID("TestMenu"), {
              .layout = {
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .sizing = {.width = CLAY_SIZING_FIT(), .height = CLAY_SIZING_FIT()},
                .childGap = 4,
                .padding = CLAY_PADDING_ALL(4),
              },
              .floating = { .attachTo = CLAY_ATTACH_TO_PARENT, .attachPoints = { .element = CLAY_ATTACH_POINT_LEFT_TOP, .parent = CLAY_ATTACH_POINT_LEFT_BOTTOM }, .offset = { .x = 0, .y = 6 }, .zIndex = 1000, .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_CAPTURE, .clipTo = CLAY_CLIP_TO_NONE },
              .backgroundColor = (Clay_Color){240,240,240,255},
            }) {
            CLAY(CLAY_ID("TestOpt100"), {.layout = {.sizing = {.width = CLAY_SIZING_FIT(), .height = CLAY_SIZING_FIT()}}}) {
              Clay_OnHover(handle_test_option_100, NULL);
              CLAY_TEXT(CLAY_STRING("Test 100 random moves"), CLAY_TEXT_CONFIG({.fontSize=20, .textColor=(Clay_Color){0,0,0,255}}));
            }
            CLAY(CLAY_ID("TestOpt500"), {.layout = {.sizing = {.width = CLAY_SIZING_FIT(), .height = CLAY_SIZING_FIT()}}}) {
              Clay_OnHover(handle_test_option_500, NULL);
              CLAY_TEXT(CLAY_STRING("Test 500 random moves"), CLAY_TEXT_CONFIG({.fontSize=20, .textColor=(Clay_Color){0,0,0,255}}));
            }
            CLAY(CLAY_ID("TestOpt1000"), {.layout = {.sizing = {.width = CLAY_SIZING_FIT(), .height = CLAY_SIZING_FIT()}}}) {
              Clay_OnHover(handle_test_option_1000, NULL);
              CLAY_TEXT(CLAY_STRING("Test 1000 random moves"), CLAY_TEXT_CONFIG({.fontSize=20, .textColor=(Clay_Color){0,0,0,255}}));
            }
          }
        }
        }
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
      DataMovesArr moves = game_get_moves_log();
      if (UI.moves_log_buffer_length < moves.count * MOVE_REPR_LENGTH) {
        UI.moves_log_buffer_length *= 2;
        char *buffer = realloc(UI.moves_log_buffer, UI.moves_log_buffer_length);
        assert(buffer && "Memory allocation failed");
        UI.moves_log_buffer = buffer;
      }
      for (size_t i = 0; i < moves.count; i++) {
        char* buffer_slice = UI.moves_log_buffer + i * MOVE_REPR_LENGTH + 1;
        Clay_String log_line = {
            .isStaticallyAllocated = false,
            .length = MOVE_REPR_LENGTH,
            .chars = buffer_slice,
        };
        get_move_repr(buffer_slice, moves.items[i].move);
        Clay_Color bg_color =
            moves.items[i].tester_code == moves.items[i].backend_code
                ? UI.colors.board_background
                : UI.colors.banner_background;
            CLAY(CLAY_IDI("MoveContainer", i),
                 {
                     .layout =
                         {
                             .padding = CLAY_PADDING_ALL(8),
                             .sizing = {.width = CLAY_SIZING_GROW()},
                             .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                         },
                     .backgroundColor =
                         Clay_Hovered() ? UI.colors.highlighted_cell : bg_color,
                 }) {
          UI.state.move_log_hover = false;
          Clay_OnHover(handle_move_log_hover, NULL);
          CLAY_TEXT(log_line, CLAY_TEXT_CONFIG({.fontSize = 32, .textColor = (Clay_Color){0, 0, 0, 255}}));
            CLAY(CLAY_IDI("LogLineIcon", i), {
            .layout = {.sizing =
                           {
                               .height = CLAY_SIZING_FIXED(30),
                               .width = CLAY_SIZING_FIXED(30),
                           }},
            .image = {.imageData = piece2tex(moves.items[i].src_piece)},
            .aspectRatio = {
              1
            }}) {}
        }
      }
    }
  }
}

void main_layout() {

  if (game_is_viewing_history() && !UI.state.move_log_hover &&
      !Clay_PointerOver(CLAY_ID("MoveHistoryPanel")))
    game_reset_board();
  if (UI.state.random_moves_left > 0) {
    Cell src = game_get_random_player_cell(game_is_whites_turn());
    UI.state.selected.row = src.row;
    UI.state.selected.col = src.col;
    Cell dst = get_random_move_cell(src);
    do_move(src, dst);
    UI.state.banner_timeout = -1;
    UI.state.random_moves_left--;
    SetTraceLogLevel(LOG_NONE);
    SetTargetFPS(0);
    SetTraceLogLevel(LOG_INFO);
  } else {
    SetTraceLogLevel(LOG_NONE);
    SetTargetFPS(60);
    SetTraceLogLevel(LOG_INFO);
  }
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
