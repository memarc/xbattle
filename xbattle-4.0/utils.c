#include <stdio.h>
  
/**** x include files ****/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include "extern.h"

/*************************************************************************/
/**	open_xwindow							**/
/** arguments:								**/
/**	struct xwindow_type *xwindow	xwindow structure		**/
/**	int xpos,ypos			window position- negative value	**/
/**					  means position with mouse	**/
/**	int xsize,ysize			window dimensions		**/
/**	char title[]			window and icon title		**/
/**	long eventmask			event mask e.g. ButtonPressMask	**/
/**                                			KeyPressMask	**/
/**                                			ExposureMask	**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
open_xwindow(xwindow,displayname,xpos,ypos,xsize,ysize,gtitle,htitle,eventmask,squaresize)
  xwindow_type *xwindow;
  char displayname[];
  int xpos,ypos,xsize,ysize;
  char gtitle[], htitle[];
  long eventmask;
  int squaresize;
{
  int i, j, k;
  GC create_gc();

  /**** visuals variables ****/
  Visual *visual;		/* struct defining window properties 	*/
  unsigned long valuemask;	/* mask for attributes 			*/
  XSetWindowAttributes attrib;	/* window attributes 			*/
  XVisualInfo vinfo;
  XPoint hex_small_points[7];
  double fraction, temp;

  GC gc_and,
     gc_all,
     gc_none,
     terrainhue[NHILLTONES];
  static Pixmap stipple[NGREYTONES+NHILLTONES];
  double ratio, value, increment;
  int limit;

  /**** open display and screen ****/
  xwindow->display = XOpenDisplay(displayname);
  if(xwindow->display == NULL)
  {
    printf("ERROR open_xwindow: can't open display %s\n",displayname);
    exit(1);
  }
  xwindow->screen  = DefaultScreen(xwindow->display);

  /**** set window position and size hints for window manager ****/
  xwindow->hint.x = xpos;
  xwindow->hint.y = ypos;
  xwindow->hint.width = xsize;
  xwindow->hint.height = ysize;
  if (xpos < 0 || ypos < 0)
    xwindow->hint.flags = ( PPosition | PSize);
  else
    xwindow->hint.flags = (USPosition | PSize);

  /**** get a visual with required properties ****/
  xwindow->depth = DefaultDepth(xwindow->display, xwindow->screen);

  visual = DefaultVisual(xwindow->display,xwindow->screen);

  if(xwindow->depth != 8)
  {
    if (XMatchVisualInfo(xwindow->display, xwindow->screen, 8, PseudoColor, &vinfo))
    {
      visual = vinfo.visual;
      xwindow->depth = 8;
    }
  }

  if (xwindow->depth != 8)
    xwindow->depth = 1;

  if(xwindow->depth == 8)
  {
    /**** create color map using visual ****/
#if NEWCOLORMAP
    xwindow->cmap = XCreateColormap(xwindow->display,
			    RootWindow(xwindow->display, xwindow->screen),
			    visual, AllocAll);
#else
    xwindow->cmap = DefaultColormap (xwindow->display, xwindow->screen);
#endif
    
    /**** set attributes ****/
    valuemask = CWBackPixel | CWBorderPixel | CWBitGravity | CWColormap;
    attrib.background_pixel = BlackPixel(xwindow->display,xwindow->screen);
    attrib.border_pixel = BlackPixel(xwindow->display,xwindow->screen);
    attrib.bit_gravity = CenterGravity;
    attrib.colormap = xwindow->cmap;

    /**** create the x window ****/
    xwindow->window = XCreateWindow(xwindow->display, 
		      RootWindow(xwindow->display, xwindow->screen),
		      xwindow->hint.x, xwindow->hint.y,
		      xwindow->hint.width, xwindow->hint.height,
		      BORDER, xwindow->depth, InputOutput, visual,
		      valuemask, &attrib);
  }
  else
  {
    xwindow->window = XCreateSimpleWindow(xwindow->display,
                                 DefaultRootWindow(xwindow->display),
                                 xwindow->hint.x, xwindow->hint.y,
                                 xwindow->hint.width, xwindow->hint.height, BORDER,
                                 BlackPixel(xwindow->display,xwindow->screen),
                                 WhitePixel(xwindow->display,xwindow->screen));
  }

  /**** get standard properties for window manager ****/
  if (xwindow->depth == 8)
    XSetStandardProperties(xwindow->display, xwindow->window, htitle, htitle,
 			 None, NULL, None, &xwindow->hint);
  else if (xwindow->depth == 1)
  {
    XSetStandardProperties(xwindow->display, xwindow->window, gtitle, gtitle, 
 			 None, NULL, None, &xwindow->hint);
  }

  /**** set window manager hints ****/
  xwindow->xwmh.flags = (InputHint|StateHint);  
  xwindow->xwmh.input = TRUE;
  xwindow->xwmh.initial_state = NormalState;
  XSetWMHints(xwindow->display, xwindow->window, &(xwindow->xwmh));

  /**** make window sensitive selected events ****/
  XSelectInput(xwindow->display, xwindow->window, eventmask);

  /**** load color map ****/
  if(xwindow->depth == 8)
  {
    for (i=0 ; i<MAXCOLORS ; i++)
    {
      xwindow->xcolor[i].flags = 0;
      xwindow->xcolor[i].pixel = i;
      xwindow->xcolor[i].red = 0;
      xwindow->xcolor[i].green = 0;
      xwindow->xcolor[i].blue = 0;
    }

    /**** set player colors ****/
    for (i=0; i<nsides; i++)
    {
      xwindow->xcolor[i].flags = DoRed | DoGreen | DoBlue;
      xwindow->xcolor[i].pixel = i;
      xwindow->xcolor[i].red =   palette[sidemap[i]][0]<<8;
      xwindow->xcolor[i].green = palette[sidemap[i]][1]<<8;
      xwindow->xcolor[i].blue =  palette[sidemap[i]][2]<<8;
    }

    /**** background color ****/
    xwindow->xcolor[none].flags = DoRed | DoGreen | DoBlue;
    xwindow->xcolor[none].pixel = none;
    xwindow->xcolor[none].red =   palette[0][0]<<8;
    xwindow->xcolor[none].green = palette[0][1]<<8;
    xwindow->xcolor[none].blue =  palette[0][2]<<8;

    /**** dark ****/
    xwindow->xcolor[dark].flags = DoRed | DoGreen | DoBlue;
    xwindow->xcolor[dark].pixel = dark;
    xwindow->xcolor[dark].red =   0<<8;
    xwindow->xcolor[dark].green = 0<<8;
    xwindow->xcolor[dark].blue =  0<<8;

    /**** light ****/
    xwindow->xcolor[light].flags = DoRed | DoGreen | DoBlue;
    xwindow->xcolor[light].pixel = light;
    xwindow->xcolor[light].red =   255<<8;
    xwindow->xcolor[light].green = 255<<8;
    xwindow->xcolor[light].blue =  255<<8;

    if (enable_terrain)
    {
      /**** set hill tones ****/
      if (enable_hills)
      {
        increment = 5.0/((double)(NHILLTONES-1));
        for (i=0; i<NHILLTONES; i++)
        {
          value = increment * i;
          xwindow->xcolor[light+1+i].flags = DoRed | DoGreen | DoBlue;
          xwindow->xcolor[light+1+i].pixel = light+1+i;

          xwindow->xcolor[light+1+i].red =   ((int)(hillintersect[0] + hillslope[0]*value))<<8;
          xwindow->xcolor[light+1+i].green = ((int)(hillintersect[1] + hillslope[1]*value))<<8;
          xwindow->xcolor[light+1+i].blue =  ((int)(hillintersect[2] + hillslope[2]*value))<<8;

/*** Simpler ratio method of hill determination ***/
/***
          ratio = ((1.25-0.75)/((double)(NHILLTONES-1)))*i + 0.75;

          xwindow->xcolor[light+1+i].red =   ((int)(ratio*hillpalette[0]))<<8;
          xwindow->xcolor[light+1+i].green = ((int)(ratio*hillpalette[1]))<<8;
          xwindow->xcolor[light+1+i].blue =  ((int)(ratio*hillpalette[2]))<<8;
***/
        }
        if (enable_sea)
        {
          xwindow->xcolor[light+1].flags = DoRed | DoGreen | DoBlue;
          xwindow->xcolor[light+1].pixel = light+1;
          xwindow->xcolor[light+1].red =   (seapalette[0])<<8;
          xwindow->xcolor[light+1].green = (seapalette[1])<<8;
          xwindow->xcolor[light+1].blue =  (seapalette[2])<<8;
        }
      }
      /**** set forest tones ****/
      if (enable_forest)
      {
        for (i=0; i<NFORESTTONES; i++)
        {
          xwindow->xcolor[light+1+i].flags = DoRed | DoGreen | DoBlue;
          xwindow->xcolor[light+1+i].pixel = light+1+i;

          ratio = ((1.25-0.75)/((double)(NFORESTTONES-1)))*(NFORESTTONES-1-i) + 0.75;

          xwindow->xcolor[light+1+i].red =   ((int)(ratio*forestpalette[0]))<<8;
          xwindow->xcolor[light+1+i].green = ((int)(ratio*forestpalette[1]))<<8;
          xwindow->xcolor[light+1+i].blue =  ((int)(ratio*forestpalette[2]))<<8;
        }
        if (enable_sea)
        {
          xwindow->xcolor[light+1].flags = DoRed | DoGreen | DoBlue;
          xwindow->xcolor[light+1].pixel = light+1;
          xwindow->xcolor[light+1].red =   (seapalette[0])<<8;
          xwindow->xcolor[light+1].green = (seapalette[1])<<8;
          xwindow->xcolor[light+1].blue =  (seapalette[2])<<8;
        }
      }
      /**** set sea tone(s) ****/
      if (enable_sea && !enable_hills && !enable_forest)
      {
        xwindow->xcolor[light+1].flags = DoRed | DoGreen | DoBlue;
        xwindow->xcolor[light+1].pixel = light+1;
        xwindow->xcolor[light+1].red =   (seapalette[0])<<8;
        xwindow->xcolor[light+1].green = (seapalette[1])<<8;
        xwindow->xcolor[light+1].blue =  (seapalette[2])<<8;

        xwindow->xcolor[light+2].flags = DoRed | DoGreen | DoBlue;
        xwindow->xcolor[light+2].pixel = light+2;
        xwindow->xcolor[light+2].red =   (palette[0][0])<<8;
        xwindow->xcolor[light+2].green = (palette[0][1])<<8;
        xwindow->xcolor[light+2].blue =  (palette[0][2])<<8;

      }
    }

#if NEWCOLORMAP
    XStoreColors(xwindow->display, xwindow->cmap, xwindow->xcolor, MAXCOLORS);
#else
    for (i=0; i<MAXCOLORS; i++)
      if (!XAllocColor(xwindow->display, xwindow->cmap, &xwindow->xcolor[i] ))
        printf ( "Warning: Couldn't allocate color cells\n");
#endif

    for (i=0; i<nsides; i++)
      xwindow->drawletter[i] = FALSE;
  }

  /**** window mapping ****/
  XMapRaised(xwindow->display, xwindow->window);

  /**** create graphics contexts (drawing colors) ****/
  for (j=0; j<=nsides+2; j++)
    xwindow->hue[j] = XCreateGC(xwindow->display, xwindow->window, 0, 0);
  xwindow->flip  = XCreateGC(xwindow->display, xwindow->window, 0, 0);

  xwindow->gc_clear  = XCreateGC(xwindow->display, xwindow->window, 0, 0);
  XSetFunction(xwindow->display, xwindow->gc_clear, GXandInverted);

  xwindow->gc_or = XCreateGC(xwindow->display, xwindow->window, 0, 0);
  XSetFunction(xwindow->display, xwindow->gc_or, GXor);

  if (enable_terrain)
  {
    if (enable_hills)
      limit = NHILLTONES;
    else if (enable_forest)
      limit = NFORESTTONES;
    if (enable_sea && !enable_hills && !enable_forest)
      limit = NSEATONES;

    for (j=0; j<limit; j++)
    {
       terrainhue[j] = XCreateGC(xwindow->display, xwindow->window, 0, 0);
       XSetFunction(xwindow->display, terrainhue[j], GXcopy);
    }
  }

  /**** set drawing functions ****/
  for (j=0; j<=nsides+2; j++)
    XSetFunction(xwindow->display, xwindow->hue[j], GXcopy);
  XSetFunction(xwindow->display, xwindow->flip,  GXinvert);

  /**** set drawing colors ****/
  if (xwindow->depth == 1)
  { 
    for (j=0; j<nsides; j++) {

      if (color2bw[sidemap[j]] == WHITE)
      {
        XSetForeground(xwindow->display, xwindow->hue[j],
                     WhitePixel(xwindow->display,xwindow->screen));
        XSetBackground(xwindow->display, xwindow->hue[j],
                     BlackPixel(xwindow->display,xwindow->screen));
      }
      else
      {
        XSetForeground(xwindow->display, xwindow->hue[j],
                     BlackPixel(xwindow->display,xwindow->screen));
        XSetBackground(xwindow->display, xwindow->hue[j],
                     WhitePixel(xwindow->display,xwindow->screen));
      }
    }
    XSetForeground(xwindow->display, xwindow->hue[none],
                     BlackPixel(xwindow->display,xwindow->screen));
    XSetBackground(xwindow->display, xwindow->hue[none],
                     WhitePixel(xwindow->display,xwindow->screen));

    XSetForeground(xwindow->display, xwindow->hue[dark],
                     BlackPixel(xwindow->display,xwindow->screen));
    XSetBackground(xwindow->display, xwindow->hue[dark],
                     WhitePixel(xwindow->display,xwindow->screen));

    XSetForeground(xwindow->display, xwindow->hue[light],
                     WhitePixel(xwindow->display,xwindow->screen));
    XSetBackground(xwindow->display, xwindow->hue[light],
                     BlackPixel(xwindow->display,xwindow->screen));

/**
    XSetForeground(xwindow->display, xwindow->flip,
                 BlackPixel(xwindow->display,xwindow->screen));
    XSetBackground(xwindow->display, xwindow->flip,
                 WhitePixel(xwindow->display,xwindow->screen));
**/
  }

  /**** set colors (link GC to color) ****/
  if (xwindow->depth == 8)
  {
#if NEWCOLORMAP
    for (j=0; j<nsides; j++)
    {
      XSetForeground (xwindow->display, xwindow->hue[j], j);
      XSetBackground (xwindow->display, xwindow->hue[j], nsides);
    }
    XSetForeground (xwindow->display, xwindow->hue[none], none);
    XSetForeground (xwindow->display, xwindow->hue[dark], dark);
    XSetForeground (xwindow->display, xwindow->hue[light], light);
#else
    for (j=0; j<nsides; j++)
    {
      XSetForeground (xwindow->display, xwindow->hue[j], xwindow->xcolor[j].pixel);
      XSetBackground (xwindow->display, xwindow->hue[j], xwindow->xcolor[nsides].pixel);
    }
    XSetForeground (xwindow->display, xwindow->hue[none], xwindow->xcolor[none].pixel);
    XSetForeground (xwindow->display, xwindow->hue[dark], xwindow->xcolor[dark].pixel);
    XSetForeground (xwindow->display, xwindow->hue[light], xwindow->xcolor[light].pixel);
#endif

    if (enable_terrain)
    {
      for (j=0; j<limit; j++)
      {
#if NEWCOLORMAP
        XSetForeground (xwindow->display, terrainhue[j], light+1+j);
        XSetBackground (xwindow->display, terrainhue[j], none);
#else
        XSetForeground (xwindow->display, terrainhue[j], xwindow->xcolor[light+1+j].pixel);
        XSetBackground (xwindow->display, terrainhue[j], xwindow->xcolor[none].pixel);
#endif
      }
    }
  }

  if (xwindow->depth == 1)
  {
    /**** set grey stipple color ****/

    init_stipple (xwindow->display, xwindow->window, stipple);

    for (j=0; j<nsides; j++)
    {
      if (color2bw[sidemap[j]] != WHITE && color2bw[sidemap[j]] != BLACK)
      {
        XSetStipple(  xwindow->display, xwindow->hue[j], stipple[color2bw[sidemap[j]]-BLACK-1]);
        XSetFillStyle(xwindow->display, xwindow->hue[j], FillOpaqueStippled);
      }
    }

    XSetStipple(  xwindow->display, xwindow->hue[nsides], stipple[NGREYTONES/2]);
    XSetFillStyle(xwindow->display, xwindow->hue[nsides], FillOpaqueStippled);
  }

  
  /**** create and initialize pixmaps for terrain ****/
  if (enable_terrain)
  {
    for (i=0; i<limit; i++)
    {
      if (enable_hex)
      {
        if (i == 0 && enable_sea)
          xwindow->terrain[i] = XCreatePixmap (xwindow->display, xwindow->window,
                                              2*fillnumber*hex_side, 6*hex_vert, xwindow->depth);
        else
          xwindow->terrain[i] = XCreatePixmap (xwindow->display, xwindow->window,
                                              2*hex_side, 6*hex_vert, xwindow->depth);
      }
      else
        xwindow->terrain[i] = XCreatePixmap (xwindow->display, xwindow->window,
                                              squaresize, squaresize, xwindow->depth);
    }

    if (xwindow->depth == 8)
    {
      for (i=0; i<limit; i++)
      {
          if (enable_hex)
            XFillRectangle(xwindow->display,xwindow->terrain[i],
                          terrainhue[i], 0, 0, 2*hex_side, 2*hex_vert);
          else
            XFillRectangle(xwindow->display,xwindow->terrain[i],
                          terrainhue[i], 0, 0, squaresize, squaresize);
      }
    }
    else if (xwindow->depth == 1)
    {
      if (enable_hex)
        init_terrain_pixmaps (xwindow, limit, 2*hex_side);
      else
        init_terrain_pixmaps (xwindow, limit, squaresize);
   
      if (enable_sea && !enable_hills && !enable_forest)
      {
        if (enable_sea)
        {
          if (enable_hex)
            XFillRectangle(xwindow->display,xwindow->terrain[1],
                   xwindow->hue[none], 0, 0, 2*hex_side, 2*hex_vert);
          else
            XFillRectangle(xwindow->display,xwindow->terrain[1],
                   xwindow->hue[none], 0, 0, squaresize, squaresize);
        }
      }
    }

    gc_and  = XCreateGC(xwindow->display, xwindow->window, 0, 0);
    XSetFunction(xwindow->display, gc_and, GXand);

    gc_all  = XCreateGC(xwindow->display, xwindow->window, 0, 0);
    XSetFunction(xwindow->display, gc_all, GXset);

    gc_none  = XCreateGC(xwindow->display, xwindow->window, 0, 0);
    XSetFunction(xwindow->display, gc_none, GXclear);

    if (enable_hex)
    {
      for (i=0; i<limit; i++)
      {
        if (i==0 && enable_sea)
        {
          for (j=0; j<fillnumber; j++)
          {
            XCopyArea (xwindow->display, xwindow->terrain[0], xwindow->terrain[0],
                              xwindow->hue[light], 0, 0, 2*hex_side, 2*hex_vert, 2*j*hex_side, 0);

            fraction = ((double)(fillnumber-j))/(fillnumber);

            hex_small_points[0].x = 2*j*hex_side + hex_halfside +
                                               (int)((1.0-fraction)*hex_halfside + 0.5);
            hex_small_points[0].y = 2*hex_vert + (int)((1.0-fraction)*hex_vert + 0.5);

            for (k=1; k<7; k++)
            {
              temp = fraction*hex_points[k].x;
              if (temp < 0)
                hex_small_points[k].x = (int)(fraction * hex_points[k].x - 0.5);
              else
                hex_small_points[k].x = (int)(fraction * hex_points[k].x + 0.5);

              temp = fraction*hex_points[k].y;
              if (temp < 0)
                hex_small_points[k].y = (int)(fraction * hex_points[k].y - 0.5);
              else
                hex_small_points[k].y = (int)(fraction * hex_points[k].y + 0.5);
            }

            XFillRectangle(xwindow->display,xwindow->terrain[0], gc_none,
                                            2*j*hex_side, 2*hex_vert, 2*hex_side, 2*hex_vert);

            XFillPolygon (xwindow->display, xwindow->terrain[0], gc_all,
                                            hex_small_points, 6, Convex, CoordModePrevious);
  
            XCopyArea (xwindow->display, xwindow->terrain[0], xwindow->terrain[0],
                       gc_and, 2*j*hex_side, 2*hex_vert, 2*hex_side, 2*hex_vert, 2*j*hex_side, 0);
          }
        }
        else
        {
          hex_points[0].x = hex_halfside;
          hex_points[0].y = 2*hex_vert;

          XFillRectangle (xwindow->display,xwindow->terrain[i], gc_none,
                                        0, 2*hex_vert, 2*hex_side, 2*hex_vert);

          XFillPolygon (xwindow->display, xwindow->terrain[i], gc_all,
                                            hex_points, 6, Convex, CoordModePrevious);
 
          XCopyArea (xwindow->display, xwindow->terrain[i], xwindow->terrain[i],
                   gc_and, 0, 2*hex_vert, 2*hex_side, 2*hex_vert, 0, 0);

        }
      }
    }
  }

  /**** load font ****/
  xwindow->font_struct = XLoadQueryFont(xwindow->display, FONT);
  if(xwindow->font_struct == 0)
  {
    printf("font %s not found, using default\n",FONT);
    xwindow->font_struct = XQueryFont(xwindow->display,XGContextFromGC(
                    DefaultGC(xwindow->display,xwindow->screen)));
  }
  else
  {
    for (i=0; i<nsides; i++)
      XSetFont(xwindow->display, xwindow->hue[i], xwindow->font_struct->fid);
    XSetFont(xwindow->display, xwindow->flip,  xwindow->font_struct->fid);
  }

  if (xwindow->depth == 1)
  {
    for (i=0; i<nsides; i++)
    {
      if (lettermap[i][0])
      {
        xwindow->drawletter[i] = TRUE;
        XGetFontProperty(xwindow->font_struct, XA_QUAD_WIDTH, &xwindow->charwidth);
        XGetFontProperty(xwindow->font_struct, XA_CAP_HEIGHT, &xwindow->charheight);
        strcpy (xwindow->letter[i], lettermap[i]);
      }
      else if (enable_reserve[i])
      {
        XGetFontProperty(xwindow->font_struct, XA_QUAD_WIDTH, &xwindow->charwidth);
        XGetFontProperty(xwindow->font_struct, XA_CAP_HEIGHT, &xwindow->charheight);
      }
      else
        xwindow->drawletter[i] = FALSE;
    }
  }
  xwindow->drawletter[none] = FALSE;

  /**** map window to make it visible ****/
  if (xwindow->depth == 8)
    XMapWindow(xwindow->display, xwindow->window);
}


