#include "../clay.h"
// #include <string.h>
// #include <stdbool.h>
#include "../components/textbox.h"
#include "../network/websocket_service.h"
#include "../renderers/raylib/raylib.h"
#include "login-page.h"
#include "mainpage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration
extern my_conn ws_connection;

typedef struct {
  intptr_t offset;
  intptr_t memory;
} LoginPage_Arena;

LoginPage_Data LoginPage_Initialize() {
  // LoginPage_Data data = {.arena = {.memory = (intptr_t)malloc(1024)}};
  // memset((void*)data.arena.memory, 0, 1024);
  LoginPage_Data data = {.focus_len = 2, .loggedIn = false}; // Initialize to 0
  return data;
}

// TODO: maybe add enum for better error handling?
bool parseSocketData(char *socketData, char *ipaddr, char *port) {
  char *p = socketData;
  int i = 0;
  for (; i < 25 && *(p + i) != ':'; i++) {
    // TODO: return error when length is inappropriate
    ipaddr[i] = *(p + i);
  }
  ipaddr[i] = '\0';

  i++; // Still point at ';'

  int j = 0;
  for (; j < 4 && *(p + i) != '\0'; i++, j++) {
    if (*(p + i) == '_')
      break; // Remainder of blinking effect
    port[j] = *(p + i);
  }
  port[j] = '\0';

  if (j < 3)
    return false;

  return true;
}

void HandleLoginButton(Clay_ElementId elementId, Clay_PointerData pointerData,
                       intptr_t userData) {
  if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
  }
}

