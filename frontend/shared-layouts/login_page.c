#include "../clay.h"
// #include <string.h>
// #include <stdbool.h>
#include <stdlib.h>

typedef struct {
    bool loggedIn;
    char username[32];
    char password[32];
} LoginPage_Data;

LoginPage_Data LoginPage_Initialize() {
    LoginPage_Data data = {0};
    data.loggedIn = false;
    return data;
}

Clay_RenderCommandArray LoginPage_CreateLayout(LoginPage_Data *data) {
    Clay_BeginLayout();

    Clay_Sizing layoutExpand = {
        .width = CLAY_SIZING_GROW(0),
        .height = CLAY_SIZING_GROW(0)};

    CLAY({.id = CLAY_ID("OuterContainer"),
          .backgroundColor = {43, 41, 51, 255},
          .layout = {
              .layoutDirection = CLAY_TOP_TO_BOTTOM,
              .sizing = layoutExpand,
              .padding = CLAY_PADDING_ALL(16),
              .childGap = 16}}) {}

    // For now, super simple: auto-login just for demo
    data->loggedIn = true;

    return Clay_EndLayout();
}
