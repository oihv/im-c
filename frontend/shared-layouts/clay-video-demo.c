#include "../clay.h"
#include <stdlib.h>
#include "mainpage.h"
#include <string.h>
#include "../renderers/raylib/raylib.h"
#include "../components/textbox.h"

void RenderHeaderButton(Clay_String text)
{
    CLAY({.layout = {.padding = {16, 16, 8, 8}},
          .backgroundColor = {140, 140, 140, 255},
          .cornerRadius = CLAY_CORNER_RADIUS(5)})
    {
        CLAY_TEXT(text, CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                          .fontSize = 16,
                                          .textColor = {255, 255, 255, 255}}));
    }
}

void RenderDropdownMenuItem(Clay_String text)
{
    CLAY({.layout = {.padding = CLAY_PADDING_ALL(16)}})
    {
        CLAY_TEXT(text, CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                          .fontSize = 16,
                                          .textColor = {255, 255, 255, 255}}));
    }
}

// Add this function to calculate responsive font size
int GetResponsiveFontSize(int baseFontSize, int windowWidth, int windowHeight) {
    // Base calculation on window width (you can adjust the formula)
    float scaleFactor = (float)windowWidth / 1200.0f; // 1200px as reference width
    
    // Clamp the scale factor between 0.8 and 1.5 for reasonable limits
    if (scaleFactor < 0.8f) scaleFactor = 0.8f;
    if (scaleFactor > 1.5f) scaleFactor = 1.5f;
    
    return (int)(baseFontSize * scaleFactor);
}

typedef struct
{
    Clay_String title;
    Clay_String contents;
} Document;

typedef struct
{
    Document *documents;
    uint32_t length;
} DocumentArray;

Document documentsRaw[5];

DocumentArray documents = {
    .length = 5,
    .documents = documentsRaw};

typedef struct
{
    Clay_String sender;
    Clay_String text;
    bool isSender; // true = user, false = bot/system
} ChatMessage;

#define MAX_MESSAGES 100
ChatMessage chatMessages[MAX_MESSAGES];
int chatMessageCount = 0;

static inline Clay_String ClayStr(const char *s)
{
    return (Clay_String){s, (int)strlen(s)}; // positional init: {ptr, length}
}

void AddBotReply(const char *replyText)
{
    if (chatMessageCount < MAX_MESSAGES)
    {
        chatMessages[chatMessageCount].sender = CLAY_STRING("Bot");
        chatMessages[chatMessageCount].text = (Clay_String){
            .chars = replyText,
            .length = (int)strlen(replyText)};
        chatMessages[chatMessageCount].isSender = false; // bot/system
        chatMessageCount++;
    }
}

void AddUserMessage(const char *userText)
{
    if (chatMessageCount < MAX_MESSAGES)
    {
        chatMessages[chatMessageCount].sender = CLAY_STRING("Ben"); // or use ClayStr("Ben") if dynamic
        chatMessages[chatMessageCount].text = (Clay_String){
            .chars = userText,
            .length = (int)strlen(userText)};
        chatMessages[chatMessageCount].isSender = true; // user
        chatMessageCount++;
    }
}

typedef struct
{
    intptr_t offset;
    intptr_t memory;
} ClayVideoDemo_Arena;

typedef struct
{
    int32_t selectedDocumentIndex;
    float yOffset;
    ClayVideoDemo_Arena frameArena;
} ClayVideoDemo_Data;

typedef struct
{
    int32_t requestedDocumentIndex;
    int32_t *selectedDocumentIndex;
} SidebarClickData;

typedef struct
{
    int dummy; // placeholder for future use
} SendClickData;

void HandleSendMessage(
    Clay_ElementId elementId,
    Clay_PointerData pointerData,
    intptr_t userData)
{
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
    {
        if (chatMessageCount < MAX_MESSAGES)
        {
            chatMessages[chatMessageCount++] = (ChatMessage){
                .sender = CLAY_STRING("Ben"),
                .text = CLAY_STRING("Hello, this is a test message!")};
        }
    }
}

void HandleSidebarInteraction(
    Clay_ElementId elementId,
    Clay_PointerData pointerData,
    intptr_t userData)
{
    SidebarClickData *clickData = (SidebarClickData *)userData;
    // If this button was clicked
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
    {
        if (clickData->requestedDocumentIndex >= 0 && clickData->requestedDocumentIndex < documents.length)
        {
            // Select the corresponding document
            *clickData->selectedDocumentIndex = clickData->requestedDocumentIndex;
        }
    }
}

void HandleSendInteraction(
    Clay_ElementId elementId,
    Clay_PointerData pointerData,
    intptr_t userData)
{
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
    {
        AddUserMessage("Hello, this is a test message!");
        AddBotReply("Got your message?"); // <-- demo bot reply
    }
}

ClayVideoDemo_Data ClayVideoDemo_Initialize()
{

    ClayVideoDemo_Data data = {
        .frameArena = {.memory = (intptr_t)malloc(1024)}};
    return data;
}

