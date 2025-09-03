#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <stddef.h>
#include <stdint.h>
#include "../clay.h"

typedef struct {
  bool* isFocus;
  size_t focus_len;
  bool* focusList;
} TextBoxEventData;

typedef struct {
  Clay_String id;
  char* buffer;
  size_t maxLen;
  size_t* len;
  uint16_t* frameCount;
  Clay_String placeholder;
  Clay_TextElementConfig textConfig;
  TextBoxEventData eventData;
} Component_TextBoxData;

void resetTextBoxFocus(bool* focusList, size_t len);

void renderTextBox(Component_TextBoxData* data);
#endif
