/*============================================================================

  fbclock 
  region.c
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <stdarg.h>
#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include "string.h" 
#include "defs.h" 
#include "log.h" 
#include "framebuffer.h" 
#include "region.h" 
#include "bitmap_font.h" 

// Bytes per pixel
#define BPP 3

static float HALFPI = M_PI / 2.0; 

struct _Region
  {
  int w;
  int h;
  BYTE *data;
  }; 


/*==========================================================================
  
  math helper functions

  These are all used by the line anti-aliasing functions. I have defined
  them as 'static inline' in the hope that gcc will do the right thing
  and write them straight into the code
    
*==========================================================================*/
static inline float absolute (float x)
  {
  if (x < 0) 
    return -x;
  else
    return x;
  }

static inline void swap (int *x, int *y)
  {
  int t = *y;
  *y = *x;
  *x = t;
  }

static inline float ipart (float x) 
  { 
  return floor (x); 
  } 
  
static inline float rnd (float x) 
  { 
  return ipart (x + 0.5);
  } 
  
static inline float fpart (float x) 
  { 
  return x - floor (x);
  } 
  
static inline float rfpart (float x) 
  { 
  return 1 - fpart (x); 
  } 
  


/*==========================================================================
  region_create
*==========================================================================*/
Region *region_create (int w, int h)
  {
  LOG_IN
  Region *self = malloc (sizeof (Region));
  self->w = w;
  self->h = h;
  self->data = malloc (w * h * BPP);
  LOG_OUT 
  return self;
  }

/*==========================================================================
  region_clone
*==========================================================================*/
Region *region_clone (const Region *other)
  {
  LOG_IN
  Region *self = region_create (other->w, other->h);

  int size = self->w * self->h * BPP;
  memcpy (self->data, other->data, size); 
 
  LOG_OUT
  return self;
  }

/*==========================================================================
  region_set_pixel
*==========================================================================*/
void region_set_pixel (Region *self, int x, int y, 
      BYTE r, BYTE g, BYTE b)
  {
  if (x >= 0 && x < self->w && y >= 0 && y < self->h)
    {
    int index24 = (y * self->w + x) * BPP;
    self->data [index24++] = b;
    self->data [index24++] = g;
    self->data [index24] = r;
    }
  }

/*==========================================================================
  region_set_pixel_t

  Set a pixel with a specific 'brightness'. This is just a helper to
  void reproducing all the per-channel brightness code repeatedly 
  elsewhere
*==========================================================================*/
void region_set_pixel_t (Region *self, int x, int y, 
      BYTE r, BYTE g, BYTE b, float t)
  {
  b = (BYTE) (t * (float)b);
  g = (BYTE) (t * (float)g);
  r = (BYTE) (t * (float)r);
  if (x > 0 && x < self->w && y > 0 && y < self->h)
    {
    int index24 = (y * self->w + x) * BPP;
    self->data [index24 + 0] = b;
    self->data [index24 + 1] = g;
    self->data [index24 + 2] = r;
    }
  }


/*==========================================================================
  region_fill_rect
  x2,y2 point is _excluded_
*==========================================================================*/
void region_fill_rect (Region *self, int x1, int y1,
      int x2, int y2, BYTE r, BYTE g, BYTE b)
  {
  LOG_IN
  if (x1 > x2) { int t = x1; x2 = x1; x1 = t; }
  if (y1 > y2) { int t = y1; y2 = y1; y1 = t; }
  for (int y = y1; y < y2; y++)
    {
    for (int x = x1; x < x2; x++)
      {
      region_set_pixel (self, x, y, r, g, b);
      }
    }
  LOG_OUT
  }

/*==========================================================================
  region_destroy
*==========================================================================*/
void region_destroy (Region *self)
  {
  LOG_IN
  if (self)
    {
    if (self->data) free (self->data);
    free (self); 
    }
  LOG_OUT
  }


