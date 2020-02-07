/*==========================================================================

  fbclock 
  usage.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

==========================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "feature.h" 
#include "usage.h" 


/*==========================================================================
  usage_show
==========================================================================*/
void usage_show (FILE *fout, const char *argv0)
  {
  fprintf (fout, "Usage: %s [options]\n", argv0);
  fprintf (fout, "  -?,--help            show this message\n");
  fprintf (fout, "  -d,--date            show date\n");
  fprintf (fout, "  -f,--fbdev=device    framebuffer device (/dev/fb0)\n");
  fprintf (fout, "  -h,--height=N         display height\n");
  fprintf (fout, "     --log-level=N     log level, 0-5 (default 2)\n");
  fprintf (fout, "  -s,--seconds         show seconds\n");
  fprintf (fout, "  -v,--version         show version\n");
  fprintf (fout, "  -w,--width=N         display width\n");
  fprintf (fout, "  -t,--transparency=%%  transparency\n");
  fprintf (fout, "  -x,--x=N             display x position\n");
  fprintf (fout, "  -y,--y=N             display y position\n");
  }

 
