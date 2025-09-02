#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <stddef.h>
#include <stdint.h>
#include "../clay.h"

typedef struct Component_TextBoxData {
  Clay_String id;
  char* buffer;
  size_t maxLen;
  int* frameCount;
  Clay_String placeholder;
  Clay_TextElementConfig textConfig;

} Component_TextBoxData;
#endif
