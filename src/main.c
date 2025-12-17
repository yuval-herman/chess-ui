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

void HandleClayErrors(Clay_ErrorData error_data) {
  printf("CLAY ERROR: %s\n", error_data.errorText.chars);
  exit(1);
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

  Font fonts[1] = {LoadFontFromMemory(".ttf", roboto, roboto_size, 60, NULL, 0)};
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
