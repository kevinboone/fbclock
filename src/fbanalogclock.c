/*==========================================================================

  fbclock 
  fbanalogclock.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

  This file contains the main body of the program.

==========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <signal.h>
#include <math.h>
#include "feature.h" 
#include "fbanalogclock.h" 
#include "string.h" 
#include "file.h" 
#include "list.h" 
#include "framebuffer.h"
#include "region.h"

static const double TWOPI = 2.0 * M_PI;

/*==========================================================================

  draw_clock_in_region

==========================================================================*/
static void draw_hand (Region *r, double angle, 
     int cx, int cy, int thickness, int l, BYTE cr, BYTE cg, BYTE cb)
  {
  int lx = l * sin (angle);
  int ly = l * cos (angle);
  int lxh = l / 10 * sin (angle + M_PI);
  int lyh = l / 10 * cos (angle + M_PI);
  region_draw_hollow_line (r, cx + lxh, cy - lyh, 
     cx + lx, cy - ly, thickness, cr, cg, cb);
  }


/*==========================================================================

  draw_date

==========================================================================*/
static void draw_date (Region *r, int l, int cx, int cy, 
     BYTE cr, BYTE cg, BYTE cb, const BitmapFont *font)
  {
  int text_height = font->height;
  int text_width = font->width;
  time_t t = time (NULL);
  const struct tm *tm = localtime (&t);

  char s[20];
  strftime (s, sizeof (s) - 1, "%a %b %d", tm);
  int xo = strlen (s) * text_width / 2; 
  region_draw_bitmap_text (r, font, s, cx - xo, 
      cy - 2 * text_height, cr, cg, cb); 
  }


/*==========================================================================

  draw_numerals 

==========================================================================*/
static void draw_numerals (Region *r, int l, int cx, int cy, 
     BYTE cr, BYTE cg, BYTE cb, const BitmapFont *font)
  {
  int text_height = font->height;
  int text_width = font->width;
  for (int i = 0; i < 12; i++)
    {
    double angle = (double) (i + 1) / 12 * TWOPI;
    int lx = (l - text_height) * sin (angle);
    int ly = (l - text_height) * cos (angle);
    char s[10];
    sprintf (s, "%d", i + 1); 
    int chrs = (i > 9) ? 2 : 1;
    int xo = chrs * text_width / 2; 
    region_draw_bitmap_text (r, font, s, cx + lx - xo, 
      cy - ly, cr, cg, cb); 
    }
  }

/*==========================================================================

  select_analog_font 

==========================================================================*/

const BitmapFont *select_analog_font (int radius)
  {
  if (radius < 80) return &font8;
  if (radius < 140) return &font12;
  return &font20;
  }


/*==========================================================================

  draw_clock_in_region

==========================================================================*/
void program_draw_clock_in_region (Region *r, BOOL seconds, BOOL date)
  {
  int width = region_get_width (r);
  int height = region_get_height (r); 

  time_t t = time (NULL);
  const struct tm *tm = localtime (&t);
  int hr = tm->tm_hour;
  int min = tm->tm_min;
  int sec = tm->tm_sec;

  BYTE cr = 255, cg = 255, cb = 255;
 
  // lm is the maximum extent of the drawing area -- the smallest
  //  of the width and height

  int lm;
  if (height < width)
    lm = height / 2;
  else
    lm = width / 2;
   
  // cx, cy are the centre of the drawing area

  int cx = width / 2;
  int cy = height / 2;

  const BitmapFont *font = select_analog_font (lm);

  draw_numerals (r, lm, cx, cy, cr, cg, cb, font);
  if (date)
    draw_date (r, lm, cx, cy, cr, cg, cb, font);

  int lm_hands = lm - 2 * font->height;

  if (seconds)
    draw_hand (r, (double)sec / 60 * TWOPI, cx, cy, 1, 
      lm_hands, cr, cg, cb); // sec
  draw_hand (r, (double)min / 60 * TWOPI, cx, cy, 5, 
    lm_hands * 9 / 10, cr, cg, cb); // min
  draw_hand (r, ((double)hr / 12 + (double)min / 60 / 12) * TWOPI, 
    cx, cy, 10, lm_hands * 6 / 10, cr, cg, cb); // hour
  }