/*==========================================================================
  region_to_fb
*==========================================================================*/
void region_to_fb (const Region *self, FrameBuffer *fb, int x1, int y1)
  {
  LOG_IN
  int w_in = self->w;
  int h_in = self->h;
  BYTE *data = framebuffer_get_data (fb);
  int w_out = framebuffer_get_width (fb);
  for (int y = 0; y < h_in; y++)
    {
    int xp = x1;
    int linestart24 = y * w_in;
    int linestart32 = (y + y1) * w_out;
    for (int x = 0; x < w_in; x++)
      {
      int index24 = (linestart24 + x) * BPP;
      int index32 = (linestart32 + x + x1) * 4;
      BYTE b = self->data [index24++];
      BYTE g = self->data [index24++];
      BYTE r = self->data [index24];
      data [index32] = b;
      data [index32+1] = g;
      data [index32+2] = r;
      xp++;
      }
    }
  LOG_OUT
  }


/*==========================================================================

  region_from_fb

  The region should already be intialized, and have the desired
  sizes

*==========================================================================*/
void region_from_fb (Region *self, const FrameBuffer *fb, int x1, int y1)
  {
  LOG_IN
  int w_in = self->w;
  int h_in = self->h;
  for (int y = 0; y < h_in; y++)
    {
    int yp = y + y1;
    int xp = x1;
    int linestart = y * w_in;
    for (int x = 0; x < w_in; x++)
      {
      BYTE r, g, b;
      framebuffer_get_pixel (fb, xp, yp, &r, &g, &b);
      int index24 = (linestart + x) * BPP;
      self->data [index24++] = b;
      self->data [index24++] = g;
      self->data [index24] = r;
      xp++;
      }
    }
  LOG_OUT
  }

/*==========================================================================

  region_darken

  Darken to the specified percentage of original value

*==========================================================================*/
void region_darken (Region *self, int percent)
  {
  LOG_IN
  int l = self->w * self->h * BPP;
  for (int i = 0; i < l; i++)
    {
    self->data[i] = self->data[i] * percent / 100;
    }
  LOG_OUT
  }

/*==========================================================================

  region_draw_bitmap_char

*==========================================================================*/
void region_draw_bitmap_char (Region *self, const BitmapFont *bf, 
       char c, int x, int y, int r, int g, int b)
  {
  LOG_IN
  if (x < self->w && y < self->h)
    {
    if (c < ' ' || c > '~') c = '?';
    int char_offset = (c - ' ') * bf->height * 
         (bf->width / 8 + (bf->width % 8 ? 1 : 0));
    const uint8_t *ptr = &bf->table[char_offset];
    for (int page = 0; page < bf->height; page++)
      {
      for (int column = 0; column < bf->width; column++)
        {
        if (*ptr & (0x80 >>(column % 8)))
          region_set_pixel (self, x + column, y + page, r, g, b);
        if (column % 8 == 7) ptr++;
        }
      if (bf->width % 8 != 0) ptr++;
      }
    }
  LOG_OUT
  }


/*==========================================================================

  region_draw_bitmap_text

*==========================================================================*/
void region_draw_bitmap_text (Region *self, const BitmapFont *bf, 
       const char *text,
       int x, int y, int r, int g, int b)
  {
  LOG_IN
  int l = strlen (text);
  for (int i = 0; i < l; i++)
    {
    char c = text[i];
    region_draw_bitmap_char (self, bf, c, x + i * bf->width, y, r, g, b);
    }
  LOG_OUT
  }


/*==========================================================================
  region_get_width
*==========================================================================*/
int region_get_width (const Region *self)
  {
  return self->w;
  }

/*==========================================================================
  region_get_height
*==========================================================================*/
int region_get_height (const Region *self)
  {
  return self->h;
  }


