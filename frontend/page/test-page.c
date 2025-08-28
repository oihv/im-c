#include "../clay.h"

Clay_RenderCommandArray ClayIMCTest_CreateLayout() {
  Clay_BeginLayout();

    CLAY({ .id = CLAY_ID("OuterContainer"),
        .backgroundColor = {43, 41, 51, 255 },
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = {
              .height = CLAY_SIZING_GROW(0),
              .width = CLAY_SIZING_GROW(0)
            },
            .padding = CLAY_PADDING_ALL(16),
            .childGap = 16
        }
    }) {}

  return Clay_EndLayout();
}
