#include "definitions.h"
#include "game.h"
#include "layout.c"
#include "protocol.h"
#include "raylib.h"
#include "stdio.h"

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
  UI.textures.chess_pieces.b_bishop = LoadTexture("resources/sprites/bd.png");
  UI.textures.chess_pieces.w_bishop = LoadTexture("resources/sprites/bl.png");
  UI.textures.chess_pieces.b_king   = LoadTexture("resources/sprites/kd.png");
  UI.textures.chess_pieces.w_king   = LoadTexture("resources/sprites/kl.png");
  UI.textures.chess_pieces.b_knight = LoadTexture("resources/sprites/nd.png");
  UI.textures.chess_pieces.w_knight = LoadTexture("resources/sprites/nl.png");
  UI.textures.chess_pieces.b_pawn   = LoadTexture("resources/sprites/pd.png");
  UI.textures.chess_pieces.w_pawn   = LoadTexture("resources/sprites/pl.png");
  UI.textures.chess_pieces.b_queen  = LoadTexture("resources/sprites/qd.png");
  UI.textures.chess_pieces.w_queen  = LoadTexture("resources/sprites/ql.png");
  UI.textures.chess_pieces.b_rook   = LoadTexture("resources/sprites/rd.png");
  UI.textures.chess_pieces.w_rook   = LoadTexture("resources/sprites/rl.png");

  UI.colors.background       = (Clay_Color){80, 80, 80, 255};
  UI.colors.light_background = (Clay_Color){150, 150, 150, 255};
  UI.colors.board_background = (Clay_Color){112, 112, 112, 255};
  UI.colors.even_cell        = (Clay_Color){100, 100, 100, 255};
  UI.colors.odd_cell         = (Clay_Color){125, 125, 125, 255};
  UI.colors.highlighted_cell = (Clay_Color){125, 125, 100, 255};
  UI.colors.turn_indicator   = (Clay_Color){125, 125, 100, 255};
  UI.colors.banner_background   = (Clay_Color){200, 125, 125, 175};
}

int main(void) {
#ifndef UI_WORK
  if (!protocol_init()) {
    protocol_close();
    return 1;
  }
#endif

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
#ifndef UI_WORK
  protocol_close();
#endif
  Clay_Raylib_Close();
  return 0;
}
