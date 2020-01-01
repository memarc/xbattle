#include <stdio.h>
  
/**** x include files ****/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include "extern.h"

/*************************************************************************/
/**	drawboard							**/
/** draw the game board							**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
drawboard(board, player,justboard)
  square_type *board;
  int player,
      justboard;
{
  int i, j, k;

  if (winopen[player])
  {
    if (enable_hex)
      XFillRectangle(xwindow[player]->display, xwindow[player]->window, xwindow[player]->hue[dark],
                  0, 0, (boardsizex-1)*3*hex_halfside + 2*hex_side, boardsizey*2*hex_vert + hex_vert);

    /**** paint the pieces ****/
    for (i=0; i<boardsizex; i++)
    {
      for (j=0; j<boardsizey; j++)
      {
        if (enable_hex)
	  blanksquare(xwindow[player],SQUARE(board,i,j),colorarray[player],
                                         squaresize[colorarray[player]], TRUE);

        if (visible(board,colorarray[player],i,j))
	  drawsquare(xwindow[player],SQUARE(board,i,j),colorarray[player],
                                         squaresize[colorarray[player]]);
        else
          drawblank(xwindow[player], i, j, SQUARE(board,i,j)->value[none],
                          SQUARE(board,i,j)->seen[colorarray[player]],
                          SQUARE(board,i,j)->growth, colorarray[player],
                          squaresize[colorarray[player]], SQUARE(board,i,j));

        if (enable_storage && player == 0)
          storedrawsquare(SQUARE(board,i,j), squaresize[0], TRUE);
      }
    }
  }

  /**** write the text ****/

  if (!justboard)
  {
    for (i=0; i<nplayers; i++)
    {
      XDrawImageString(xwindow[i]->display,xwindow[i]->window,
		       xwindow[i]->hue[0],
		       TEXTX,textyh[colorarray[i]][colorarray[i]],
		       blankline,strlen(blankline));
      XDrawImageString(xwindow[i]->display,xwindow[i]->window,
		       xwindow[i]->hue[0],
		       TEXTX,textyh[colorarray[i]][colorarray[i]],
		       messagestr,strlen(messagestr));
    }
  }
}


/*************************************************************************/
/**	blanksquare						  	**/
/** Draw blank hexagon							**/ 
/**	Greg Lesher (lesher@cns.bu.edu)					**/
/*************************************************************************/
blanksquare (xwindow, square, windowcolor, squaresize, flag)
  xwindow_type *xwindow;
  square_type *square;
  int windowcolor,
      squaresize,
      flag;
{
  int  xpos, ypos;

  xpos = square->xpos;
  ypos = square->ypos;

  hex_points[0].x = xpos - hex_halfside;
  hex_points[0].y = ypos - hex_vert; 

  if (flag)
    XFillPolygon(xwindow->display,xwindow->window,xwindow->hue[none],
                               hex_points, 6, Convex, CoordModePrevious);
  if (enable_grid[windowcolor])
    XDrawLines(xwindow->display,xwindow->window,xwindow->hue[dark], hex_points, 7, CoordModePrevious);
}


