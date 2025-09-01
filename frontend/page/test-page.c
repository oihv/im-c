#include "../clay.h"
#include "../components/textbox.c"
#include <stdio.h>
#include "../components/textbox.h"

Clay_RenderCommandArray ClayIMCTest_CreateLayout(char* buffer, int* frameCount) {
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
  }) {
    Component_TextBoxData textData = {
       .buffer = buffer,
       .frameCount = frameCount,
       .max_len = 254,
       .textConfig = (Clay_TextElementConfig) {
.fontId = FONT_ID_BODY_16,
                                         .fontSize = 16,
                                         .textColor = {255, 255, 255, 255},
                                         .wrapMode = CLAY_TEXT_WRAP_WORDS}
       };
    
    renderTextBox(&textData);
  }

  return Clay_EndLayout();
}
