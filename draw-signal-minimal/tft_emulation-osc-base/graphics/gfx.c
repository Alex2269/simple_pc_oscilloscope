#include <X11/Xlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


Display *gfx_display=0;
Window gfx_window;
GC gfx_gc;
Colormap gfx_colormap;
int gfx_fast_color_mode = 0;


/* Open a new graphics window. */
void gfx_open( int width, int height, const char *title )
{
  gfx_display = XOpenDisplay(0);

  if(!gfx_display)
  {
    fprintf(stderr,"gfx_open: unable to open the graphics window.\n");
    exit(1);
  }

  Visual *visual = DefaultVisual(gfx_display,0);

  if(visual && visual->class==TrueColor)
  {
    gfx_fast_color_mode = 1;
  }

  else
  {
    gfx_fast_color_mode = 0;
  }

  int blackColor = BlackPixel(gfx_display, DefaultScreen(gfx_display));
  int whiteColor = WhitePixel(gfx_display, DefaultScreen(gfx_display));

  gfx_window = XCreateSimpleWindow(gfx_display, DefaultRootWindow(gfx_display), 0, 0, width, height, 0, blackColor, blackColor);
  XSetWindowAttributes attr;
  attr.backing_store = Always;
  XChangeWindowAttributes(gfx_display,gfx_window,CWBackingStore,&attr);
  XStoreName(gfx_display,gfx_window,title);
  XSelectInput(gfx_display, gfx_window, StructureNotifyMask|KeyPressMask|ButtonPressMask);
  XMapWindow(gfx_display,gfx_window);
  gfx_gc = XCreateGC(gfx_display, gfx_window, 0, 0);
  XSetForeground(gfx_display, gfx_gc, whiteColor);

  // Wait for the MapNotify event
  for(;;)
  {
    XEvent e;
    XNextEvent(gfx_display, &e);
    if (e.type == MapNotify)
    break;
  }
}

/* Flush all previous output to the window. */
void gfx_flush(void)
{
  XFlush(gfx_display);
}