/*************************************************************************/
/**	drawmarch						  	**/
/** Draw passive march markers						**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
drawmarch(board)
  square_type *board;
{
  int x, y, k;

  for (x=0; x<boardsizex; x++)
  {
    for (y=0; y<boardsizey; y++)
    {
      if (SQUARE(board,x,y)->anymarch == ACTIVE)
        drawmarcharrows (SQUARE(board,x,y),SQUARE(board,x,y)->marchcolor,ACTIVE);
      else
      {
        for (k=0; k<nsides; k++)
        {
          if (SQUARE(board,x,y)->march[k] == PASSIVE)
            if (SQUARE(board,x,y)->color == none)
            {
              drawmarcharrows(SQUARE(board,x,y),k,PASSIVE);
            }
            else if (!visible(board,k,x,y))
              drawmarcharrows(SQUARE(board,x,y),k,PASSIVE);
        }
      }
    }
  }
}


/*************************************************************************/
/**	drawmarcharrows						  	**/
/** Draw passive march markers						**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
drawmarcharrows (square,marchcolor,type)
  square_type *square;
  int marchcolor,
      type;
{
  int k,
      xpos, ypos,
      hafsquare;

  if (enable_hex)
  {
    xpos = square->xpos;
    ypos = square->ypos;

    for (k=0; k<nplayers; k++)
    {
      if (colorarray[k] == marchcolor)
      {
        if(square->marchdir[marchcolor][HEX_UP])
        {
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos+2, ypos, xpos+2, ypos-hex_vert+2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos+3, ypos, xpos+3, ypos-hex_vert+2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos-2, ypos, xpos-2, ypos-hex_vert+2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos-3, ypos, xpos-3, ypos-hex_vert+2);
        }
        if(square->marchdir[marchcolor][HEX_DOWN])
        {
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos+2, ypos, xpos+2, ypos+hex_vert-2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos+3, ypos, xpos+3, ypos+hex_vert-2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos-2, ypos, xpos-2, ypos+hex_vert-2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos-3, ypos, xpos-3, ypos+hex_vert-2);
        }
        if(square->marchdir[marchcolor][HEX_RIGHT_UP])
        {
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos-2, ypos-2, xpos+hex_3quarterside-2, ypos-hex_halfvert-2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos-3, ypos-3, xpos+hex_3quarterside-3, ypos-hex_halfvert-3);

          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos+2, ypos+2, xpos+hex_3quarterside+2, ypos-hex_halfvert+2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos+3, ypos+3, xpos+hex_3quarterside+3, ypos-hex_halfvert+3);
        }
        if(square->marchdir[marchcolor][HEX_RIGHT_DOWN])
        {
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos+2, ypos-2, xpos+hex_3quarterside+2, ypos+hex_halfvert-2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos+3, ypos-3, xpos+hex_3quarterside+3, ypos+hex_halfvert-3);

          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos-2, ypos+2, xpos+hex_3quarterside-2, ypos+hex_halfvert+2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos-3, ypos+3, xpos+hex_3quarterside-3, ypos+hex_halfvert+3);
        }
        if(square->marchdir[marchcolor][HEX_LEFT_UP])
        {
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos+2, ypos-2, xpos-hex_3quarterside+2, ypos-hex_halfvert-2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos+3, ypos-3, xpos-hex_3quarterside+3, ypos-hex_halfvert-3);

          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos-2, ypos+2, xpos-hex_3quarterside-2, ypos-hex_halfvert+2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos-3, ypos+3, xpos-hex_3quarterside-3, ypos-hex_halfvert+3);
        }
        if(square->marchdir[marchcolor][HEX_LEFT_DOWN])
        {
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos-2, ypos-2, xpos-hex_3quarterside-2, ypos+hex_halfvert-2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos-3, ypos-3, xpos-hex_3quarterside-3, ypos+hex_halfvert-3);

          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos+2, ypos+2, xpos-hex_3quarterside+2, ypos+hex_halfvert+2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos+3, ypos+3, xpos-hex_3quarterside+3, ypos+hex_halfvert+3);
        }
  
        if (type == PASSIVE)
          XDrawRectangle(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
              xpos-DSIZE, ypos-DSIZE, 2*DSIZE, 2*DSIZE);
      }
    }
  }
  else
  {
    hafsquare=squaresize[marchcolor]/2;
    xpos = square->x*squaresize[marchcolor]+hafsquare;
    ypos = square->y*squaresize[marchcolor]+hafsquare;

    for (k=0; k<nplayers; k++)
    {
      if (colorarray[k] == marchcolor)
      {
        if(square->marchdir[marchcolor][UP])
        {
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos+2, ypos, xpos+2, ypos-hafsquare);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos+3, ypos, xpos+3, ypos-hafsquare);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos-2, ypos, xpos-2, ypos-hafsquare);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos-3, ypos, xpos-3, ypos-hafsquare);
        }
        if(square->marchdir[marchcolor][DOWN])
        {
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos+2, ypos, xpos+2, ypos+hafsquare-1);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos+3, ypos, xpos+3, ypos+hafsquare-1);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos-2, ypos, xpos-2, ypos+hafsquare-1);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos-3, ypos, xpos-3, ypos+hafsquare-1);
        }
        if(square->marchdir[marchcolor][RIGHT])
        {
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos, ypos+2, xpos+hafsquare-1, ypos+2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos, ypos+3, xpos+hafsquare-1, ypos+3);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos, ypos-2, xpos+hafsquare-1, ypos-2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos, ypos-3, xpos+hafsquare-1, ypos-3);
        }
        if(square->marchdir[marchcolor][LEFT])
        {
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos, ypos+2, xpos-hafsquare, ypos+2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos, ypos+3, xpos-hafsquare, ypos+3);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos, ypos-2, xpos-hafsquare, ypos-2);
          XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
            xpos, ypos-3, xpos-hafsquare, ypos-3);
        }
  
        if (type == PASSIVE)
          XDrawRectangle(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[marchcolor],
              xpos-DSIZE, ypos-DSIZE, 2*DSIZE, 2*DSIZE);
      }
    }
  }
}


/*************************************************************************/
/**	drawsquare						  	**/
/** Draw one square of the game board					**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
drawsquare(xwindow,square,windowcolor,squaresize)
  xwindow_type *xwindow;
  square_type *square;
  int windowcolor,
      squaresize;
{
  int i, j, k,
      offset,
      ipos, jpos,
      hafsquare, psize, bsize, hafpsize, psize1, hafpsize1,
      value,
      terrain_type,
      max, smax, max_pos, smax_pos,
      csize,hafc;
  static GC onhue, offhue;

  hafsquare=squaresize/2;

  /**** get square coordinates ****/
  i = square->x;
  j = square->y;

  if (enable_hex)
  {
    ipos = square->xpos;
    jpos = square->ypos;
  }
  else
  {
    ipos = i*squaresize;
    jpos = j*squaresize;
  }

  /**** set drawing color & value ****/

  if (square->color == none || square->color == FIGHT)
    value=0;
  else
  {
    onhue = xwindow->hue[square->color];
    if ((xwindow->depth==1 && color2bw[sidemap[square->color]]<=GRAY3) ||
        (xwindow->depth==8 && sidemap[square->color]==BLACK))
      offhue = xwindow->hue[light];
    else
      offhue = xwindow->hue[dark];
    value = square->value[square->color];
  }

  if (square->color == none)
  {
    onhue = xwindow->hue[none];
    offhue = xwindow->hue[dark];
    value = 0;
  }
  
  /**** compute piece size ****/

  if (enable_hex)
    bsize = 11*hex_vert/6;
  else
    bsize = squaresize;
  psize = (int)(bsize * areavalue[value]);
  hafpsize = psize/2;

  /**** clear with appropriately colored square ****/

  if (enable_terrain)
  {
    terrain_type = square->value[none];

    if (enable_hex)
    {
      offset = -2 * square->value[none];

      if (terrain_type < 0)
      {
        double_copy (xwindow, square, windowcolor, squaresize, terrain_type, 0);
        terrain_type = 0;
      }
      else
        offset = 0;

      if (terrain_type == 1 && !enable_hills && !enable_forest)
        XFillArc (xwindow->display,xwindow->window, xwindow->hue[none],
	       ipos-(hex_vert-1), jpos-(hex_vert-1), 2*(hex_vert-1), 2*(hex_vert-1), 0, 23040);
      else
        double_copy (xwindow, square, windowcolor, squaresize, terrain_type, offset);
    }
    else
    {
      offset = ((double)(-terrain_type) * squaresize)/(2*fillnumber);

      if (terrain_type < 0)
        terrain_type = 0;
      else
        offset = 0;

      if (offset != 0)
        XCopyArea (xwindow->display, xwindow->terrain[1], xwindow->window,
                    xwindow->hue[dark], 0, 0, squaresize, squaresize, i*squaresize, j*squaresize);

      XCopyArea (xwindow->display, xwindow->terrain[terrain_type], xwindow->window,
                          xwindow->hue[dark], 0, 0, squaresize-2*offset, squaresize-2*offset,
                          i*squaresize+offset, j*squaresize+offset);
    }
  }
  else
  {
    if (enable_hex)
    {
      XFillArc (xwindow->display,xwindow->window, xwindow->hue[none],
	       ipos-(hex_vert-1), jpos-(hex_vert-1), 2*(hex_vert-1), 2*(hex_vert-1), 0, 23040);

/**
      blanksquare(xwindow, square, windowcolor, squaresize, TRUE);
**/
    }
    else
      XFillRectangle(xwindow->display,xwindow->window, xwindow->hue[none],
		   i*squaresize,j*squaresize, squaresize, squaresize);
  }

  if (!enable_hex && enable_grid[windowcolor])
  {
     XDrawLine(xwindow->display,xwindow->window,xwindow->hue[dark],
	       ipos, jpos, ipos+squaresize, jpos);
     XDrawLine(xwindow->display,xwindow->window,xwindow->hue[dark],
	       ipos, jpos, ipos, jpos+squaresize);
  }

  
  /**** singularly occupied square ****/
  if (square->color != none && square->color != FIGHT)
  {

    /**** draw center marker ****/
    if (enable_hex)
    {
      XFillArc (xwindow->display,xwindow->window, onhue,
                          ipos-hafpsize, jpos-hafpsize, psize, psize, 0, 23040);
 
      if (xwindow->drawletter[square->color])
        XDrawString(xwindow->display,xwindow->window, xwindow->flip,
                     ipos-xwindow->charwidth/2, jpos+xwindow->charheight/2,
                     xwindow->letter[square->color], 1);
    }
    else
    {
      if (!xwindow->drawletter[square->color] && !enable_grid[windowcolor])
          XDrawRectangle(xwindow->display,xwindow->window,xwindow->hue[dark],
		   ipos+hafsquare-CSIZE, jpos+hafsquare-CSIZE, 2*CSIZE, 2*CSIZE);

      XFillRectangle(xwindow->display,xwindow->window,onhue,
		   ipos+hafsquare-hafpsize, jpos+hafsquare-hafpsize, psize, psize);

      if (xwindow->drawletter[square->color])
        XDrawString(xwindow->display,xwindow->window, xwindow->flip,
                   ipos+squaresize/2-xwindow->charwidth/2, jpos+squaresize/2+xwindow->charheight/2,
                   xwindow->letter[square->color], 1);
    }

    /**** draw arrows ****/
    if (enable_hidden[windowcolor])
    {
      if (square->color == windowcolor)
      {
#if SHOWFLOW
        if (value == 0)
          drawarrows(xwindow,square,onhue,offhue,i,j,hafpsize,squaresize,TRUE);
        else
          drawarrows(xwindow,square,onhue,offhue,i,j,hafpsize,squaresize,FALSE);
#else
        drawarrows(xwindow,square,onhue,offhue,i,j,hafpsize,squaresize,FALSE);
#endif
      }
    }
    else
    {
#if SHOWFLOW
      if (value == 0)
        drawarrows(xwindow,square,onhue,offhue,i,j,hafpsize,squaresize,TRUE);
      else
        drawarrows(xwindow,square,onhue,offhue,i,j,hafpsize,squaresize,FALSE);
#else
      drawarrows(xwindow,square,onhue,offhue,i,j,hafpsize,squaresize,FALSE);
#endif
    }
  }
  else if(square->color == FIGHT)       /**** draw fight square ****/
  {
    /**** find two largest components of square ****/
    max = -1;
    smax = -1; 
    max_pos = 0;
    for (k=0;k<nsides;k++)
    {
      if (square->value[k] > max)
      {
        smax = max; 
        smax_pos = max_pos;
        max = square->value[k];
        max_pos = k;
      }
      else if (square->value[k] > smax)
      {
        smax = square->value[k];
        smax_pos = k;
      }
    }
    onhue = xwindow->hue[max_pos];
    offhue = xwindow->hue[smax_pos];
    psize = (int)(bsize * areavalue[square->value[max_pos]]);
    hafpsize = psize/2;
    psize1 = (int)(bsize * areavalue[square->value[smax_pos]]);
    hafpsize1 = psize1/2;

    if (enable_hex)
    {
      XFillArc (xwindow->display,xwindow->window, onhue,
	       ipos-hafpsize, jpos-hafpsize, psize, psize, 0, 23040);
      XFillArc (xwindow->display,xwindow->window, offhue,
	       ipos-hafpsize1, jpos-hafpsize1, psize1, psize1, 0, 23040);

      XDrawLine(xwindow->display, xwindow->window, onhue, ipos - hex_halfside, jpos - hex_halfvert,
                  ipos + hex_halfside, jpos + hex_halfvert);
      XDrawLine(xwindow->display, xwindow->window, onhue, ipos - hex_halfside, jpos + hex_halfvert,
                  ipos + hex_halfside, jpos - hex_halfvert);
    }
    else
    {
      XFillRectangle(xwindow->display,xwindow->window,onhue,
		     ipos+hafsquare-hafpsize, jpos+hafsquare-hafpsize, psize, psize);
      XFillRectangle(xwindow->display,xwindow->window,offhue,
		     ipos+hafsquare-hafpsize1, jpos+hafsquare-hafpsize1, psize1, psize1);

      /**** draw battle cross ****/
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos+2,jpos+2, ipos+squaresize-2,jpos+squaresize-2);
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos+squaresize-2,jpos+2, ipos+2,jpos+squaresize-2);
    }
  }

  /**** draw circle for town or base ****/
  if(square->angle > 0)
  {

    if (enable_hex)
    {
      if (square->angle < 23040)
        csize = ((square->oldgrowth - 52)*7*hex_vert)/200;
      else
        csize = ((square->growth - 52)*7*hex_vert)/200;

      hafc = csize/2;

      XDrawArc(xwindow->display,xwindow->window,offhue,
	       ipos-hafc, jpos-hafc, csize, csize, 0, square->angle);

      XDrawArc(xwindow->display,xwindow->window,offhue,
	       ipos-hafc + 1, jpos-hafc + 1, csize - 2, csize - 2, 0, square->angle);
    }
    else
    {
      if (square->angle < 23040)
        csize = ((square->oldgrowth - 52)*squaresize)/50;
      else
        csize = ((square->growth - 52)*squaresize)/50;

      hafc = (squaresize - csize)/2;

      XDrawArc(xwindow->display,xwindow->window,offhue,
               ipos+hafc, jpos+hafc, csize, csize, 0, square->angle);

      XDrawArc(xwindow->display,xwindow->window,offhue,
               ipos+hafc + 1, jpos+hafc + 1, csize - 2, csize - 2, 0, square->angle);
    }
  }
}


