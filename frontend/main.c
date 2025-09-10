#include "renderers/raylib/raylib.h"
#include "shared-layouts/login-page.h"
#include <stdio.h>
#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "network/websocket_service.h"
#include "page/test-page.c"
#include "renderers/raylib/clay_renderer_raylib.c"
#include "shared-layouts/clay-video-demo.c"
#include "shared-layouts/login-page.c"
#include <stdint.h>

// Forward declaration
extern my_conn ws_connection;

// This function is new since the video was published
void HandleClayErrors(Clay_ErrorData errorData) {
  printf("%s", errorData.errorText.chars);
}

Clay_RenderCommandArray ClayIMCTest_CreateLayout(char *buffer,
                                                 uint16_t *frameCount);

int main(void) {
  Clay_Raylib_Initialize(
      1024, 768, "Introducing Clay Demo",
      FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT |
          FLAG_VSYNC_HINT); // Extra parameters to this function are new since
                            // the video was published

  uint64_t clayRequiredMemory = Clay_MinMemorySize();
  Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(
      clayRequiredMemory, malloc(clayRequiredMemory));
  Clay_Initialize(
      clayMemory,
      (Clay_Dimensions){.width = GetScreenWidth(), .height = GetScreenHeight()},
      (Clay_ErrorHandler){HandleClayErrors}); // This final argument is new
                                              // since the video was published
  Font fonts[1];
  fonts[FONT_ID_BODY_16] =
      LoadFontEx("resources/Roboto-Regular.ttf", 48, 0, 400);
  SetTextureFilter(fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);
  Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

  LoginPage_Data loginData = LoginPage_Initialize();
  ClayVideoDemo_Data data = ClayVideoDemo_Initialize();
  uint16_t frameCounts = 0; // For button testing

  // Enable debugger
  Clay_SetDebugModeEnabled(true);
  char buffer[MAX_INPUT_CHAR + 1] = "\0";

  if (!websocket_service_init()) {
    lwsl_err("Failed to initialize websocket connection!");
    return 1;
  }

  while (!WindowShouldClose()) {
    // Set mouse cursor back
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    // Initialize WebSocket service
    if (loginData.status == InitiateConnect) {
      websocket_service_connect();
      loginData.status = Connecting;
    }

    if (loginData.status == Connected || loginData.status == Connecting) {
      // Update WebSocket service every frame
      WebSocketData *ws_data = websocket_service_update();

      // Check if connection succeeded
      if (ws_data->connected == true) loginData.loggedIn = true;
      else if (ws_data->error == true) {
        loginData.loggedIn = false;
        loginData.status = ConnectionError;
        printf("loh\n");
      }

      // Handle new messages
      if (ws_data->has_new_message) {
        printf("Received: %s\n", ws_data->message);
        ws_data->has_new_message = false;
      }
    }

    // Run once per frame
    Clay_SetLayoutDimensions((Clay_Dimensions){.width = GetScreenWidth(),
                                               .height = GetScreenHeight()});

    Vector2 mousePosition = GetMousePosition();
    Vector2 scrollDelta = GetMouseWheelMoveV();
    Clay_SetPointerState((Clay_Vector2){mousePosition.x, mousePosition.y},
                         IsMouseButtonDown(0));
    Clay_UpdateScrollContainers(
        true, (Clay_Vector2){scrollDelta.x, scrollDelta.y}, GetFrameTime());

    Clay_RenderCommandArray renderCommands;
    if (!loginData.loggedIn) {
      renderCommands = LoginPage_CreateLayout(&loginData);
      // renderCommands = ClayIMCTest_CreateLayout(buffer, &frameCounts);
    } else {
      renderCommands = ClayVideoDemo_CreateLayout(&data);
    }

    BeginDrawing();
    ClearBackground(BLACK);
    Clay_Raylib_Render(renderCommands, fonts);
    EndDrawing();
    websocket_service_send("Hello from client!");
  }
  // This function is new since the video was published
  Clay_Raylib_Close();
}