Clay_RenderCommandArray LoginPage_CreateLayout(LoginPage_Data *data) {
  // data->arena.offset = 0;
  Clay_BeginLayout();

  int textBoxFontSize = 24;

  Clay_Sizing layoutExpand = {.width = CLAY_SIZING_GROW(0),
                              .height = CLAY_SIZING_GROW(0)};

  Clay_Color contentBackgroundColor = {90, 90, 90, 255};

  CLAY({.id = CLAY_ID("OuterContainer"),
        .backgroundColor = {204, 153, 0, 255},
        .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .sizing = layoutExpand,
                   .padding = CLAY_PADDING_ALL(16),
                   .childGap = 16,
                   .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                      .y = CLAY_ALIGN_Y_CENTER}}}) {
    CLAY({.id = CLAY_ID("MainContent"),
          .backgroundColor = contentBackgroundColor,
          .clip = {.vertical = true, .childOffset = Clay_GetScrollOffset()},
          .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .padding = CLAY_PADDING_ALL(16),
                     .sizing = {CLAY_SIZING_PERCENT(0.60),
                                CLAY_SIZING_PERCENT(0.60)},
                     .childGap = 16,
                     .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                        .y = CLAY_ALIGN_Y_CENTER}},
          .cornerRadius = CLAY_CORNER_RADIUS(8)}) {
      CLAY({
          .id = CLAY_ID("TItleBar"),
          .layout = {.padding = {0, 0, 0, 50}},

      }) {
        CLAY_TEXT(CLAY_STRING("EggChat"),
                  CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                    .fontSize = 64,
                                    .textColor = {255, 255, 255, 255}}));
      };

      CLAY({
          .id = CLAY_ID("Topcontent"),
          .layout =
              {
                  .sizing = {.width = CLAY_SIZING_GROW(0)},

              },
      }) {
        CLAY({.layout =
                  {
                      .padding = {16, 16, 8, 8},
                      .sizing = {.width = CLAY_SIZING_GROW(0),
                                 .height = CLAY_SIZING_GROW(0)},
                      .childGap = 8,
                      // .childAlignment = {.x = CLAY_ALIGN_X_LEFT, .y =
                      // CLAY_ALIGN_Y_TOP}
                  },
              .backgroundColor = {140, 140, 140, 255},
              .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
          // CLAY_TEXT(CLAY_STRING("Username:"), CLAY_TEXT_CONFIG({.fontId =
          // FONT_ID_BODY_16,
          //                                                      .fontSize =
          //                                                      16, .textColor
          //                                                      = {255, 255,
          //                                                      255, 255}}));

          // Get value from the arena
          // bool* isFocus = (bool*) (data->arena.memory + data->arena.offset);
          // data->arena.offset += sizeof(bool);
          //  printf("uname offset: %d\n", (int)data->arena.offset);

          Component_TextBoxData username_data = (Component_TextBoxData){
              .id = CLAY_STRING("username_textbox"),
              .textConfig =
                  (Clay_TextElementConfig){.fontId = FONT_ID_BODY_16,
                                           .fontSize = textBoxFontSize,
                                           .textColor = {255, 255, 255, 255}},
              .buffer = data->username_buf,
              .frameCount = &(data->frameCount),
              .len = &data->username_len,
              .maxLen = sizeof(data->username_buf),
              .placeholder = CLAY_STRING("Enter your username:"),
              .eventData = (TextBoxEventData){
                  .focusList = data->focusList,
                  .isFocus = data->focusList,
                  .focus_len = 2,
              }};

          renderTextBox(&username_data);
        }
      }

      CLAY({
          .id = CLAY_ID("MIDcontent"),
          .layout =
              {
                  .sizing = {.width = CLAY_SIZING_GROW(0)},

              },
      }) {

        CLAY({.layout = {.padding = {16, 16, 8, 8},
                         .sizing = {.width = CLAY_SIZING_GROW(0)},
                         .childGap = 8,
                         .childAlignment = {.x = CLAY_ALIGN_X_LEFT,
                                            .y = CLAY_ALIGN_Y_BOTTOM}},
              .backgroundColor = {140, 140, 140, 255},
              .cornerRadius = CLAY_CORNER_RADIUS(5)}) {

          // CLAY_TEXT(CLAY_STRING("IP Address:"), CLAY_TEXT_CONFIG({.fontId =
          // FONT_ID_BODY_16,
          //                                                      .fontSize =
          //                                                      16, .textColor
          //                                                      = {255, 255,
          //                                                      255, 255}}));

          // Get value from the arena
          // bool* isFocus = (bool*) (data->arena.memory + data->arena.offset);
          // data->arena.offset += sizeof(bool);
          //  printf("ipaddr offset: %d\n", (int)data->arena.offset);

          Component_TextBoxData ipaddr_data = (Component_TextBoxData){
              .id = CLAY_STRING("ipaddr_textbox"),
              .textConfig =
                  (Clay_TextElementConfig){.fontId = FONT_ID_BODY_16,
                                           .fontSize = textBoxFontSize,
                                           .textColor = {255, 255, 255, 255}},
              .buffer = data->ipaddr_buf,
              .frameCount = &(data->frameCount),
              .len = &data->ipaddr_len,
              .maxLen = sizeof(data->ipaddr_buf),
              .placeholder = CLAY_STRING("Enter IP Address:"),
              .eventData = (TextBoxEventData){
                  .focusList = data->focusList,
                  .isFocus = &data->focusList[1],
                  .focus_len = 2,
              }};

          renderTextBox(&ipaddr_data);
        }
      }

      // Handle textbox clicked
      if (IsMouseButtonDown(0) && Clay_PointerOver(Clay_GetElementId(
                                      CLAY_STRING("username_textbox")))) {
        resetTextBoxFocus(data->focusList, data->focus_len);
        data->focusList[0] = true;
      } else if (IsMouseButtonDown(0) && Clay_PointerOver(Clay_GetElementId(
                                             CLAY_STRING("ipaddr_textbox")))) {
        resetTextBoxFocus(data->focusList, data->focus_len);
        data->focusList[1] = true;
      } else if (IsMouseButtonDown(0) &&
                 !Clay_PointerOver(
                     Clay_GetElementId(CLAY_STRING("ipaddr_textbox"))) &&
                 !Clay_PointerOver(
                     Clay_GetElementId(CLAY_STRING("username_textbox")))) {
        resetTextBoxFocus(data->focusList, data->focus_len);
      }

      CLAY({
          .id = CLAY_ID("BOTcontent"),
          .layout = {.sizing = {.width = CLAY_SIZING_GROW(0)},
                     .padding = {.top = 40},
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                        .y = CLAY_ALIGN_Y_CENTER}},
      }) {
        Clay_Color base = {140, 140, 140, 255};
        Clay_Color hover = {120, 120, 120, 255};
        CLAY({.id = CLAY_ID("LoginButton"),
              .layout = {.padding = {16, 16, 8, 8},
                         .sizing = {.width = CLAY_SIZING_FIT(0),
                                    .height = CLAY_SIZING_FIXED(40)},
                         .childGap = 8,
                         .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                            .y = CLAY_ALIGN_Y_CENTER}},
              .backgroundColor = (Clay_Hovered()) ? hover : base,
              .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
          CLAY_TEXT(CLAY_STRING("Login"),
                    CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                      .fontSize = 32,
                                      .textColor = {255, 255, 255, 255}}));
        }
        if (data->status == ConnectionError) {
          CLAY_TEXT(
              CLAY_STRING(
                  "Connection Error: please enter the appropriate IP adress."),
              CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                .fontSize = 18,
                               .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                                .textColor = {255, 0, 0, 255}}));
        }
      }
      if (IsMouseButtonDown(0) &&
          Clay_PointerOver(Clay_GetElementId(CLAY_STRING("LoginButton")))) {
        if (data->status == Disconnected) {
          if (parseSocketData(data->ipaddr_buf, data->ipaddr, data->port)) {
            // TODO: implement error handling
            data->status = InitiateConnect;
            ws_connection.ipaddr = data->ipaddr;
            ws_connection.port = atoi(data->port);
            printf("loginpage: %p\n", ws_connection.ipaddr);
          }
        }
      }
    }
  }

  // For now, super simple: auto-login just for demo
  // data->loggedIn = true;

  return Clay_EndLayout();
}