/*************************************************************************/
/**	double_copy						  	**/
/** do flickerless pixmap copy for hexagonal board			**/ 
/**	Greg Lesher (lesher@cns.bu.edu)					**/
/*************************************************************************/
double_copy (xwindow, square, windowcolor, squaresize, terrain_type, offset)
  xwindow_type *xwindow;
  square_type *square;
  int windowcolor,
      squaresize,
      terrain_type,
      offset;
{
  if (terrain_type < 0)
  {
    if (enable_hills || enable_forest)
    {
      double_copy (xwindow, square, windowcolor, squaresize, 1, 0);
      if (enable_grid[windowcolor])
        blanksquare(xwindow, square, windowcolor, squaresize, FALSE);
    }
    else
      blanksquare(xwindow, square, windowcolor, squaresize, TRUE);
    return;
  }

  XCopyArea (xwindow->display, xwindow->window, xwindow->terrain[terrain_type], xwindow->hue[light],
                     square->xpos - hex_side, square->ypos - hex_vert, 2*hex_side, 2*hex_vert,
                     0, 4*hex_vert);

  XCopyArea (xwindow->display, xwindow->terrain[terrain_type], xwindow->terrain[terrain_type],
                     xwindow->gc_clear, offset*hex_side, 2*hex_vert, 2*hex_side, 2*hex_vert,
                     0, 4*hex_vert);

  XCopyArea (xwindow->display, xwindow->terrain[terrain_type], xwindow->terrain[terrain_type],
                     xwindow->gc_or, offset*hex_side, 0, 2*hex_side, 2*hex_vert,
                     0, 4*hex_vert);

  XCopyArea (xwindow->display, xwindow->terrain[terrain_type], xwindow->window, xwindow->hue[light],
                     0, 4*hex_vert, 2*hex_side, 2*hex_vert, square->xpos - hex_side,
                     square->ypos - hex_vert);

  if (enable_grid[windowcolor])
    blanksquare(xwindow, square, windowcolor, squaresize, FALSE);
}


