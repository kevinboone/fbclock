# fbclock 

Version 1.0b

# What is this?

`fbclock` is a utility that draws an analogue clock face directly
on the Linux framebuffer. It is intended for embedded Linux systems,
for use on lock screens or to superimpose on a wallpaper.
It will run on desktop Linux systems, but there's not a lot of point.

This utility is written in C and is completely self-contained -- 
it even contains its own fonts. It therefore has no external
dependencies apart from the standard C library.

`fbclock` is part of a set of embedded Linux utilities, that includes
`jpegtofb` -- a direct-to-framebuffer image slideshow.

## Building

The usual:

    $ make
    $ sudo make install

## Command-line switches

`-d,--date`

Show the date on the clock face.

`-f,--fbdev`

Select the framebuffer device -- default is `/dev/fb0`.

`h,--height=N` 

Height of the display, in pixels

`--log-level=N`

Set the log level from 0 to 5, Default is 2. Levels higher than
three will only make sense when examined alongside the source
code.

`-s,--seconds` 

Show second hand

`-w,--width=N`

Width of the display, in pixels

`t,--transparency=%`

Percentage transparency. When set to 100, the underlying framebuffer
is completely visible. When 0, the clock display is on a black
background.

`x,--x=N`

Horizontal position of the top-left corner of the display, in pixels.

`y,--y=N`

Vertical position of the top-left corner of the display, in pixels.

## Notes

`fbclock` tries to superimpose itself on the existing framebuffer
contents but, of course, the framebuffer contents may change.
This will erase the clock display until the next update -- which
might be a minute -- and when the display does update, the background
will be incorrect. To handle this situation -- and this is only 
possible when all the various applications collaborate -- `fbclock`
will update, including sampling the framebuffer contents, when it receives
signal USR2. 

Even if the simple refresh procedure is used, it could theoretically
still fail, if the signal arrives whilst the clock display is
being redrawn. This doesn't seem to be a problem in practice, 
but in principle it could be.

I've taken some trouble to minimize the amount of work done when
the display refreshes, but there's still a fair amount of math
and data-pushing. 
With the second-hand drawn, and a 300x300 display size, `fbclock` uses
0.5%-1.5% CPU on a Raspberry Pi 3. 

It should go without saying that, to use this utility, the user must
have read/write access to the framebuffer. It's designed to run 
in an environment without X or any graphical desktop -- if you
have X running, then you have far better options than this.

## Legal, etc

`fbclock` is copyright (c)2020 Kevin Boone, and distributed under the
terms of the GNU Public Licence, version 3.0. Essentially that means
you may use the software however you wish, so long as the source
code continues to be made available, and the original author is
acknowledged. There is no warranty of any kind.

## Revision history

1.0a Feb 2020<br/>
First working release

1.0b Feb 2020<br/>
Added code to allow for framebuffers with stride != 
width * bytes\_per\_pixel


