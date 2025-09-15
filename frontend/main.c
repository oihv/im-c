#include "renderers/raylib/raylib.h"
#include "shared-layouts/login_page.h"
#include <stdio.h>
#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "network/websocket_service.h"
#include "components/textbox.c"
#include "page/debug_page.c"
#include "renderers/raylib/clay_renderer_raylib.c"
#include "shared-layouts/chat_interface.c"
#include "shared-layouts/login_page.c"
#include <stdint.h>

// Forward declaration
extern my_conn ws_connection;

// Error handling helper
void HandleClayErrors(Clay_ErrorData errorData) {
  printf("%s", errorData.errorText.chars);
}

int main(void) {
  Clay_Raylib_Initialize(
       1024, 768, "EggChat",
       FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT |
           FLAG_VSYNC_HINT);

  uint64_t clayRequiredMemory = Clay_MinMemorySize();
  Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(
      clayRequiredMemory, malloc(clayRequiredMemory));
  Clay_Initialize(
      clayMemory,
      (Clay_Dimensions){.width = GetScreenWidth(), .height = GetScreenHeight()},
      (Clay_ErrorHandler){HandleClayErrors});
  Font fonts[1];
  fonts[FONT_ID_BODY_16] =
      LoadFontEx("resources/Roboto-Regular.ttf", 48, 0, 400);
  SetTextureFilter(fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);
  Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

  // Initialize persistent data per page
  LoginPage_Data loginData = LoginPage_Initialize();
  ChatApp_Data data = ChatApp_Initialize();

  // Connect login credentials
  data.login_credentials = &loginData;
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
      
       // Pass WebSocket data to the chat app data
      data.ws_data = ws_data;

      // Check if connection succeeded
      if (ws_data->connected == true) loginData.loggedIn = true;
      else if (ws_data->error == true) {
        loginData.loggedIn = false;
        loginData.status = ConnectionError;
      }

      // Handle new messages (optional: for debugging)
      if (ws_data->has_new_message) {
        // printf("New messages received and will be displayed in UI!\n");
        ws_data->has_new_message = false;
      }
    }

    // Run once per frame
    Clay_SetLayoutDimensions((Clay_Dimensions){.width = GetScreenWidth(),
                                               .height = GetScreenHeight()});

    Vector2 mousePosition = GetMousePosition();
    Vector2 scrollDelta = GetMouseWheelMoveV();
    
    // Enhance scroll sensitivity and smoothness
    float scrollMultiplier = 3.0f; // Increase scroll sensitivity
    scrollDelta.x *= scrollMultiplier;
    scrollDelta.y *= scrollMultiplier;
    
    Clay_SetPointerState((Clay_Vector2){mousePosition.x, mousePosition.y},
                         IsMouseButtonDown(0));
    Clay_UpdateScrollContainers(
        true, (Clay_Vector2){scrollDelta.x, scrollDelta.y}, GetFrameTime());

    Clay_RenderCommandArray renderCommands;
    if (!loginData.loggedIn) {
      renderCommands = LoginPage_CreateLayout(&loginData);
      // renderCommands = DebugPage_CreateLayout(buffer, &frameCounts);
    } else {
      renderCommands = ChatApp_CreateLayout(&data);
    }

    BeginDrawing();
    ClearBackground(BLACK);
    Clay_Raylib_Render(renderCommands, fonts);
    EndDrawing();
  }
  Clay_Raylib_Close();
}