/*************************************************************************/
/**     drawarrows                                                      **/
/**  Draws appropriate movement arrows					**/
/**     Greg Lesher (lesher@park.bu.edu)                                **/
/*************************************************************************/
drawarrows(xwindow,square,onhue,offhue,i,j,hafpsize, squaresize, halfsize)
  xwindow_type *xwindow;
  square_type *square;
  GC onhue, offhue;
  int i, j,
      hafpsize,
      squaresize,
      halfsize;
{
  int ipos, jpos,
      vert, horiz, halfvert,
      hafsquare;

  if (enable_hex)
  {
    ipos = square->xpos;
    jpos = square->ypos;

    vert = hex_vert;
    halfvert = hex_halfvert;
    horiz = hex_3quarterside;

    if (halfsize)
    {
      vert = vert/2;
      halfvert = halfvert/2;
      horiz = horiz/2;
    }

    /**** draw direction arrows ****/
    if(square->dir[HEX_UP])
    {
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos, jpos, ipos, jpos - vert + 2); 
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos+1, jpos, ipos+1, jpos - vert + 2); 
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos-1, jpos, ipos-1, jpos - vert + 2); 
    }
    if(square->dir[HEX_DOWN])
    {
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos, jpos, ipos, jpos + vert - 2); 
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos+1, jpos, ipos+1, jpos + vert - 2); 
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos-1, jpos, ipos-1, jpos + vert - 2); 
    }
    if(square->dir[HEX_RIGHT_UP])
    {
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos, jpos, ipos + horiz - 1,
                                                jpos - halfvert + 1); 
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos+1, jpos, ipos+1 + horiz - 1,
                                                jpos - halfvert + 1); 
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos, jpos-1, ipos+1 + horiz - 1,
                                                jpos-1 - halfvert + 1); 
    }
    if(square->dir[HEX_RIGHT_DOWN])
    {
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos, jpos, ipos + horiz - 1,
                                                jpos + halfvert - 1); 
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos+1, jpos, ipos+1 + horiz - 1,
                                                jpos + halfvert - 1); 
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos, jpos+1, ipos + horiz - 1,
                                                jpos+1 + halfvert - 1); 
    }    
    if(square->dir[HEX_LEFT_UP])
    {
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos, jpos, ipos - horiz + 1,
                                                jpos - halfvert + 1); 
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos-1, jpos, ipos-1 - horiz + 1,
                                                jpos - halfvert + 1); 
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos, jpos-1, ipos - horiz + 1,
                                                jpos-1 - halfvert + 1); 
    }
    if(square->dir[HEX_LEFT_DOWN])
    {
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos, jpos, ipos - horiz + 1,
                                                jpos + halfvert - 1); 
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos-1, jpos, ipos-1 - horiz + 1,
                                                jpos + halfvert - 1); 
      XDrawLine(xwindow->display,xwindow->window,onhue, ipos, jpos+1, ipos - horiz + 1,
                                                jpos+1 + halfvert - 1); 
    }    
  }
  else
  {
    hafsquare=squaresize/2;

    ipos = i*squaresize+hafsquare;
    jpos = j*squaresize+hafsquare;

    if (halfsize)
      hafsquare /= 2;

    /**** draw direction arrows ****/
    if(square->dir[UP])
    {
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos-1, jpos, ipos-1, jpos-hafsquare);
#if INVERT
      XDrawLine(xwindow->display,xwindow->window,offhue,
                  ipos, jpos, ipos, jpos-hafpsize);
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos, jpos-hafpsize, ipos, jpos-hafsquare);
#else
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos, jpos, ipos, jpos-hafsquare);
#endif
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos+1, jpos, ipos+1, jpos-hafsquare);
    }
    if(square->dir[DOWN])
    {
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos-1, jpos, ipos-1, jpos+hafsquare-1);
#if INVERT
      XDrawLine(xwindow->display,xwindow->window,offhue,
                  ipos, jpos, ipos, jpos+hafpsize);
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos, jpos+hafpsize, ipos, jpos+hafsquare-1);
#else
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos, jpos, ipos, jpos+hafsquare-1);
#endif
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos+1, jpos, ipos+1, jpos+hafsquare-1);
    }
    if(square->dir[RIGHT])
    {
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos, jpos-1, ipos+hafsquare-1, jpos-1);
#if INVERT
      XDrawLine(xwindow->display,xwindow->window,offhue,
                  ipos, jpos, ipos+hafpsize, jpos);
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos+hafpsize, jpos, ipos+hafsquare-1, jpos);
#else
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos, jpos, ipos+hafsquare-1, jpos);
#endif
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos, jpos+1, ipos+hafsquare-1, jpos+1);
    }
    if(square->dir[LEFT])
    {
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos, jpos-1, ipos-hafsquare, jpos-1);
#if INVERT
      XDrawLine(xwindow->display,xwindow->window,offhue,
                  ipos, jpos, ipos-hafpsize, jpos);
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos-hafpsize, jpos, ipos-hafsquare, jpos);
#else
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos, jpos, ipos-hafsquare, jpos);
#endif
      XDrawLine(xwindow->display,xwindow->window,onhue,
                  ipos, jpos+1, ipos-hafsquare, jpos+1);
    }
  }
}


