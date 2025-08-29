#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <stddef.h>
#include <stdint.h>
#include "../clay.h"

typedef struct Component_TextBoxData {
  char *buffer;
  size_t max_len;
  int *frameCount;
  Clay_TextElementConfig textConfig;

} Component_TextBoxData;
#endif
