#include "../clay.h"
// #include <string.h>
// #include <stdbool.h>
#include <stdlib.h>
#include "mainpage.h"

typedef struct
{
    bool loggedIn;
    char username[32];
    char password[32];
    uint16_t length;
} LoginPage_Data;

LoginPage_Data LoginPage_Initialize()
{
    LoginPage_Data data = {0};
    data.loggedIn = false;
    return data;
}

Clay_RenderCommandArray LoginPage_CreateLayout(LoginPage_Data *data)
{
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
                                 .sizing = {.width = CLAY_SIZING_GROW(0)},
                                 .childGap = 8,
                                 .childAlignment = {.x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_TOP}},
                      .backgroundColor = {140, 140, 140, 255},
                      .cornerRadius = CLAY_CORNER_RADIUS(5)})
                {

                    CLAY_TEXT(CLAY_STRING("Username:"), CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                                         .fontSize = 16,
                                                                         .textColor = {255, 255, 255, 255}}));
                }
                {
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

                    CLAY_TEXT(CLAY_STRING("IP Address:"), CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                                         .fontSize = 16,
                                                                         .textColor = {255, 255, 255, 255}}));
                }
                {
                }
            }

            CLAY({
                .id = CLAY_ID("BOtcontent"),
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


    // For now, super simple: auto-login just for demo
    data->loggedIn = true;

    return Clay_EndLayout();
}
