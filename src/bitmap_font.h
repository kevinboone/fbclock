/*============================================================================

  fbclock 
  bitmap_fonts.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include "defs.h"

typedef struct _BitmapFont
  {
  const uint8_t *table;
  int width;
  int height; 
  } BitmapFont;

extern BitmapFont font8;
extern BitmapFont font12;
extern BitmapFont font16;
extern BitmapFont font20;
extern BitmapFont font24;

BEGIN_DECLS

END_DECLS