/*************************************************************************/
/**	close_xwindow							**/
/**  Close the x window							**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/*************************************************************************/
close_xwindow(xwindow)
  xwindow_type *xwindow;
{
  XDestroyWindow(xwindow->display,xwindow->window);
  XCloseDisplay(xwindow->display);
}


/*************************************************************************/
/**     init_stipple                                                    **/
/**  create stipple patterns for greytoning				**/
/**     Greg Lesher (lesher@park.bu.edu)                                **/
/*************************************************************************/
init_stipple (display, window, stipple)

  Display *display;
  Window window;
  Pixmap stipple[];

{
  int i;

  static char grey[NGREYTONES+NHILLTONES][8] = {
    {0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF},
    {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55},
    {0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00}};

  for (i=0; i<NGREYTONES; i++)
    stipple[i] = XCreateBitmapFromData(display,window,grey[i],8,8);
}


/*************************************************************************/
/**     init_terrain                                                    **/
/**  yse error diffusion to emulate grayscale terrain			**/
/**     Greg Lesher (lesher@park.bu.edu)                                **/
/*************************************************************************/
init_terrain_pixmaps (xwindow, limit, squaresize)
  xwindow_type *xwindow;
  int limit,
      squaresize;
{
  int i, x, y,
      enable_quasi;

  double value,
         quasi,
         target[2*MAXSQUARESIZE+20][2*MAXSQUARESIZE+20],
         error,
         drand48();

  if (NHILLTONES > 5)
    enable_quasi = TRUE;
  else
    enable_quasi = FALSE;

  /**** for each level of greytone ****/
  for (i=0; i<limit; i++)
  {
    /**** compute desired graylevel in [0,1] ****/
   
    if (enable_forest)
      value = 0.75 - ((0.75-0.25)/((double)(limit-1)))*(limit-1-i);
    else
      value = 0.75 - ((0.75-0.25)/((double)(limit-1)))*i;

    if (enable_sea && i == 0)
    {
      value = SEAVALUE;
      enable_quasi = TRUE;
    }
    else if (NHILLTONES <= 5)
      enable_quasi = FALSE;

    for (x=0; x<squaresize+20; x++)
      for (y=0; y<squaresize+20; y++)
        target[x][y] = value;

    /**** for each square threshold and diffuse error ****/
    for (x=0; x<squaresize+20; x++)
    {
      for (y=0; y<squaresize+20; y++)
      {
        /**** threshold and set error ****/
        if (target[x][y] > 0.5)
        {
          if (x>=10 && x<squaresize+10 && y>=10 && y<squaresize+10)
            XDrawPoint (xwindow->display, xwindow->terrain[i], xwindow->hue[dark], x-10, y-10);
          error = target[x][y] - 1.0;
        }
        else
        {
          if (x>=10 && x<squaresize+10 && y>=10 && y<squaresize+10)
            XDrawPoint (xwindow->display, xwindow->terrain[i], xwindow->hue[light], x-10, y-10);
          error = target[x][y];
        }

        /**** enable randomness if over 5 levels (removes artifacts and worms) ****/
        if (enable_quasi)
          quasi = drand48()/20.0;
        else
          quasi = 0;

        /**** distribute error ****/
        if (y == 0)
        {
          if (x == squaresize+20-1)
          {
            target[x][y+1] += 1.0*error;
          }
          else
          {
            target[x][y+1] += (0.4+quasi)*error;
            target[x+1][y] += (0.4-quasi)*error;
            target[x+1][y+1] += 0.2*error;
          }
        }
        else if (y == squaresize+20-1)
        {
          if (x != squaresize+20-1)
          {
            target[x+1][y] += (0.7+quasi)*error;
            target[x+1][y-1] += (0.3-quasi)*error;
          }
        }
        else if (x == squaresize+20-1)
        {
          target[x][y+1] += 1.0*error;
        }
        else
        {
          target[x][y+1] += (0.3+quasi)*error;
          target[x+1][y] += (0.3-quasi)*error;
          target[x+1][y+1] += 0.2*error;
          target[x+1][y-1] += 0.2*error;
        }
      }
    }

    if (enable_sea && !enable_hills && !enable_forest)
      return;
  }
}