/*************************************************************************/
/**     drawblank                                                       **/
/**  Draws a blank square                                               **/
/**     Mark Lauer (elric@cs.su.oz.au)                                  **/
/**     Greg Lesher (lesher@park.bu.edu)                                **/
/*************************************************************************/
drawblank (xwindow, i, j, terrain_type, seen, growth, windowcolor, squaresize, square)
  xwindow_type *xwindow;
  int i, j,
      terrain_type,
      seen,
      growth,
      windowcolor,
      squaresize;
  square_type *square;
{
  int csize, hafc,
      ipos, jpos,
      offset;

  if (enable_hex)
  {
    ipos = square->xpos;
    jpos = square->ypos;
  }
  else
  {
    ipos = i*squaresize;
    jpos = j*squaresize;
  }

  /**** clear with appropriately colored square ****/
  if (enable_terrain)
  {
    if (seen && !enable_localmap[windowcolor])
    {
      if (enable_hex)
      {
        offset = -2 * square->value[none];

        if (terrain_type < 0)
        {
          double_copy (xwindow, square, windowcolor, squaresize, terrain_type, 0);
          terrain_type = 0;
        }
        else
          offset = 0;

        if (terrain_type == 1 && !enable_hills && !enable_forest)
          XFillArc (xwindow->display,xwindow->window, xwindow->hue[none],
	         ipos-(hex_vert-1), jpos-(hex_vert-1), 2*(hex_vert-1), 2*(hex_vert-1), 0, 23040);
        else
          double_copy (xwindow, square, windowcolor, squaresize, terrain_type, offset);
      }
      else
      {
        offset = ((double)(-terrain_type) * squaresize)/(2*fillnumber);

        if (terrain_type < 0)
          terrain_type = 0;
        else
          offset = 0;

        if (offset != 0)
          XCopyArea (xwindow->display, xwindow->terrain[1], xwindow->window,
                      xwindow->hue[dark], 0, 0, squaresize, squaresize, ipos, jpos);
  
        XCopyArea (xwindow->display, xwindow->terrain[terrain_type], xwindow->window,
                          xwindow->hue[dark], 0, 0, squaresize-2*offset, squaresize-2*offset,
                          ipos+offset, jpos+offset);
      }

      if (enable_basemap[windowcolor] && growth > 50)
      {
        if (enable_hex)
        {
          csize = ((growth - 52)*7*hex_vert)/200;
          hafc = csize/2;

          XDrawArc(xwindow->display,xwindow->window,xwindow->hue[dark],
	           ipos-hafc, jpos-hafc, csize, csize, 0, square->angle);
 
          XDrawArc(xwindow->display,xwindow->window,xwindow->hue[dark],
	           ipos-hafc + 1, jpos-hafc + 1, csize - 2, csize - 2, 0, square->angle);
        }
        else
        {
          csize = ((growth - 52)*squaresize)/50;
          hafc = (squaresize - csize)/2;
          XDrawArc(xwindow->display,xwindow->window,xwindow->hue[dark],
	           ipos+hafc, jpos+hafc, csize, csize, 0, 23040);

          XDrawArc(xwindow->display,xwindow->window,xwindow->hue[dark],
                   ipos+hafc + 1, jpos+hafc + 1, csize - 2, csize - 2, 0, 23040);
        }
      }
    }
    else
    {
      if (enable_hex)
      {
        /** Using double copy results in some aliasing due to fact that hue[none] is **/
        /** usually drawn not copied and is a stipple.				     **/ 
/**
        double_copy (xwindow, square, windowcolor, squaresize, 1, 0);
**/

        blanksquare(xwindow, square, windowcolor, squaresize, TRUE);
      }
      else
        XFillRectangle(xwindow->display,xwindow->window,
                       xwindow->hue[none], ipos,jpos, squaresize, squaresize);
    }
  }
  else
  {
     if (enable_hex) 
     {
        XFillArc (xwindow->display,xwindow->window, xwindow->hue[none],
	       ipos-(hex_vert-1), jpos-(hex_vert-1), 2*(hex_vert-1), 2*(hex_vert-1), 0, 23040);
/**
        blanksquare(xwindow, square, windowcolor, squaresize, TRUE);
**/
     }
     else
       XFillRectangle(xwindow->display,xwindow->window,
                     xwindow->hue[none],
                     ipos,jpos, squaresize, squaresize);

    if (enable_basemap[windowcolor])
    {
      if (growth > 50 && !enable_localmap[windowcolor] && seen)
      {
        if (enable_hex) 
        {
          csize = ((growth - 52)*7*hex_vert)/200;
          hafc = csize/2;

          XDrawArc(xwindow->display,xwindow->window,xwindow->hue[dark],
	           ipos-hafc, jpos-hafc, csize, csize, 0, square->angle);
 
          XDrawArc(xwindow->display,xwindow->window,xwindow->hue[dark],
	           ipos-hafc + 1, jpos-hafc + 1, csize - 2, csize - 2, 0, square->angle);
        }
        else
        {
          csize = ((growth - 52)*squaresize)/50;
          hafc = (squaresize - csize)/2;

          XDrawArc(xwindow->display,xwindow->window,xwindow->hue[dark],
	         ipos+hafc, jpos+hafc, csize, csize, 0, 23040);

          XDrawArc(xwindow->display,xwindow->window,xwindow->hue[dark],
  	         ipos+hafc + 1, jpos+hafc + 1, csize - 2, csize - 2, 0, 23040);
        }
      }
    }
  }

  if (!enable_hex && enable_grid[windowcolor])
  {
     XDrawLine(xwindow->display,xwindow->window,xwindow->hue[dark],
	       ipos,jpos,
	       ipos+squaresize,jpos);
     XDrawLine(xwindow->display,xwindow->window,xwindow->hue[dark],
	       ipos,jpos,
	       ipos,jpos+squaresize);
  }
}


