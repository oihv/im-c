#include "../clay.h"
// #include <string.h>
// #include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mainpage.h"
#include "../components/textbox.h"
#include "login-page.h"

typedef struct
{
    intptr_t offset;
    intptr_t memory;
} LoginPage_Arena;


LoginPage_Data LoginPage_Initialize()
{
    // LoginPage_Data data = {.arena = {.memory = (intptr_t)malloc(1024)}};
    // memset((void*)data.arena.memory, 0, 1024);
    LoginPage_Data data = { .focus_len = 2, .loggedIn = true }; // Initialize to 0
    return data;
}

Clay_RenderCommandArray LoginPage_CreateLayout(LoginPage_Data *data)
{
    // data->arena.offset = 0;
    Clay_BeginLayout();

    Clay_Sizing layoutExpand = {
        .width = CLAY_SIZING_GROW(0),
        .height = CLAY_SIZING_GROW(0)};

    Clay_Color contentBackgroundColor = {90, 90, 90, 255};

    CLAY({.id = CLAY_ID("OuterContainer"),
          .backgroundColor = {204, 153, 0, 255},
          .layout = {
              .layoutDirection = CLAY_TOP_TO_BOTTOM,
              .sizing = layoutExpand,
              .padding = CLAY_PADDING_ALL(16),
              .childGap = 16,
              .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}}})
    {
        CLAY({.id = CLAY_ID("MainContent"),
              .backgroundColor = contentBackgroundColor,
              .clip = {.vertical = true, .childOffset = Clay_GetScrollOffset()},
              .layout = {
                  .layoutDirection = CLAY_TOP_TO_BOTTOM,
                  .padding = CLAY_PADDING_ALL(16),
                  .sizing = {CLAY_SIZING_PERCENT(0.60), CLAY_SIZING_PERCENT(0.60)},
                       .childGap = 16,
                       .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}
              },
              .cornerRadius = CLAY_CORNER_RADIUS(8)})
        {
            CLAY({
                .id = CLAY_ID("TItleBar"),
                .layout = {.padding = {0, 0, 0, 50}},
                
            })
            {
            CLAY_TEXT(CLAY_STRING("EggChat"), CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                                 .fontSize = 32,
                                                                 .textColor = {255, 255, 255, 255}}));
                                                                 
            };

            CLAY({
                .id = CLAY_ID("Topcontent"),
                .layout = {.sizing = {
                               .width = CLAY_SIZING_GROW(0)},
                               
                            },
            })
            {
                CLAY({.layout = {.padding = {16, 16, 8, 8},
                                 .sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                                 .childGap = 8,
                                 // .childAlignment = {.x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_TOP}
                                 },
                      .backgroundColor = {140, 140, 140, 255},
                      .cornerRadius = CLAY_CORNER_RADIUS(5)})
                {
                    // CLAY_TEXT(CLAY_STRING("Username:"), CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                    //                                                      .fontSize = 16,
                    //                                                      .textColor = {255, 255, 255, 255}}));

                    // Get value from the arena
                    // bool* isFocus = (bool*) (data->arena.memory + data->arena.offset);
                    // data->arena.offset += sizeof(bool);
                    //  printf("uname offset: %d\n", (int)data->arena.offset);

                    Component_TextBoxData username_data = (Component_TextBoxData) {
                       .id = CLAY_STRING("username_textbox"),
                       .textConfig = (Clay_TextElementConfig) {
                                     .fontId = FONT_ID_BODY_16,
                                     .fontSize = 16,
                                     .textColor = {255, 255, 255, 255}},
                       .buffer = data->username_buf,
                       .frameCount = &(data->frameCount),
                       .len = &data->username_len,
                       .maxLen = sizeof(data->username_buf),
                       .placeholder = CLAY_STRING("Enter your username:"),
                       .eventData = (TextBoxEventData) {
                         .focusList = data->focusList,
                         .isFocus = data->focusList,
                         .focus_len = 2,
                       }
                     };

                     renderTextBox(&username_data);
                }
            }

            CLAY({
                .id = CLAY_ID("MIDcontent"),
                .layout = {.sizing = {
                               .width = CLAY_SIZING_GROW(0)},
                       
                            },
            })
            {

                CLAY({.layout = {.padding = {16, 16, 8, 8},
                                 .sizing = {.width = CLAY_SIZING_GROW(0)},
                                 .childGap = 8,
                                 .childAlignment = {.x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_BOTTOM}},
                      .backgroundColor = {140, 140, 140, 255},
                      .cornerRadius = CLAY_CORNER_RADIUS(5)})
                {

                    // CLAY_TEXT(CLAY_STRING("IP Address:"), CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                    //                                                      .fontSize = 16,
                    //                                                      .textColor = {255, 255, 255, 255}}));

                    // Get value from the arena
                    // bool* isFocus = (bool*) (data->arena.memory + data->arena.offset);
                    // data->arena.offset += sizeof(bool);
                    //  printf("ipaddr offset: %d\n", (int)data->arena.offset);

                    Component_TextBoxData pass_data = (Component_TextBoxData) {
                       .id = CLAY_STRING("ipaddr_textbox"),
                       .textConfig = (Clay_TextElementConfig) {
                                     .fontId = FONT_ID_BODY_16,
                                     .fontSize = 16,
                                     .textColor = {255, 255, 255, 255}},
                       .buffer = data->pass_buf,
                       .frameCount = &(data->frameCount),
                       .len = &data->pass_len,
                       .maxLen = sizeof(data->pass_buf),
                       .placeholder = CLAY_STRING("Enter IP Address:"),
                       .eventData = (TextBoxEventData) {
                         .focusList = data->focusList,
                         .isFocus = &data->focusList[1],
                         .focus_len = 2,
                       }
                    };

                    renderTextBox(&pass_data);
                }
            }

            CLAY({
                .id = CLAY_ID("BOTcontent"),
                .layout = {.sizing = {
                               .width = CLAY_SIZING_GROW(0)},
                           .padding = {.top = 40},
                           .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}
                            },
            })
            {

                CLAY({.layout = {.padding = {16, 16, 8, 8},
                                 .sizing = {.width = CLAY_SIZING_FIT(0), .height = CLAY_SIZING_FIXED(40)},
                                 .childGap = 8,
                                 .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}},
                      .backgroundColor = {140, 140, 140, 255},
                      .cornerRadius = CLAY_CORNER_RADIUS(5)})
                {

                    CLAY_TEXT(CLAY_STRING("LOGIN"), CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                                         .fontSize = 18,
                                                                         .textColor = {255, 255, 255, 255}}));
                }
            }
        }
    }

                          printf("%d %d\n", data->focusList[0], data->focusList[1]);

    // For now, super simple: auto-login just for demo
    // data->loggedIn = true;

    return Clay_EndLayout();
}
