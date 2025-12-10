#include "stdio.h"
#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "clay_renderer_raylib.c"

void HandleClayErrors(Clay_ErrorData errorData) {
    printf("%s", errorData.errorText.chars);
}

int main(void) {
  Clay_Raylib_Initialize(
      1024, 768, "test windows",
      FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT |
          FLAG_VSYNC_HINT);

  uint64_t clay_required_memory = Clay_MinMemorySize();
  Clay_Arena clay_memory = Clay_CreateArenaWithCapacityAndMemory(
      clay_required_memory, malloc(clay_required_memory));
  Clay_Initialize(
      clay_memory,
      (Clay_Dimensions){.width = GetScreenWidth(), .height = GetScreenHeight()},
      (Clay_ErrorHandler){HandleClayErrors, NULL});
  Font fonts[1] = {GetFontDefault()};
  Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

  while (!WindowShouldClose()) {
    Clay_SetLayoutDimensions((Clay_Dimensions){.width = GetScreenWidth(),
                                               .height = GetScreenHeight()});

    Vector2 mousePosition = GetMousePosition();
    Vector2 scrollDelta = GetMouseWheelMoveV();
    Clay_SetPointerState((Clay_Vector2){mousePosition.x, mousePosition.y},
                         IsMouseButtonDown(0));
    Clay_UpdateScrollContainers(
        true, (Clay_Vector2){scrollDelta.x, scrollDelta.y}, GetFrameTime());

    Clay_BeginLayout();
    CLAY(CLAY_ID("WindowContainer"),
         {
             .layout = {.sizing =
                            {
                                .height = CLAY_SIZING_GROW(),
                                .width = CLAY_SIZING_GROW(),
                            }},
             .backgroundColor = {125, 125, 125, 255},
         }) {
      CLAY(CLAY_ID("Rect"), {.layout = {.sizing =
                                            {
                                                .width = CLAY_SIZING_FIXED(20),
                                                .height = CLAY_SIZING_FIXED(20),
                                            }},
                             .backgroundColor = {255, 0, 0, 255}}) {}
    }
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