/*************************************************************************/
/**	storedrawsquare						  	**/
/** Draw one square of the game board to a file				**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
storedrawsquare(square, squaresize, special_flag)
  square_type *square;
  int squaresize,
      special_flag;
{
  int i, j, k,
      bsize, csize, psize, psize1,
      value,
      max, smax, max_pos, smax_pos;

  i = square->x;
  j = square->y;

  if (enable_hex)
    bsize = 9*hex_vert/5;
  else
    bsize = squaresize;

  if (enable_terrain)
  {
    if (enable_hex && special_flag)
    {
      if (!enable_hills && !enable_forest && square->value[none] == 1)
        fprintf (fpout,"b%c%c",i,j);
      else
        fprintf (fpout,"A%c%c%c",i,j,square->value[none]);
    }
    else
      fprintf (fpout,"A%c%c%c",i,j,square->value[none]);
  }
  else
  {
    if (enable_hex && special_flag)
      fprintf (fpout,"b%c%c",i,j);
    else
      fprintf (fpout,"B%c%c",i,j);
  }
  
  /**** singularly occupied square ****/
  if (square->color != none && square->color != FIGHT)
  {
    storedrawarrows(square,i,j,square->color);

    value = square->value[square->color];
    if (value < 0)
      value = 0;
    psize = (int)(bsize * areavalue[value]);
    
    fprintf (fpout,"D%c%c%c%c",i,j,psize,square->color);
  }
  else if(square->color == FIGHT)
  {
    /**** find two largest components of square ****/
    max = -1;
    smax = -1; 
    for (k=0;k<nsides;k++)
    {
      if (square->value[k] > max)
      {
        smax = max; 
        smax_pos = max_pos;
        max = square->value[k];
        max_pos = k;
      }
      else if (square->value[k] > smax)
      {
        smax = square->value[k];
        smax_pos = k;
      }
    }
    psize = (int)(bsize * areavalue[square->value[max_pos]]);
    psize1 = (int)(bsize * areavalue[square->value[smax_pos]]);

    fprintf (fpout,"E%c%c%c%c%c%c",i,j,psize,max_pos,psize1,smax_pos);
  }

  /**** draw circle for town or base ****/
  if(square->angle > 0)
  {
    if (enable_hex)
    {
      if (square->angle < 23040)
        csize = ((square->oldgrowth - 52)*7*hex_vert)/200;
      else
        csize = ((square->growth - 52)*7*hex_vert)/200;
    }
    else
    {
      if (square->angle < 23040)
        csize = ((square->oldgrowth - 52)*squaresize)/50;
      else
        csize = ((square->growth - 52)*squaresize)/50;
    }

    fprintf (fpout,"F%c%c%c%c",i,j,csize,(square->angle*255)/23040);
  }
}