/*****************************************************************/
/**	intval							**/
/** returns the int value of the argument indexed by index+1	**/
/**	Steve Lehar (slehar@park.bu.edu)			**/
/*****************************************************************/
intval(str,argc,argv)
  char str[];
  int argc;
  char *argv[];
{
  int i,
      index=0;

  for (i=1; i<argc; i++)
  {
    if (strcmp(argv[i],str)==0)
      index=i;
  }
  if (index>0)
    sscanf(argv[index+1],"%d",&i);
  else
    i=0;
  return(i);
}


/*****************************************************************/
/**	doubleval						**/
/** returns the double value of the argument indexed by index+1	**/
/**	Steve Lehar (slehar@park.bu.edu)			**/
/*****************************************************************/
double doubleval(str,argc,argv)
  char str[];
  int argc;
  char *argv[];
{
  float f;
  int i,
      index=0;

  for (i=1; i<argc; i++)
  {
    if (strcmp(argv[i],str)==0)
      index=i;
  }
  if (index>0)
    sscanf(argv[index+1],"%f",&f);
  else
    f = 0.0;
  return((double)(f));
}


/*****************************************************************/
/**	stringval						**/
/** sets string to the string argument indexed by index+1	**/
/**	Greg Lesher (lesher@park.bu.edu)			**/
/*****************************************************************/
int stringval(str,argc,argv,string)
  char str[];
  int argc;
  char *argv[];
  char string[];
{
  int i,
      index=0;

  for (i=1; i<argc; i++)
  {
    if (strcmp(argv[i],str)==0)
      index=i;
  }
  if (index == (argc-1))
    return (FALSE);
  if (argv[index+1][0] == '-' || argv[index+1][0] == '}')
    return (FALSE);
  strcpy (string, argv[index+1]);
  return(TRUE);
}


/*****************************************************************/
/**	is_arg							**/
/** determines wether a command line argument exists in the	**/
/** form of str.						**/
/**	Steve Lehar (slehar@park.bu.edu)			**/
/*****************************************************************/
is_arg(str,argc,argv)
  char str[];
  int argc;
  char *argv[];
{
  int i,
      found=0;

  for (i=1; i<argc; i++)
  {
    if (strcmp(argv[i],str)==0)
      found=i;
  }
  return(found);
}


/*************************************************************************/
/**     matchsstr                                                       **/
/**  attempts to match first part of two input strings			**/
/**     Greg Lesher (lesher@park.bu.edu)                                **/
/*************************************************************************/
matchstr (line, word)
  char line[], word[];
{
  int i;

  for (i=0; i<strlen(word); i++)
    if (line[i] != word[i])
      return (FALSE);
  return (TRUE);
}


#if NODRAND48
double
drand48()
{
  double temp;

  temp = (double)(rand() & 0xFFFF)/((double)(0xFFFF));

  return (temp);
}
#endif
