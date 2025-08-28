#include "../clay.h"
// #include "../renderers/raylib/clay_renderer_raylib.c"
#include "../renderers/raylib/raylib.h"
#include "../shared-layouts/mainpage.h"
#include "stdio.h"

void HandleTextBoxInteraction(Clay_ElementId elementId,
                              Clay_PointerData pointerData, intptr_t userData) {
  // SidebarClickData *clickData = (SidebarClickData*)userData;
  bool *isFocus = (bool *)userData;
  // If this button was clicked
  // TODO: Implement how to lose focus when other object is clicked
  if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    *isFocus = true;
  }
}

// TODO: simplify data passing in a struct, pass textconfig too for
// customization by caller
void renderTextBox(char *buffer, int *frameCount) {
  CLAY({.layout = {.padding = {16, 16, 8, 8},
                   .sizing = {.width = CLAY_SIZING_GROW(0),
                              .height = CLAY_SIZING_GROW(0, 50)}},
        .cornerRadius = CLAY_CORNER_RADIUS(5),
        .backgroundColor = {205, 205, 205, 205}}) {
    static int len = 0;
    size_t blink_len;
    static bool isFocus = true; // for testing
    int key = 0;

    if (isFocus) {
      key = GetCharPressed();
      (*frameCount)++;

      while (key > 0) {
        if (len < MAX_INPUT_CHAR) {
          if (buffer[len - 1] == '_')
            len--;
          buffer[len] = key;
          buffer[len + 1] = '\0';
          len++;
        }

        key = GetCharPressed();
        printf("\"%s\" len: %d\n", buffer, len);
      }

      // Blinking underscore at the end
      blink_len = len + 1;
      // if (len == 0 || buffer[len] == '\0' && (buffer[len - 1] != '_' &&
      // buffer[len - 1] != ' ')) { printf("%c\n", buffer[len - 1]);
      //   len++;
      buffer[blink_len] = '\0';
      // }
      if (((*(frameCount) / 20) % 2) == 0) {
        buffer[blink_len - 1] = '_';
      } else if (buffer[blink_len - 1] == '_') {
        buffer[blink_len - 1] = '\0';
      }

      key = GetKeyPressed();
      while (key > 0) {
        // Implement 'ctrl+backspace'
        // TODO! implement this not only for spaces but other chars as wel
        if (IsKeyDown(KEY_LEFT_CONTROL) && key == KEY_BACKSPACE) {
          // If right at the back of the cursor is a space, delete it too,
          if (len > 0 && buffer[len - 1] == '\0')
            len--;
          // Delete until space
          while (len > 0) {
            if (buffer[len - 1] == ' ')
              break;
            len--;
          }
          buffer[len] = '\0';
          // Implement 'backspace'
        } else if (key == KEY_BACKSPACE) {
          if (len > 0) {
            if (buffer[len - 1] == '_')
              len--;
            buffer[len - 1] = '\0';
            len--;
          }
        }

        // TODO! implement enter message, with a separate logic so it can be
        // used by button too
        if (key == KEY_ENTER) {
          buffer[0] = '\0';
          len = 0;
        }

        key = GetKeyPressed();
      }
    }

    Clay_OnHover(HandleTextBoxInteraction, (intptr_t)&isFocus);

    Clay_String buf_str = (Clay_String){
        .isStaticallyAllocated = false, .length = blink_len, .chars = buffer};

    CLAY_TEXT(buf_str, CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                         .fontSize = 34,
                                         .textColor = {255, 255, 255, 255},
                                         .wrapMode = CLAY_TEXT_WRAP_WORDS}));
  }
}