/*************************************************************************/
/**     storedrawarrows                                                 **/
/**  Draws appropriate movement arrows to file				**/
/**     Greg Lesher (lesher@park.bu.edu)                                **/
/*************************************************************************/
storedrawarrows(square,i,j,colornumber)
  square_type *square;
  int i, j,
      colornumber;
{
  if (enable_hex)
  {
    if(square->dir[HEX_UP])
      fprintf (fpout,"G%c%c%cu",i,j,colornumber);

    if(square->dir[HEX_DOWN])
      fprintf (fpout,"G%c%c%cd",i,j,colornumber);

    if(square->dir[HEX_RIGHT_UP])
      fprintf (fpout,"G%c%c%cR",i,j,colornumber);

    if(square->dir[HEX_RIGHT_DOWN])
      fprintf (fpout,"G%c%c%cr",i,j,colornumber);

    if(square->dir[HEX_LEFT_UP])
      fprintf (fpout,"G%c%c%cL",i,j,colornumber);

    if(square->dir[HEX_LEFT_DOWN])
      fprintf (fpout,"G%c%c%cl",i,j,colornumber);
  }
  else
  {
    if(square->dir[UP])
      fprintf (fpout,"G%c%c%cu",i,j,colornumber);

    if(square->dir[DOWN])
      fprintf (fpout,"G%c%c%cd",i,j,colornumber);

    if(square->dir[RIGHT])
      fprintf (fpout,"G%c%c%cr",i,j,colornumber);

    if(square->dir[LEFT])
      fprintf (fpout,"G%c%c%cl",i,j,colornumber);
  }
}


/*************************************************************************/
/**	print_message							**/
/** Print the latest message to the screen.  The message is purposely	**/
/** allowed to flash in order to attract attention.  Further attention	**/
/** can be called by use of ^g to ring the bell.			**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
print_message(text,textcount,newcolor,player,board)
  char text[];
  int textcount,
      newcolor,
      player;
  square_type *board;
{
  int i,
      scroll, scrollandprint;

  static oldcolor=0,
         veryfirst=TRUE,
         firsttime[MAXSIDES];

  char oldmessage[512],
       line[100];

  if (veryfirst)
  {
    for (i=0; i<MAXSIDES; i++)
      firsttime[i] = TRUE;
    veryfirst = FALSE;
  }

  scroll = FALSE;
  scrollandprint = FALSE;

#if !MULTITEXT
  /**** copy old message ****/
  strcpy (oldmessage,messagestr);
  if (newcolor != oldcolor)
    strcpy(messagestr,"");