Clay_RenderCommandArray ClayVideoDemo_CreateLayout(ClayVideoDemo_Data *data)
{
    data->frameArena.offset = 0;

    // Get current window size for responsive fonts
    int windowWidth = GetScreenWidth();   // Raylib function
    int windowHeight = GetScreenHeight(); // Raylib function
    
    // Calculate responsive font sizes
    int bodyFontSize = GetResponsiveFontSize(16, windowWidth, windowHeight);
    int titleFontSize = GetResponsiveFontSize(20, windowWidth, windowHeight);

    Clay_BeginLayout();

    Clay_Sizing layoutExpand = {
        .width = CLAY_SIZING_GROW(0),
        .height = CLAY_SIZING_GROW(0)};

    Clay_Color contentBackgroundColor = {90, 90, 90, 255};

    // Build UI here
    CLAY({.id = CLAY_ID("OuterContainer"),
          .backgroundColor = {43, 41, 51, 255},
          .layout = {
              .layoutDirection = CLAY_TOP_TO_BOTTOM,
              .sizing = layoutExpand,
              .padding = CLAY_PADDING_ALL(16),
              .childGap = 16}})
    {
        // Child elements go inside braces
        CLAY({.id = CLAY_ID("LowerContent"),
              .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
                         .sizing = {
                             .width = CLAY_SIZING_GROW(1),
                             .height = CLAY_SIZING_GROW(0)},
                         .childGap = 16}})
        {
            CLAY({.id = CLAY_ID("Sidebar"),
                  .backgroundColor = contentBackgroundColor,
                  .cornerRadius = CLAY_CORNER_RADIUS(8),
                  .layout = {
                      .layoutDirection = CLAY_TOP_TO_BOTTOM,
                      .padding = CLAY_PADDING_ALL(16),
                      .childGap = 16,
                      .sizing = {
                          .width = CLAY_SIZING_FIXED(150),
                          .height = CLAY_SIZING_GROW(0)}}})
            {
                // --- IP Address panel ---
                CLAY({.id = CLAY_ID("IpAddress"),
                      .backgroundColor = {120, 120, 120, 255},
                      .cornerRadius = CLAY_CORNER_RADIUS(8),
                      .layout = {
                          .padding = CLAY_PADDING_ALL(12),
                          .sizing = {.width = CLAY_SIZING_GROW(0)}}})
                {
                    CLAY_TEXT(CLAY_STRING("IP Address: 127.0.0.1:1000"),
                              CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16, .fontSize = bodyFontSize, .textColor = {255, 255, 255, 255}}));
                }

                // --- Username panel ---
                CLAY({.id = CLAY_ID("Username"),
                      .backgroundColor = {120, 120, 120, 255},
                      .cornerRadius = CLAY_CORNER_RADIUS(8),
                      .layout = {
                          .padding = CLAY_PADDING_ALL(12),
                          .sizing = {.width = CLAY_SIZING_GROW(0)}}})
                {
                    CLAY_TEXT(CLAY_STRING("Username: Ben"),
                              CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16, .fontSize = bodyFontSize, .textColor = {255, 255, 255, 255}}));
                }

                // --- Spacer to push logout button down ---
                CLAY({.layout = {.sizing = {.height = CLAY_SIZING_GROW(1)}}});

                // --- Logout button ---
                Clay_Color logoutBase = {120, 120, 120, 255};
                Clay_Color logoutHover = {140, 140, 140, 255};
                Clay_Color logoutColor = logoutBase;
                if (Clay_Hovered())
                    logoutColor = logoutHover;

                CLAY({.id = CLAY_ID("LogoutButton"),
                      .backgroundColor = logoutColor, // Use hover color
                      .cornerRadius = CLAY_CORNER_RADIUS(8),
                      .layout = {
                          .padding = CLAY_PADDING_ALL(12),
                          .sizing = {.width = CLAY_SIZING_GROW(0)},
                          .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}}})
                {
                    CLAY_TEXT(CLAY_STRING("Logout"),
                              CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16, .fontSize = bodyFontSize, .textColor = {255, 255, 255, 255}}));
                }
            }

            // 📌 New vertical container for MainContent + BottomBar
            CLAY({.id = CLAY_ID("RightPane"),
                  .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                             .sizing = {
                                 .width = CLAY_SIZING_GROW(1),
                                 .height = CLAY_SIZING_GROW(0)},
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

                        CLAY({.layout = {
                                  .layoutDirection = CLAY_LEFT_TO_RIGHT,
                                  .sizing = {.width = CLAY_SIZING_GROW(1)},
                                  .childAlignment = {.x = isUser ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT}}})
                        {
                            CLAY({.layout = {
                                      .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                      .padding = CLAY_PADDING_ALL(10),
                                      .sizing = {.width = CLAY_SIZING_FIT(1)}},
                                  .backgroundColor = isUser ? (Clay_Color){0, 120, 215, 255} : // User bubble (blue)
                                                         (Clay_Color){120, 120, 120, 255},     // Bot bubble (gray)

                                  // Rounded corners like WhatsApp
                                  .cornerRadius = isUser ? (Clay_CornerRadius){.topLeft = 12, .topRight = 12, .bottomLeft = 12, .bottomRight = 2} : (Clay_CornerRadius){.topLeft = 12, .topRight = 12, .bottomLeft = 2, .bottomRight = 12}})
                            {
                                CLAY_TEXT(message.text, CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                                          .fontSize = bodyFontSize,
                                                                          .textColor = {255, 255, 255, 255}}));
                            }
                        }
                    }
                }

                CLAY({.id = CLAY_ID("BottomBar"),
                      .layout = {
                          .layoutDirection = CLAY_LEFT_TO_RIGHT, // Add this line
                          .sizing = {
                              .height = CLAY_SIZING_FIXED(60),
                              .width = CLAY_SIZING_GROW(1)},
                          .padding = CLAY_PADDING_ALL(16), // Change this line
                          .childGap = 16,
                          .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}},
                      .backgroundColor = contentBackgroundColor,
                      .cornerRadius = CLAY_CORNER_RADIUS(8)})
                {
                    CLAY({
                        .id = CLAY_ID("InputBox"),
                        .layout = {.sizing = {
                                       .width = CLAY_SIZING_GROW(1),
                                   }},
                    })
                    {
                        CLAY({.layout = {.padding = {16, 16, 8, 8},
                                         .sizing = {.width = CLAY_SIZING_GROW(0)}},
                              .backgroundColor = {140, 140, 140, 255},
                              .cornerRadius = CLAY_CORNER_RADIUS(5)})
                        {
                            // Add hover colors for input box
                            Clay_Color inputBase = {140, 140, 140, 255};
                            Clay_Color inputHover = {160, 160, 160, 255};
                            Clay_Color inputColor = inputBase;
                            if (Clay_Hovered())
                                inputColor = inputHover;

                            CLAY({.layout = {.padding = CLAY_PADDING_ALL(3),
                                             .sizing = {.width = CLAY_SIZING_GROW(0)}},
                                  .backgroundColor = inputColor,
                                  .cornerRadius = CLAY_CORNER_RADIUS(5)})
                            {
                                CLAY_TEXT(CLAY_STRING("Enter a message.."), CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                                                              .fontSize = bodyFontSize,
                                                                                              .textColor = {200, 200, 200, 255}}));
                            }
                            // Component_TextBoxData username_data = (Component_TextBoxData) {
                            //    .id = CLAY_STRING("username_textbox"),
                            //    .textConfig = (Clay_TextElementConfig) {
                            //                  .fontId = FONT_ID_BODY_16,
                            //                  .fontSize = 16,
                            //                  .textColor = {255, 255, 255, 255}},
                            //    // .buffer = data->username_buf,
                            //    // .frameCount = &(data->frameCount),
                            //    // .len = &data->username_len,
                            //    // .maxLen = sizeof(data->username_buf),
                            //    // .placeholder = CLAY_STRING("Enter your username:"),
                            //    // .eventData = (TextBoxEventData) {
                            //    //   .focusList = data->focusList,
                            //    //   .isFocus = data->focusList,
                            //    //   .focus_len = 2,
                            //    }
                            //  };

                            // renderTextBox(&username_data);
                        }
                    }
                    Clay_Color base = {140, 140, 140, 255};
                    Clay_Color hover = {160, 160, 160, 255};

                    Clay_Color btnColor = base;
                    if (Clay_Hovered())
                        btnColor = hover;

                    CLAY({.id = CLAY_ID("SendButton"),
                          .layout = {.sizing = {.width = CLAY_SIZING_FIXED(70),
                                                .height = CLAY_SIZING_FIXED(30)},
                                     .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER},
                                     .padding = CLAY_PADDING_ALL(6)},
                          .backgroundColor = btnColor,
                          .cornerRadius = CLAY_CORNER_RADIUS(5)})
                    {
                        SendClickData *clickData = (SendClickData *)(data->frameArena.memory + data->frameArena.offset);
                        *clickData = (SendClickData){0};
                        data->frameArena.offset += sizeof(SendClickData);

                        // Still use OnHover, but inside your handler check if the mouse was clicked
                        Clay_OnHover(HandleSendInteraction, (intptr_t)clickData);

                        CLAY_TEXT(CLAY_STRING("Send"), CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                                         .fontSize = bodyFontSize,
                                                                         .textColor = {255, 255, 255, 255}}));
                    }
                }

            } // End RightPane
        }
    }

    // if (mouseButtonDown(0) && Clay_PointerOver(Clay_GetElementId(CLAY_STRING("ProfilePicture")))) {
    //     // Handle profile picture clicked
    // }
    Clay_RenderCommandArray renderCommands = Clay_EndLayout();

    for (int32_t i = 0; i < renderCommands.length; i++)
    {
        Clay_RenderCommandArray_Get(&renderCommands, i)->boundingBox.y += data->yOffset;
    }

    return renderCommands;
}
