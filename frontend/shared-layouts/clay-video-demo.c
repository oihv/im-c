#include "../clay.h"
#include <stdlib.h>
#include "mainpage.h"

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

ClayVideoDemo_Data ClayVideoDemo_Initialize()
{

    ClayVideoDemo_Data data = {
        .frameArena = {.memory = (intptr_t)malloc(1024)}};
    return data;
}

Clay_RenderCommandArray ClayVideoDemo_CreateLayout(ClayVideoDemo_Data *data)
{
    data->frameArena.offset = 0;

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
                  .layout = {
                      .layoutDirection = CLAY_TOP_TO_BOTTOM,
                      .padding = CLAY_PADDING_ALL(16),
                      .childGap = 8,
                      .sizing = {
                          .width = CLAY_SIZING_FIXED(200),
                          .height = CLAY_SIZING_GROW(0)}}})
            {
                for (int i = 0; i < documents.length; i++)
                {
                    Document document = documents.documents[i];
                    Clay_LayoutConfig sidebarButtonLayout = {
                        .sizing = {.width = CLAY_SIZING_GROW(0)},
                        .padding = CLAY_PADDING_ALL(16)};

                    if (i == data->selectedDocumentIndex)
                    {
                        CLAY({.layout = sidebarButtonLayout,
                              .backgroundColor = {120, 120, 120, 255},
                              .cornerRadius = CLAY_CORNER_RADIUS(8)})
                        {
                            CLAY_TEXT(document.title, CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                                        .fontSize = 20,
                                                                        .textColor = {255, 255, 255, 255}}));
                        }
                    }
                    else
                    {
                        SidebarClickData *clickData = (SidebarClickData *)(data->frameArena.memory + data->frameArena.offset);
                        *clickData = (SidebarClickData){.requestedDocumentIndex = i, .selectedDocumentIndex = &data->selectedDocumentIndex};
                        data->frameArena.offset += sizeof(SidebarClickData);
                        CLAY({.layout = sidebarButtonLayout, .backgroundColor = (Clay_Color){120, 120, 120, Clay_Hovered() ? 120 : 0}, .cornerRadius = CLAY_CORNER_RADIUS(8)})
                        {
                            Clay_OnHover(HandleSidebarInteraction, (intptr_t)clickData);
                            CLAY_TEXT(document.title, CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                                        .fontSize = 20,
                                                                        .textColor = {255, 255, 255, 255}}));
                        }
                    }
                }
            }

            CLAY({.id = CLAY_ID("MainContent"),
                  .backgroundColor = contentBackgroundColor,
                  .clip = {.vertical = true, .childOffset = Clay_GetScrollOffset()},
                  .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                             .childGap = 16,
                             .padding = CLAY_PADDING_ALL(16),
                             .sizing = {
                                 .width = CLAY_SIZING_GROW(1),
                                 .height = CLAY_SIZING_GROW(0)}}})
            {
                Document selectedDocument = documents.documents[data->selectedDocumentIndex];
                CLAY_TEXT(selectedDocument.title,
                          CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                            .fontSize = 24,
                                            .textColor = COLOR_WHITE}));
                CLAY_TEXT(selectedDocument.contents,
                          CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                            .fontSize = 24,
                                            .textColor = COLOR_WHITE}));
            }
        }

        CLAY({.id = CLAY_ID("BottomBar"),
              .layout = {
                  .sizing = {
                      .height = CLAY_SIZING_FIXED(60),
                      .width = CLAY_SIZING_GROW(0)},
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
                                                                                  .textColor = {255, 255, 255, 255}}));
                }
            }
            CLAY({.id = CLAY_ID("SendButton"),
                  .layout = {.sizing = {.width = CLAY_SIZING_FIXED(60)},
                             .childAlignment = {.x = CLAY_ALIGN_X_RIGHT}}})
            {
                RenderHeaderButton(CLAY_STRING("Send"));
            }
        }
    }
    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    for (int32_t i = 0; i < renderCommands.length; i++)
    {
        Clay_RenderCommandArray_Get(&renderCommands, i)->boundingBox.y +=
            data->yOffset;
    }
    return renderCommands;
}
