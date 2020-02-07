/*============================================================================
 
  fbclock 
  file.c
  Copyright (c)2020 Kevin Boone, GPL v3.0

  Various file-handling helper functions

  This file is not a 'class' in the broadest sense, just an assortment of
  functions. Some of these are wrapped by the path class, which provides
  a class representation of a pathname

============================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <regex.h>
#include "defs.h" 
#include "file.h" 
#include "log.h" 
#include "list.h" 
#include "string.h" 


/*==========================================================================
  file_readline 
  Read a line from a file, allocating a buffer to store it. This function
    will probably only work with UTF8/ASCII files
  Returns the number of bytes read, or zero if end of file, or
    -1 if error
*==========================================================================*/
int file_readline (FILE *f, char **buffer)
  {
  int ch;
  int ret = -1;
  size_t buflen = 0, nchar = 64;
  int count = 0;

  LOG_IN

  *buffer = malloc (nchar);    

  while ((ch = fgetc (f)) != '\n' && ch != EOF) 
    {
    (*buffer)[buflen++] = ch;
    count++;

    if (buflen + 1 >= nchar) 
      {  
      char *tmp = realloc (*buffer, nchar * 2);
      *buffer = tmp;
      nchar *= 2;
      }
    }

  (*buffer)[buflen] = 0;

  if (buflen == 0 && ch == EOF) 
    {
    free (*buffer);
    ret = 0; 
    }
  else
    ret = count;

  return ret;
  LOG_OUT
  }


/*==========================================================================
  file_get_size
  Gets the size of a file, if possible. In failure, returns -1 and
    errno will be set. If the filename is a symlink, then the result
    is the size of the file, not the link
*==========================================================================*/
int64_t file_get_size (const char *filename)
  {
  LOG_IN
  int ret = -1;
  struct stat sb;
  if (stat (filename, &sb) == 0)
    {
    ret = sb.st_size;
    }
  LOG_OUT
  return ret;
  }


/*==========================================================================
  file_get_mtime
  Gets the mtime of a file, if possible. In failure, returns -1 and
    errno will be set. If the filename is a symlink, then the result
    applies to the file, not the link
*==========================================================================*/
time_t file_get_mtime (const char *filename)
  {
  LOG_IN
  time_t ret = -1;
  struct stat sb;
  if (stat (filename, &sb) == 0)
    {
    ret = sb.st_mtime;
    }
  LOG_OUT
  return ret;
  }

/*==========================================================================
  file_exists
  Returns TRUE if the file exists, in the most rudimentary sense -- 
    a stat() call succeeds.
*==========================================================================*/
BOOL file_exists (const char *filename)
  {
  LOG_IN
  BOOL ret = FALSE;
  struct stat sb;
  if (stat (filename, &sb) == 0)
    {
    ret = TRUE; 
    }
  LOG_OUT
  return ret;
  }

/*==========================================================================
  file_is_regular
  Return TRUE if the file exists, and is a regular file. 
    A non-existent file is, by definition, not regular 
*==========================================================================*/
BOOL file_is_regular (const char *filename)
  {
  LOG_IN
  BOOL ret = FALSE;
  struct stat sb;
  if (stat (filename, &sb) == 0)
    {
    ret = S_ISREG (sb.st_mode); 
    }
  LOG_OUT
  return ret;
  }

/*==========================================================================
  file_is_directory
  Return TRUE if the file exists, and is a directory. A non-existent file
    is, by definition, not a directory
*==========================================================================*/
BOOL file_is_directory (const char *filename)
  {
  LOG_IN
  BOOL ret = FALSE;
  struct stat sb;
  if (stat (filename, &sb) == 0)
    {
    ret = S_ISDIR (sb.st_mode); 
    }
  LOG_OUT
  return ret;
  }

/*==========================================================================

  file_write_from_string

  Writes a buffer to a file. If it exists, the file is replaced without
    warning

*==========================================================================*/
BOOL file_write_from_string (const char *filename, const String *string)
  {
  LOG_IN
  BOOL ret = FALSE;

  log_debug ("file_write_from_string: %s", filename);
  int f = open (filename, O_WRONLY | O_CREAT | O_TRUNC, 0770);
  if (f)
    {
    log_debug ("file opened");
    if (write (f, string_cstr(string), 
         string_length (string)) == string_length (string))
      ret = TRUE;
    close (f);
    }
  else
    {
    log_debug ("can't open file for writing: %s: %s", filename,
      strerror (errno));
    }

  LOG_OUT
  return ret;
  }

