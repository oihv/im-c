            // 📌 New vertical container for MainContent + BottomBar
            CLAY({.id = CLAY_ID("RightPane"),
                  .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                             .sizing = {
                                 .width = CLAY_SIZING_GROW(1),
                                 .height = CLAY_SIZING_GROW(1)},
                             .childGap = 16}})
            {
                CLAY({.id = CLAY_ID("MainContent"),
                      .backgroundColor = contentBackgroundColor,
                      .cornerRadius = CLAY_CORNER_RADIUS(8),
                      .clip = {.vertical = true, .childOffset = Clay_GetScrollOffset()},
                      .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                                 .childGap = 16,
                                 .padding = CLAY_PADDING_ALL(16),
                                 .sizing = {
                                     .width = CLAY_SIZING_GROW(1),
                                     .height = CLAY_SIZING_GROW(1)}}})

                {
                    for (int i = chatMessageCount - 1; i >= 0; i--)
                    {
                        ChatMessage message = chatMessages[i];
                        bool isUser = message.isSender;

                        // Container row for alignment
                        CLAY({.layout = {
                                  .layoutDirection = CLAY_LEFT_TO_RIGHT,
                                  .sizing = {.width = CLAY_SIZING_GROW(0)}, // ✅ use GROW(0) for row, not full width
                                  .childAlignment = {.x = isUser ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT}}})
                        {
                            // Bubble itself
                            CLAY({.layout = {
                                      .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                      .padding = CLAY_PADDING_ALL(10),
                                      .sizing = {
                                          .width = CLAY_SIZING_PERCENT(0.7f) // Max 70% of available width
                                      }},
                                  .backgroundColor = isUser ? (Clay_Color){0, 120, 215, 255} : (Clay_Color){120, 120, 120, 255},
                                  .cornerRadius = isUser ? (Clay_CornerRadius){.topLeft = 12, .topRight = 12, .bottomLeft = 12, .bottomRight = 2} : (Clay_CornerRadius){.topLeft = 12, .topRight = 12, .bottomLeft = 2, .bottomRight = 12}})
                            {
                                CLAY_TEXT(message.text, CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                                          .fontSize = 16,
                                                                          .textColor = {255, 255, 255, 255},
                                                                          .wrapMode = CLAY_TEXT_WRAP_WORDS})); // Enable word wrapping
                            }
                        }
                    }
                }

                CLAY({.id = CLAY_ID("BottomBar"),
                      .layout = {
                          .sizing = {
                              .height = CLAY_SIZING_FIXED(60),
                              .width = CLAY_SIZING_GROW(1)},
                          .padding = {16, 16, 0, 0},
                          .childGap = 16,
                          .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}},
                      .backgroundColor = contentBackgroundColor,
                      .cornerRadius = CLAY_CORNER_RADIUS(8)})
                {
                    CLAY({
                        .id = CLAY_ID("InputBox"),
                        .layout = {.sizing = {
                                       .width = CLAY_SIZING_GROW(120),
                                   }},
                    })
                    {
                        CLAY({.layout = {.padding = {16, 16, 8, 8},
                                         .sizing = {.width = CLAY_SIZING_GROW(0)}},
                              .backgroundColor = {140, 140, 140, 255},
                              .cornerRadius = CLAY_CORNER_RADIUS(5)})
                        {
                            CLAY_TEXT(CLAY_STRING("Enter a message.."), CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                                                          .fontSize = 16,
                                     