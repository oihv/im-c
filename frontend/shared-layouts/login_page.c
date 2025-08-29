#include "../clay.h"
// #include <string.h>
// #include <stdbool.h>
#include <stdlib.h>

typedef struct
{
    bool loggedIn;
    char username[32];
    char password[32];
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
              .childGap = 16}})
    {
        CLAY_TEXT(CLAY_STRING("EggChat"),
                  CLAY_TEXT_CONFIG({.fontId = 0,
                                    .fontSize = 32,
                                    .textColor = {255, 255, 255, 255}}));

        CLAY({.id = CLAY_ID("MainContent"),
              .backgroundColor = contentBackgroundColor,
              .clip = {.vertical = true, .childOffset = Clay_GetScrollOffset()},
              .layout = {
                  .layoutDirection = CLAY_TOP_TO_BOTTOM,
                  .childGap = 16,
                    .padding = CLAY_PADDING_ALL(16),
                    .sizing = {CLAY_SIZING_FIXED(400), CLAY_SIZING_FIXED(400)},
                    .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}},
                .cornerRadius = CLAY_CORNER_RADIUS(8)})
        {
            
        }
        // Closing brace for MainContent CLAY block
    }

    // For now, super simple: auto-login just for demo
    data->loggedIn = true;

    return Clay_EndLayout();
}
