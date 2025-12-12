#include "definitions.h"
#include "stdio.h"
#include <raylib.h>
#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "clay_renderer_raylib.c"
#include "main_layout.c"

UIData UI = {0};

void HandleClayErrors(Clay_ErrorData error_data) {
  printf("%s", error_data.errorText.chars);
}

void initUIData() {
  UI.textures.chess_pieces.b_bishop = LoadTexture("sprites/bd.png");
  UI.textures.chess_pieces.w_bishop = LoadTexture("sprites/bl.png");
  UI.textures.chess_pieces.b_king   = LoadTexture("sprites/kd.png");
  UI.textures.chess_pieces.w_king   = LoadTexture("sprites/kl.png");
  UI.textures.chess_pieces.b_knight = LoadTexture("sprites/nd.png");
  UI.textures.chess_pieces.w_knight = LoadTexture("sprites/nl.png");
  UI.textures.chess_pieces.b_pawn   = LoadTexture("sprites/pd.png");
  UI.textures.chess_pieces.w_pawn   = LoadTexture("sprites/pl.png");
  UI.textures.chess_pieces.b_queen  = LoadTexture("sprites/qd.png");
  UI.textures.chess_pieces.w_queen  = LoadTexture("sprites/ql.png");
  UI.textures.chess_pieces.b_rook   = LoadTexture("sprites/rd.png");
  UI.textures.chess_pieces.w_rook   = LoadTexture("sprites/rl.png");

  UI.colors.background       = (Clay_Color){80, 80, 80, 255};
  UI.colors.board_background = (Clay_Color){112, 112, 112, 255};
  UI.colors.even_cell        = (Clay_Color){100, 100, 100, 255};
  UI.colors.odd_cell         = (Clay_Color){125, 125, 125, 255};
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

  Clay_Raylib_Initialize(800, 800, "chess", FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);

  initUIData();

  uint64_t clay_required_memory = Clay_MinMemorySize();

  Clay_Arena clay_memory = Clay_CreateArenaWithCapacityAndMemory(clay_required_memory, malloc(clay_required_memory));

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
    main_layout(board);

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
