#include "definitions.h"
#include "stdio.h"
#include <raylib.h>
#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "clay_renderer_raylib.c"
#include "main_layout.c"

void HandleClayErrors(Clay_ErrorData error_data) {
  printf("%s", error_data.errorText.chars);
}

int main(void) {
  char board[8][8] = {{'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
                      {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
                      {'#', '#', '#', '#', '#', '#', '#', '#'},
                      {'#', '#', '#', '#', '#', '#', '#', '#'},
                      {'#', '#', '#', '#', '#', '#', '#', '#'},
                      {'#', '#', '#', '#', '#', '#', '#', '#'},
                      {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
                      {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}};

  Clay_Raylib_Initialize(800, 400, "test window", FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);

  uint64_t clay_required_memory = Clay_MinMemorySize();

  Clay_Arena clay_memory = Clay_CreateArenaWithCapacityAndMemory(clay_required_memory, malloc(clay_required_memory));

  ChessTextures chess_textures = {
      .b_bishop = LoadTexture("sprites/bd.png"), .w_bishop = LoadTexture("sprites/bl.png"),
      .b_king   = LoadTexture("sprites/kd.png"),   .w_king = LoadTexture("sprites/kl.png"),
      .b_knight = LoadTexture("sprites/nd.png"), .w_knight = LoadTexture("sprites/nl.png"),
      .b_pawn   = LoadTexture("sprites/pd.png"),   .w_pawn = LoadTexture("sprites/pl.png"),
      .b_queen  = LoadTexture("sprites/qd.png"),  .w_queen = LoadTexture("sprites/ql.png"),
      .b_rook   = LoadTexture("sprites/rd.png"),   .w_rook = LoadTexture("sprites/rl.png"),
  };

  Clay_Initialize(
      clay_memory,
      (Clay_Dimensions){.width = GetScreenWidth(), .height = GetScreenHeight()},
      (Clay_ErrorHandler){HandleClayErrors, NULL});

  Font fonts[1] = {GetFontDefault()};
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
    main_layout(&chess_textures, board);

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();

    BeginDrawing();
    ClearBackground(BLACK);
    Clay_Raylib_Render(renderCommands, fonts);
    EndDrawing();
  }
  // This function is new since the video was published
  Clay_Raylib_Close();
  return 0;
}