#endif

  /**** remove control, shift & meta ****/
  if (textcount==0)
    return;

  text[textcount] = 0;

  if (firsttime[newcolor])
  {
    strcpy(personal_messagestr[newcolor], "");
    firsttime[newcolor] = FALSE;
  }
  
  switch (text[0])
  {
    /**** backspace ****/
    case BACKSPACE:
    case DELETE:
#if MULTITEXT
      if (strlen(personal_messagestr[newcolor]) > 0)
        personal_messagestr[newcolor][(strlen(personal_messagestr[newcolor]))-1] =
                                                  messagestr[(strlen(personal_messagestr[newcolor]))];
#else
      if (strlen(messagestr) > 0)
        messagestr[(strlen(messagestr))-1] = messagestr[(strlen(messagestr))];
#endif
      break;

    /**** newline ****/
    case RETURN:
#if MULTITEXT
      strcpy(personal_messagestr[newcolor],"");
#else
      scroll = TRUE;
#endif
      break;
    
    case CTRLC:
    case CTRLQ:
      firsttime[newcolor] = TRUE;
      removeplayer (player, board);
      break;

    case CTRLP:
      gamestats (board);
      dump_board (board, "xbattle.xbt");
      break;

    case CTRLW:
      winwatch[player] = TRUE;
      enable_personalhorizon[colorarray[player]] = FALSE;
      drawboard (board, player, TRUE);
      firsttime[newcolor] = TRUE;
      sprintf (line, "%s has quit the game", huename[sidemap[colorarray[player]]]);
      print_message(line,strlen(line),colorarray[player],player,board);
      for (i=0; i<nplayers; i++)
        if (winopen[i]) XBell(xwindow[i]->display,100);
      break;

    case CTRLG:
      for (i=0; i<nplayers; i++)
        if (winopen[i])
          XBell(xwindow[i]->display,100);
      break;

    case SPACE:
#if MULTITEXT
      if ((strlen(personal_messagestr[newcolor]) + 1) > 511)
      {
        scroll = TRUE;
      }
      else
        strcat(personal_messagestr[newcolor]," ");
#else
      if ((strlen(messagestr) + 1) > 511)
      {
        scroll = TRUE;
      }
      else
        strcat(messagestr," ");
#endif

      break;

    default:
#if MULTITEXT
      if ((strlen(personal_messagestr[newcolor]) + strlen(text)) > 511)
      {
        scroll = TRUE;
      }
      else
        strcat(personal_messagestr[newcolor],text);
#else
      if ((strlen(messagestr) + strlen(text)) > 511)
      {
        scroll = TRUE;
      }
      else
        strcat(messagestr,text);
#endif

      break;
  }

#if !MULTITEXT
  if (newcolor != oldcolor)
    scrollandprint = TRUE;
#endif

  /**** print the string on all xwindows ****/
  for (i=0; i<nplayers; i++)
  {
    if (!winopen[i])
      continue;
#if !MULTITEXT
    /**** scroll bottom text line up ****/
    if (scroll || scrollandprint)
    {
      XDrawImageString(xwindow[i]->display,xwindow[i]->window,
		       xwindow[i]->hue[oldcolor],
		       TEXTX,textyh[colorarray[i]][newcolor],
		       blankline,strlen(blankline));
      XDrawImageString(xwindow[i]->display,xwindow[i]->window,
		       xwindow[i]->hue[oldcolor],
		       TEXTX,textyh[colorarray[i]][newcolor],
		       oldmessage,strlen(oldmessage));
      XDrawImageString(xwindow[i]->display,xwindow[i]->window,
		       xwindow[i]->hue[newcolor],
		       TEXTX,textyl[colorarray[i]][newcolor],
		       blankline,strlen(blankline));
    }

    /**** print new text line ****/
    if (scrollandprint || !scroll)
    {
      XDrawImageString(xwindow[i]->display,xwindow[i]->window,
		       xwindow[i]->hue[newcolor],
		       TEXTX,textyl[colorarray[i]][newcolor],
		       blankline,strlen(blankline));
      XDrawImageString(xwindow[i]->display,xwindow[i]->window,
		       xwindow[i]->hue[newcolor],
		       TEXTX,textyl[colorarray[i]][newcolor],
		       messagestr,strlen(messagestr));
    }
#else
    /**** print new text line ****/
    if (scrollandprint || !scroll)
    {
      XDrawImageString(xwindow[i]->display,xwindow[i]->window,
		       xwindow[i]->hue[newcolor],
		       TEXTX,textyh[colorarray[i]][newcolor],
		       blankline,strlen(blankline));
      XDrawImageString(xwindow[i]->display,xwindow[i]->window,
		       xwindow[i]->hue[newcolor],
		       TEXTX,textyh[colorarray[i]][newcolor],
		       personal_messagestr[newcolor],strlen(personal_messagestr[newcolor]));
    }
#endif
  }

#if !MULTITEXT
  if (scrollandprint || !scroll)
    oldcolor=newcolor;
#endif

  if (scroll)
    strcpy(messagestr,"");
}