/*==========================================================================
  
  region_draw_line_one_pixel
  https://en.wikipedia.org/wiki/Xiaolin_Wu%27s_line_algorithm

*==========================================================================*/
void region_draw_line_one_pixel (Region *self, int x0, int y0, 
          int x1, int y1, BYTE r, BYTE g, BYTE b)
  {
  LOG_IN

  BOOL steep = absolute (y1 - y0) > absolute (x1 - x0); 
  
  if (steep) 
    { 
    swap (&x0, &y0); 
    swap (&x1, &y1); 
    } 
  if (x0 > x1) 
    { 
    swap (&x0, &x1); 
    swap (&y0, &y1); 
    } 
  
  float dx = x1 - x0; 
  float dy = y1 - y0; 
  float gradient = dy / dx; 
  if (dx == 0.0) 
    gradient = 1; 

  float xend = rnd (x0);
  float yend = y0 + gradient * (xend - x0);
  float xgap = rfpart (x0 + 0.5);
  int xpxl1 = xend;
  int ypxl1 = ipart (yend);
  if (steep)
    {
    region_set_pixel_t (self, ypxl1, xpxl1, r, g, b, rfpart (yend) * xgap);
    region_set_pixel_t (self, ypxl1 + 1, xpxl1, r, g, b, fpart(yend) * xgap); 
    }
  else
    {
    region_set_pixel_t (self, xpxl1, ypxl1, r, g, b, rfpart(yend) * xgap);
    region_set_pixel_t (self, xpxl1, ypxl1 + 1, r, g, b, fpart(yend) * xgap);
    }

  float intersectY = yend + gradient; 

  xend = rnd (x1);
  yend = y1 + gradient * (xend - x1);
  xgap = rfpart (x1 + 0.5);
  int xpxl2 = xend;
  int ypxl2 = ipart (yend);
  if (steep)
    {
    region_set_pixel_t (self, ypxl2, xpxl2, r, g, b, rfpart (yend) * xgap);
    region_set_pixel_t (self, ypxl2 + 1, xpxl2, r, g, b, fpart(yend) * xgap); 
    }
  else
    {
    region_set_pixel_t (self, xpxl2, ypxl2, r, g, b, rfpart(yend) * xgap);
    region_set_pixel_t (self, xpxl2, ypxl2 + 1, r, g, b, fpart(yend) * xgap);
    }

   if (steep) 
    { 
    for (int x = xpxl1 ; x <=xpxl2 ; x++) 
      { 
      float t1 = rfpart (intersectY);
      float t2 = fpart (intersectY);
      region_set_pixel_t (self, ipart (intersectY), x, r, g, b, t1); 
      region_set_pixel_t (self, ipart (intersectY) + 1, x, r, g, b, t2);
      intersectY += gradient; 
      } 
    } 
  else
    {
    for (int x = xpxl1 ; x <=xpxl2 ; x++) 
      { 
      float t1 = rfpart (intersectY);
      float t2 = fpart (intersectY);
      region_set_pixel_t (self, x, ipart (intersectY), r, g, b, t1); 
      region_set_pixel_t (self, x, ipart (intersectY) + 1, r, g, b, t2);
      intersectY += gradient; 
      }
    }
  LOG_OUT
  }



/*==========================================================================

  region_draw_hollow_line

  Draws a hollow line of the specified thickness. In other words, draw
  a long, thin rectangle

*==========================================================================*/
void region_draw_hollow_line (Region *self, int x1, int y1, int x2, int y2, 
          int t, BYTE r, BYTE g, BYTE b)
  {
  if (t == 1)
    {
    region_draw_line_one_pixel (self, x1, y1, x2, y2, r, g, b);
    }
  else
    {
    double theta = atan2 (y2 - y1, x2 - x1); 
    double q = HALFPI - theta;

    int i = 0;
    int d = i - t / 2; 
    int nx1 = (int) (x1 - (cos (q) * d) + 0.5);
    int ny1 = (int) (y1 + (sin (q) * d) + 0.5);

    int p1x = nx1;
    int p1y = ny1;
    int p2x = nx1 + (x2 - x1); 
    int p2y = ny1 + (y2 - y1); 

    i = t - 1;
    d = i - t / 2; 
    nx1 = (int) (x1 - (0.5 * cos (q) * d) + 0.5);
    ny1 = (int) (y1 + (0.5 * sin (q) * d) + 0.5);

    int p3x = nx1;
    int p3y = ny1;
    int p4x = nx1 + (x2 - x1); 
    int p4y = ny1 + (y2 - y1); 

    region_draw_line_one_pixel (self, p1x, p1y, p2x, p2y, r, g, b);
    region_draw_line_one_pixel (self, p2x, p2y, p4x, p4y, r, g, b);
    region_draw_line_one_pixel (self, p4x, p4y, p3x, p3y, r, g, b);
    region_draw_line_one_pixel (self, p3x, p3y, p1x, p1y, r, g, b);
    }
  }


