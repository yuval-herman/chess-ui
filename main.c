#include "definitions.h"
#include "game.h"
#include "layout.c"
#include "protocol.h"
#include "raylib.h"
#include "stdio.h"
#include "packed_files.h"
#include <stdlib.h>

#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "clay_renderer_raylib.c"
#include <string.h>

UIData UI = {0};

void HandleClayErrors(Clay_ErrorData error_data) {
  printf("CLAY ERROR: %s\n", error_data.errorText.chars);
  exit(1);
}

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
}

bool wait_for_backend(Font message_font) {
#ifndef UI_WORK
  // first attempt pipe connection
  if (!protocol_init()) {
    protocol_close();
    return false;
  }

  while (!WindowShouldClose() && !protocol_has_started()) {
    BeginDrawing();
    ClearBackground(BLACK);
    const char *message = "Waiting for client connection";
    const int font_size = 24;
    Vector2 measurements = MeasureTextEx(message_font, message, font_size, 1);
    DrawText(message, (GetScreenWidth() - measurements.x) / 2,
             (GetScreenHeight() - measurements.y) / 2, font_size, RED);
    EndDrawing();
  }
#else
  (void)message_font;
#endif
  return true;
}

void clean_resources() {
#ifndef UI_WORK
  protocol_close();
#endif
  Clay_Raylib_Close();
}

int main(void) {
  Clay_Raylib_Initialize(1500, 800, "chess",
                         FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT |
                             FLAG_VSYNC_HINT);

  initUIData();
  initGameState();

  uint64_t clay_required_memory = Clay_MinMemorySize();

  Clay_Arena clay_memory = Clay_CreateArenaWithCapacityAndMemory(clay_required_memory, malloc(clay_required_memory));

  Clay_Initialize(
      clay_memory,
      (Clay_Dimensions){.width = GetScreenWidth(), .height = GetScreenHeight()},
      (Clay_ErrorHandler){HandleClayErrors, NULL});

  Font fonts[1] = {LoadFontEx("resources/fonts/Roboto-Regular.ttf", 60, NULL, 0)};
  Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

  // Clay_SetDebugModeEnabled(true);
  if(!wait_for_backend(fonts[0])) {
    clean_resources();
    return 1;
  }

  while (!WindowShouldClose()) {
    Clay_SetLayoutDimensions((Clay_Dimensions){.width = GetScreenWidth(),
                                               .height = GetScreenHeight()});

    Vector2 mousePosition = GetMousePosition();
    Vector2 scrollDelta = GetMouseWheelMoveV();
    Clay_SetPointerState((Clay_Vector2){mousePosition.x, mousePosition.y}, IsMouseButtonDown(0));
    Clay_UpdateScrollContainers(true, (Clay_Vector2){scrollDelta.x, scrollDelta.y}, GetFrameTime());

    Clay_BeginLayout();
    main_layout();

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();

    BeginDrawing();
    ClearBackground(BLACK);
    Clay_Raylib_Render(renderCommands, fonts);
    EndDrawing();
  }
  clean_resources();
  return 0;
}
