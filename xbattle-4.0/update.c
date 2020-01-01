#include <stdio.h>
  
/**** x include files ****/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include "extern.h"

/*************************************************************************/
/**	update_board							**/
/** Update all game board values based on the values encoded in the 	**/
/** squares, and display any square that has changed.  The xtab[] and 	**/
/** ytab[] arrays are shuffled randomly so that each square will be 	**/
/** updated in a random order to remove update sequence artifacts.	**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
update_board(board)
  square_type *board;
{
  int i, j, k, l,
      disrupt,
      wrap,
      swapindex, tempindex,
      x1, y1, x2, y2;
  static int firstime=1,
             xtab[MAXBOARDSIZE],ytab[MAXBOARDSIZE];
  square_type *square, *square2;
  static unsigned int xrand=0;
  double thresh;

#if PAUSE
  if (paused)
    return;
#endif

  /*** initialize the tables first time ****/
  if (firstime)
  {
    firstime = 0;
    for (i=0; i<boardsizex; i++)
       xtab[i] = i;
    for (i=0; i<boardsizey; i++)
       ytab[i] = i;

    /**** make all squares outdated ****/
    for (i=0; i<boardsizex; i++)
      for (j=0; j<boardsizey; j++)
      {
        outdated[i][j] = ALL;
        oldvalue[i][j] = -1;
      }
  }

  /**** shuffle the tab tables ****/
  for (i=0; i<boardsizex; i++)
  {
    swapindex = (int)(drand48()*(double)(boardsizex));
    tempindex = xtab[i];
    xtab[i] = xtab[swapindex];
    xtab[swapindex] = tempindex;
  }

  for (i=0; i<boardsizey; i++)
  {
    swapindex = (int)(drand48()*(double)(boardsizey));
    tempindex = ytab[i];
    ytab[i] = ytab[swapindex];
    ytab[swapindex] = tempindex;
  }

  /**** update the board ****/
  for (i=0; i<boardsizex; i++)
  {
    for (j=0; j<boardsizey; j++)
    {
      x1 = xtab[i];
      y1 = ytab[j];

      /**** grow ****/
      growsquare(SQUARE(board,x1,y1));

      if (enable_decay)
        decaysquare(SQUARE(board,x1,y1));

      /**** fight ****/
      if (SQUARE(board,x1,y1)->color == FIGHT)
	fight(board,SQUARE(board,x1,y1));

      if (SQUARE(board,x1,y1)->color != FIGHT && SQUARE(board,x1,y1)->color != none)
      {
        disrupt = enable_disrupt[SQUARE(board,x1,y1)->color];
        wrap = enable_wrap[SQUARE(board,x1,y1)->color];
        if (SQUARE(board,x1,y1)->value[SQUARE(board,x1,y1)->color] > 0)
          SQUARE(board,x1,y1)->age = 0;
      }
      else
      {
        disrupt = FALSE;
        wrap = enable_anywrap;
        SQUARE(board,x1,y1)->age = 0;
      }
	
      /**** if move command is active ****/
      if (SQUARE(board,x1,y1)->move) 
      {
        if (SQUARE(board,x1,y1)->color!=FIGHT || !disrupt)
        {
          xrand = (xrand+1)%directions;
          for (k=0; k<directions; k++)
          {
            l = (k+xrand)%directions;

	    if (SQUARE(board,x1,y1)->dir[l])
            {

              square2 = SQUARE(board,x1,y1)->connect[l];

              if (enable_sea)
              {
                if (square2->value[none] > 0)
                  update_square(board,SQUARE(board,x1,y1),square2);
              }
              else
                update_square(board,SQUARE(board,x1,y1),square2);
            }
	  }
        }
      }
    }
  }

  /**** redraw all squares that have been changed ****/

  for (i=0; i<boardsizex; i++)
  {
    for (j=0; j<boardsizey; j++)
    {
      x1 = xtab[i];
      y1 = ytab[j];
      square = SQUARE(board,x1,y1);

      /**** outdate if change in value has occurred ****/
      if (square->color != FIGHT)
      {
        if (square->value[square->color] != oldvalue[x1][y1])
          outdated[x1][y1] = ALL;

        square->age++;

        if (enable_erode[square->color])
        {
          if (square->age > eroderate[square->color])
          {
            if (drand48() < ERODETHRESH)
            {
              for (k=0; k<directions; k++)
                square->dir[k] = 0;
              square->move = 0;
              square->age = 0;
              outdated[x1][y1] = ALL;
              if (square->value[square->color] == 0)
                square->color = none;
            }
          }
        }
      }

      for (k=0;k<nplayers;k++)
      {
        if (!winopen[k])
          continue;
        if (outdated[x1][y1] == ALL || outdated[x1][y1] == colorarray[k])
        {
          if (visible(board,colorarray[k],x1,y1))
            drawsquare(xwindow[k],square,colorarray[k],squaresize[colorarray[k]]);
          else
            drawblank(xwindow[k], x1, y1, square->value[none], square->seen[colorarray[k]],
                          square->growth, colorarray[k], squaresize[colorarray[k]], square);
        }
      }

      /**** make indated ****/
      if (outdated[x1][y1] >= 0)
      {
        if (enable_storage)
          storedrawsquare(square, squaresize[0], FALSE);
        outdated[x1][y1] = OK;
      }

      /**** set old values ****/
      if (square->color != FIGHT)
        oldvalue[x1][y1] = square->value[square->color];
      else
        oldvalue[x1][y1] = -1;
    }
  }

  if (enable_march)
    drawmarch(board);
}


/*************************************************************************/
/**	update_square							**/
/** update the square with a move command, and the  destination square	**/
/** into   which  it is  moving by  decrementing  the square value and	**/
/** incrementing the destination    square value.   The influences  of	**/
/** hills and forests are also accounted for.				**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
update_square(board,square1,square2)
  square_type *board,*square1,*square2;
{
  int i,
      side,
      disrupt,
      color1,color2,
      surplus,
      nmove;
  double slope,
         hinder,
         shunt,
         doublenmove;

  /**** get color ****/
  color1 = square1->color;
  color2 = square2->color;

  if (color1 == FIGHT)
    disrupt = enable_disrupt[0];
  else
    disrupt = enable_disrupt[color1];

  if (color1 == FIGHT && !disrupt)
  {
    if (color2 == none || color2 == square1->oldcolor)
      color1 = square1->oldcolor;
  }

  surplus = square1->value[color1] - square1->lowbound;
  if (surplus < 0)
    surplus = 0;

  /**** check value ****/
  if (surplus == 0)
    goto quit;

  /**** compute slope and forest hinderance ****/

  if (color1 != FIGHT)
  {
    slope = (double)(square2->value[none] - square1->value[none]) /
      NGREYTONES * hillfactor[color1];
    hinder = slowfactor[color1] + forest[color1] * (double)square2->value[none];

    if (enable_digin[color1])
      shunt = shuntval[color1]*((double)(square2->value[color1]))/MAXVAL + 1.0;
    else
      shunt = 1.0;
  }
  else
  {
    slope = (double)(square2->value[none] - square1->value[none]) /
      NGREYTONES * hillfactor[0];
    hinder = slowfactor[0] + forest[0] * (double)square2->value[none];
  }

  /**** advance into grey square ****/
  if (color2 == none && color1 != FIGHT)
  {
    /**** compute the number to move ****/

    doublenmove = (double)surplus / hinder / shunt /
		  (double)square1->move * (1.0-slope);
    nmove = (int)(doublenmove);
		  
    if (nmove > surplus)
      nmove = surplus;
    if (nmove + square2->value[color1] > MAXVAL)
      nmove = MAXVAL - square2->value[color1];
    if (nmove == 0 && doublenmove > 0.0 && drand48() < doublenmove)
      nmove = 1;
    if (nmove <= 0)
      goto quit;

    /**** move ****/
    square1->value[color1] = square1->value[color1] - nmove;
    for (i=0; i<directions; i++)
      square1->dir[i] = square1->dir[i];
    square2->color = color2 = color1;
    square2->value[color2] = nmove;
    square2->age = 0;

    /**** outdate the squares (for later redrawing) ****/
    outdated[square1->x][square1->y] = ALL;
    outdated[square2->x][square2->y] = ALL;

    /**** outdate neighboring squares if using horizon ****/
    if (enable_horizon)
    {
      if (square1->value[color1] == 0)
        outdatehorizon (board, square1->x, square1->y, color1);
      if (square2->value[color1] > 0)
        outdatehorizon (board, square2->x, square2->y, color1);
    }

  /**** move into friendly square ****/
  }
  else if (color1 == color2 && color1 != FIGHT)
  {
    /**** compute the number to move ****/

    doublenmove = (double)surplus / hinder / shunt /
		  (double)square1->move * (1.0-slope);
    nmove = (int)(doublenmove);

    if (nmove > surplus)
      nmove = surplus;
    if (nmove + square2->value[color1] > MAXVAL)
      nmove = MAXVAL - square2->value[color1];
    if (nmove == 0 && doublenmove > 0.0 && drand48() < doublenmove)
      nmove = 1;
    if (nmove <= 0)
      goto quit;

    /**** move ****/
    square1->value[color1] = square1->value[color1] - nmove;
    square2->value[color2] = square2->value[color1] + nmove;

    /**** outdate neighboring squares if using horizon ****/
    if (enable_horizon)
    {
      if (square1->value[color1] == 0)
        outdatehorizon (board, square1->x, square1->y, color1);
      if (square2->value[color1] == nmove && nmove>0)
        outdatehorizon (board, square2->x, square2->y, color1);
    }

  /**** invade enemy square ****/
  }
  else if (color1 != color2 && color2 != none && color2 != FIGHT)
  {
    /**** compute the number to move ****/
    doublenmove = (double)surplus / hinder / shunt /
		  (double)square1->move * (1.0-slope);
    nmove = (int)(doublenmove);

    if (nmove > surplus)
      nmove = surplus;
    if (nmove + square2->value[color1] > MAXVAL)
      nmove = MAXVAL - square2->value[color1];
    if (nmove == 0 && doublenmove > 0.0 && drand48() < doublenmove)
      nmove = 1;
    if (nmove <= 0)
      goto quit;

    /**** immobilize the enemy ****/
    if (disrupt)
      for (i=0; i<directions; i++)
        square2->dir[i] = 0;

    /**** do the move ****/
    square2->color = FIGHT;
    square1->value[color1]  = square1->value[color1] - nmove;
    square2->value[color1] = square2->value[color1] + nmove;
    square2->oldcolor = color2;

    /**** outdate the squares (for later redrawing) ****/
    outdated[square1->x][square1->y] = ALL;
    outdated[square2->x][square2->y] = ALL;

    /**** outdate neighboring squares if using horizon ****/
    if (enable_horizon)
    {
      if (square1->value[color1] == 0)
        outdatehorizon (board, square1->x, square1->y, color1);
      outdatehorizon (board, square2->x, square2->y, color1);
    }

  /**** join battle in square ****/
  }
  else if (color2 == FIGHT)
  {
    /**** compute the number to move ****/
    doublenmove = (double)surplus / hinder / shunt /
		  (double)square1->move * (1.0-slope);
    nmove = (int)(doublenmove);

    if (nmove > surplus)
      nmove = surplus;
    if (nmove + square2->value[color1] > MAXVAL)
      nmove = MAXVAL - square2->value[color1];
    if (nmove == 0 && doublenmove > 0.0 && drand48() < doublenmove)
      nmove = 1;
    if (nmove <= 0)
      goto quit;

    /**** do the move ****/
    square1->value[color1] = square1->value[color1] - nmove;
    square2->value[color1] = square2->value[color1] + nmove;

    /**** outdate the squares (for later redrawing) ****/
    outdated[square1->x][square1->y] = ALL;
    outdated[square2->x][square2->y] = ALL;

    /**** outdate neighboring squares if using horizon ****/
    if (enable_horizon)
    {
      if (square1->value[color1] == 0)
        outdatehorizon (board, square1->x, square1->y, color1);
    }
  }
  quit:;
}


/*************************************************************************/
/**	growsquare							**/
/** every square with a grow value is incremented in each update cycle	**/
/** by the grow value.  This applies to farms, towns and bases.		**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/*************************************************************************/
growsquare(square)
  square_type *square;
{
  int activecolor;

  /**** get color ****/
  activecolor = square->color;
  if(activecolor == none || activecolor == FIGHT) goto quit;

  /**** grow square ****/
  if(square->growth>0 && square->value[activecolor]<MAXVAL)
  {
    if(square->growth>50)
      square->age = 0;
    if(square->growth > (int)(100.0 * drand48())) 
      square->value[activecolor] ++;
    if(square->value[activecolor] > MAXVAL) 
      square->value[activecolor] = MAXVAL;
  }
 quit:;
}


/*************************************************************************/
/**	decaysquare							**/
/**	 Greg Lesher (lesher@park.bu.edu				**/
/*************************************************************************/
decaysquare(square)
  square_type *square;
{
  int activecolor;

  double thresh;

  /**** get color ****/
  activecolor = square->color;
  if(activecolor == none || activecolor == FIGHT) 
    return;

  thresh = decayrate[activecolor] * square->value[activecolor];
  if (drand48() < thresh)
    square->value[activecolor] -= 1;

  if (square->value[activecolor] < 0)
    square->value[activecolor] = 0;
}


/*************************************************************************/
/**	fight								**/
/** Battles  occur when  opposing  forces  occupy a square  (color  ==	**/
/** FIGHT).   The outcome of the  battle   is  determined by a   random	**/
/** variable and a nonlinear function of the relative strengths of the	**/
/** combatants, such that a larger unbalance  results in a much larger	**/
/** advantage.    In  each update    cycle each  side  suffers  losses	**/
/** determined by that function.					**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
fight(board,square)
  square_type *board,*square;
{
  int i,j,
      count, pos,
      versus[MAXSIDES];

  double sideinfer[MAXSIDES],
         sideloss[MAXSIDES];

  /**** compute the number of attacking forces against each color ****/
  for (i=0; i<nsides; i++)
  {
    versus[i] = 0;
    for (j=0; j<nsides; j++)
    {
      if (j!=i)
      {
        if (square->value[j] > 0)
          versus[i] += square->value[j];
      }
    }
  }

  /**** compute damages for each color ****/
  for (i=0; i<nsides; i++)
    if (square->value[i] > 0)
      sideinfer[i] = (double)versus[i]/((double)square->value[i]);

  for (i=0; i<nsides; i++)
  {
    if (square->value[i] > 0)
    {
      sideloss[i] = ((sideinfer[i]*sideinfer[i]) - 1.0 + drand48()*2.0) * firepower[i];
      if (sideloss[i] < 0.0)
        sideloss[i] = 0.0;
    }
    else
      sideloss[i] = 0.0;
    square->value[i] -= (int)(sideloss[i]+0.5);
  }

  /**** count remaining colors ****/
  count = 0;
  for (i=0; i<nsides; i++)
  {
    if (square->value[i] <= 0)
      square->value[i] = 0;
    else
    {
      count++;
      pos = i;
    }
  }

  /**** if there is a winner, give the square to that color ****/
  if (count == 1)
  {
    square->color = pos;  
    square->age = 0;
    if (square->oldcolor != pos)
    {
      for (i=0; i<directions; i++) square->dir[i] = 0;
      square->move = 0;
      square->lowbound = 0;
    }
  }
  else if (count == 0)
  {
    square->color = none;
    square->move = 0;
    square->lowbound = 0;
    square->age = 0;
    for (i=0; i<directions; i++) square->dir[i] = 0;
  }

  /**** outdate squares for later redrawing ****/
  outdated[square->x][square->y] = ALL;
  if (enable_horizon)
  {
    outdatehorizon (board, square->x, square->y, ALL);
  }
}


/*************************************************************************/
/**     march                                                           **/
/**  handle marching 							**/
/**     Greg Lesher (lesher@park.bu.edu)                                **/
/*************************************************************************/
march (board)
  square_type *board;
{
  int i, j,
      x, y,
      newx, newy,
      color;

  square_type *square, *square2;


  for (x=0; x<boardsizex; x++)
  {
    for (y=0; y<boardsizey; y++)
    {
      square = SQUARE(board,x,y);
      color = square->color; 

      if (square->anymarch == ACTIVE)
      {
        if (color == none || color==FIGHT)
          square->anymarch = FALSE;
        else if (color != square->marchcolor)
          square->anymarch = FALSE;
      }

      if (enable_personalmarch[color] && square->anymarch==ACTIVE)
      {
        square->marchcount += 1;
        if (square->marchcount == marchrate[color]) 
        {
          square->anymarch = FALSE;
          outdated[square->x][square->y] = ALL;

          square->move = 0;
          for (i=0; i<directions; i++)
          {
            if (square->marchtype[color] == FORCE)
              square->dir[i] = square->marchdir[color][i];
            else
              square->dir[i] = square->marchdir[color][i] || square->dir[i];
            if (square->dir[i])
              square->move++;
          }

          for (i=0; i<directions; i++)
          {
            newx = x; 
            newy = y; 

            if (square->marchdir[color][i])
            {
              square2 = SQUARE(board,x,y)->connect[i];

              if (square2->color == none || square2->color == color)
              {
                if (enable_sea && square2->value[none] <= 0)
                {
                }
                else if (square2->march[color] == PASSIVE) 
                {
                   square2->anymarch = TEMP;
                   square2->march[color] = FALSE;
                   square2->marchcolor = color;
                   square2->marchcount = 0;
                }
                else
                {
                  square2->anymarch = TEMP;
                  square2->marchtype[color] = SQUARE(board,x,y)->marchtype[color];
                  square2->marchcount = 0;
                  square2->marchcolor = color;
                  for (j=0; j<directions; j++)
                    square2->marchdir[color][j] = square->marchdir[color][j];
                }
              }
            }
          }
        }
      }

      if (color != none && color != FIGHT)
      {
        if (square->march[color] == PASSIVE)
        {
          if (square->value[color] > 0)
          {
            square->anymarch = ACTIVE;
            square->march[color] = FALSE;
            square->marchcolor = color;
            square->marchcount = 0;
          }
        }
      }
    }
  }

  for (x=0; x<boardsizex; x++)
    for (y=0; y<boardsizey; y++)
      if (SQUARE(board,x,y)->anymarch == TEMP)
        SQUARE(board,x,y)->anymarch = ACTIVE;
}


/*************************************************************************/
/**     fire                                                            **/
/**  control artillery fire						**/
/**     Greg Lesher (lesher@park.bu.edu)                                **/
/*************************************************************************/
fire (board, xpos, ypos, x, y, colorarray)
  square_type *board;
  int xpos, ypos,
      x, y,
      colorarray[];
{
  static int xrand, yrand;

  int i,
      tdir[DIRS],
      xdiff, ydiff,
      xdest, ydest,
      xfdest, yfdest,
      destcolor,
      sourcecolor,
      csize, hafc,
      winxsize, winysize;

  square_type *square;

  /**** make sure shell can be afforded ****/
  if (SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] < SHELL_COST+1)
    return;
  SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] -= SHELL_COST;

  sourcecolor = SQUARE(board,x,y)->color;

  /**** compute direction and distance of artillery shell ****/

  if (enable_hex)
  {
    xdiff = xpos - SQUARE(board,x,y)->xpos;
    ydiff = ypos - SQUARE(board,x,y)->ypos;
  }
  else
  {
    xdiff = xpos - (squaresize[sourcecolor]*SQUARE(board,x,y)->x+squaresize[sourcecolor]/2);
    ydiff = ypos - (squaresize[sourcecolor]*SQUARE(board,x,y)->y+squaresize[sourcecolor]/2);
  }

  xfdest = xpos + xdiff * 40 * artillery[SQUARE(board,x,y)->color];
  yfdest = ypos + ydiff * 40 * artillery[SQUARE(board,x,y)->color];

  if (enable_wrap[sourcecolor])
  {
    if (enable_hex)
    {
      winxsize = (boardsizex-1)*3*(hex_halfside) + 2*hex_side;
      winysize = boardsizey*2*hex_vert + hex_vert;
    }
    else
    {
      winxsize = boardsizex*squaresize[sourcecolor];
      winysize = boardsizey*squaresize[sourcecolor];
    }

    if (xfdest > winxsize)
      xfdest -= winxsize;
    else if (xfdest < 0)
      xfdest = winxsize + xfdest;

    if (yfdest > winysize)
      yfdest -= winysize;
    else if (yfdest < 0)
      yfdest = winysize + yfdest;
  }

  getsquare (xfdest, yfdest, &xdest, &ydest, tdir, squaresize[sourcecolor], board, FALSE);

  if (xdest < 0)
    xdest = 0;
  else if (xdest > boardsizex-1)
    xdest = boardsizex-1;

  if (ydest < 0)
    ydest = 0;
  else if (ydest > boardsizey-1)
    ydest = boardsizey-1;

  /**** compute effect of artillery shell ****/
  square = SQUARE(board,xdest,ydest);
  destcolor = square->color;

  if (destcolor != none && destcolor != FIGHT)
  {
    square->value[destcolor] -= SHELL;
    if (square->value[destcolor] < 0)
    {
      square->color = none;
      square->age = 0;
      square->march[destcolor] = FALSE;
      square->anymarch = FALSE;
      square->value[destcolor] = 0;
      square->move = 0;
      square->lowbound = 0;
      for (i=0; i<directions; i++)
        square->dir[i] = 0;
      if (enable_horizon)
      {
        outdatehorizon (board, xdest, ydest, destcolor);
      }
    }
  }

  /**** redraw affected squares and shell burst ****/

  for (i=0; i<nplayers; i++)
  {

    csize = 10 * squaresize[colorarray[i]]/50;
    if (enable_hex)
      hafc = csize/2;
    else
      hafc = (squaresize[colorarray[i]]-csize)/2; 

    if (!winopen[i])
      continue;

    if (visible(board,colorarray[i],x,y))
    {
      if (enable_hidden[colorarray[i]])
      {
        if (SQUARE(board,x,y)->color == colorarray[i])
          drawsquare(xwindow[i],SQUARE(board,x,y),colorarray[i],squaresize[colorarray[i]]);
      }
      else
       drawsquare(xwindow[i],SQUARE(board,x,y),colorarray[i],squaresize[colorarray[i]]);
    }

    if (enable_storage && !i)
      storedrawsquare(SQUARE(board,x,y),squaresize[0], FALSE);

    if (visible(board,colorarray[i],xdest,ydest))
    {
      if (enable_hidden[colorarray[i]])
      {
        if (SQUARE(board,xdest,ydest)->color == colorarray[i])
              drawsquare(xwindow[i],SQUARE(board,xdest,ydest),colorarray[i],squaresize[colorarray[i]]);
      }
      else
       drawsquare(xwindow[i],SQUARE(board,xdest,ydest),colorarray[i],squaresize[colorarray[i]]);

      if (enable_hex)
      {
        xrand = (xrand+2938345)%hex_side;
        yrand = (yrand+2398321)%hex_side;

        XFillArc(xwindow[i]->display,xwindow[i]->window,xwindow[i]->hue[sourcecolor],
                 SQUARE(board,xdest,ydest)->xpos - hafc + xrand - hex_halfside,
                 SQUARE(board,xdest,ydest)->ypos - hafc + yrand - hex_halfside,
                 csize, csize, 0, 23040);
      }
      else
      {
        xrand = (xrand+2938345)%(squaresize[colorarray[i]]/2);
        yrand = (yrand+2398321)%(squaresize[colorarray[i]]/2);

        XFillArc(xwindow[i]->display,xwindow[i]->window,xwindow[i]->hue[sourcecolor],
                 xdest*squaresize[colorarray[i]]+hafc+xrand-squaresize[colorarray[i]]/4,
                 ydest*squaresize[colorarray[i]]+hafc+yrand-squaresize[colorarray[i]]/4,
                 csize, csize, 0, 23040);
      }
      outdated[xdest][ydest] = ALL;
    }
    if (enable_storage && !i)
    {
      storedrawsquare(SQUARE(board,xdest,ydest),squaresize[0], FALSE);
      fprintf (fpout, "H%c%c%c%c", xdest, ydest, csize, sourcecolor);
    }
  }
}


/*************************************************************************/
/**     para                                                            **/
/**  control paratroopers						**/
/**     Greg Lesher (lesher@park.bu.edu)                                **/
/*************************************************************************/
para (board, xpos, ypos, x, y, colorarray)
  square_type *board;
  int xpos, ypos,
      x, y,
      colorarray[];
{
  static xrand, yrand;

  int i,
      tdir[DIRS],
      xdiff, ydiff,
      xdest, ydest,
      xfdest, yfdest,
      ipos, jpos,
      destcolor,
      sourcecolor,
      csize, hafc,
      winxsize, winysize;

  square_type *square;

  /**** make sure that paratroopers can be afforded ****/
  if (SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] < PARA_COST+1)
    return;
  SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] -= PARA_COST;
  sourcecolor = SQUARE(board,x,y)->color;

  /**** compute direction and distance of paratroopers ****/

  if (enable_hex)
  {
    xdiff = xpos - SQUARE(board,x,y)->xpos;
    ydiff = ypos - SQUARE(board,x,y)->ypos;
  }
  else
  {
    xdiff = xpos - (squaresize[sourcecolor]*SQUARE(board,x,y)->x+squaresize[sourcecolor]/2);
    ydiff = ypos - (squaresize[sourcecolor]*SQUARE(board,x,y)->y+squaresize[sourcecolor]/2);
  }

  xfdest = xpos + xdiff * 40 * artillery[SQUARE(board,x,y)->color];
  yfdest = ypos + ydiff * 40 * artillery[SQUARE(board,x,y)->color];

  if (enable_wrap[sourcecolor])
  {
    if (enable_hex)
    {
      winxsize = (boardsizex-1)*3*(hex_halfside) + 2*hex_side;
      winysize = boardsizey*2*hex_vert + hex_vert;
    }
    else
    {
      winxsize = boardsizex*squaresize[sourcecolor];
      winysize = boardsizey*squaresize[sourcecolor];
    }

    if (xfdest > winxsize)
      xfdest -= winxsize;
    else if (xfdest < 0)
      xfdest = winxsize + xfdest;

    if (yfdest > winysize)
      yfdest -= winysize;
    else if (yfdest < 0)
      yfdest = winysize + yfdest;
  }


  getsquare (xfdest, yfdest, &xdest, &ydest, tdir, squaresize[sourcecolor], board);

  if (xdest < 0)
    xdest = 0;
  else if (xdest > boardsizex-1)
    xdest = boardsizex-1;

  if (ydest < 0)
    ydest = 0;
  else if (ydest > boardsizey-1)
    ydest = boardsizey-1;

  /**** compute effect of paratroopers ****/
  square = SQUARE(board,xdest,ydest);
  destcolor = square->color;

  if (enable_sea && square->value[none] <= 0)
  {
  }
  else
  {
    if (destcolor == none || destcolor == sourcecolor)
    {
      square->color = sourcecolor;
      square->age = 0;
      square->anymarch = FALSE;
      square->value[sourcecolor] += PARA;
      if (square->value[sourcecolor] > MAXVAL)
        square->value[sourcecolor] = MAXVAL;
      if (enable_horizon && destcolor == none)
      {
        outdatehorizon (board, xdest, ydest, sourcecolor);
      }
    }
    else if (destcolor != sourcecolor && destcolor != FIGHT)
    {
      square->value[sourcecolor] += PARA;
      square->color = FIGHT;

      if (enable_disrupt[sourcecolor])
      {
        for (i=0; i<directions; i++)
          square->dir[i] = 0;
        square->move = 0;
      }

      if (enable_horizon)
      {
        outdatehorizon (board, xdest, ydest, sourcecolor);
      }
    }
    else if (destcolor == FIGHT)
    {
      square->value[sourcecolor] += PARA;
      if (square->value[sourcecolor] > MAXVAL)
        square->value[sourcecolor] = MAXVAL;
    }
  }

  /**** redraw affected squares and parachute ****/

  for (i=0; i<nplayers; i++)
  {
    csize = 20 * squaresize[colorarray[i]]/50;
    if (enable_hex)
      hafc = csize/2;
    else
      hafc = (squaresize[colorarray[i]]-csize)/2; 

    if (!winopen[i])
      continue;
    if (visible(board,colorarray[i],x,y))
    {
      if (enable_hidden[colorarray[i]])
      {
        if (SQUARE(board,x,y)->color == colorarray[i])
              drawsquare(xwindow[i],SQUARE(board,x,y),colorarray[i],squaresize[colorarray[i]]);
      }
      else
       drawsquare(xwindow[i],SQUARE(board,x,y),colorarray[i],squaresize[colorarray[i]]);
    }
    if (enable_storage && !i)
      storedrawsquare(SQUARE(board,x,y),squaresize[0], FALSE);

    if (visible(board,colorarray[i],xdest,ydest))
    {
      if (enable_hidden[colorarray[i]])
      {
        if (SQUARE(board,xdest,ydest)->color == colorarray[i])
              drawsquare(xwindow[i],SQUARE(board,xdest,ydest),colorarray[i],squaresize[colorarray[i]]);
      }
      else
        drawsquare(xwindow[i],SQUARE(board,xdest,ydest),colorarray[i],squaresize[colorarray[i]]);

      if (enable_hex)
      {
        xrand = (xrand+2938345)%hex_side;
        yrand = (yrand+2398321)%hex_side;

        ipos = SQUARE(board,xdest,ydest)->xpos - hafc + xrand - hex_halfside;
        jpos = SQUARE(board,xdest,ydest)->ypos - hafc + yrand - hex_halfside;
      }
      else
      {
        xrand = (xrand+2938345)%(squaresize[colorarray[i]]/2);
        yrand = (yrand+2398321)%(squaresize[colorarray[i]]/2);

        ipos = xdest*squaresize[colorarray[i]]+hafc+xrand-squaresize[colorarray[i]]/4;
        jpos = ydest*squaresize[colorarray[i]]+hafc+yrand-squaresize[colorarray[i]]/4;
      }

      XFillArc(xwindow[i]->display,xwindow[i]->window,xwindow[i]->hue[sourcecolor],
               ipos, jpos, csize, csize, 0, 11520);

      XDrawLine(xwindow[i]->display,xwindow[i]->window,xwindow[i]->hue[sourcecolor],
               ipos, jpos + csize/2, ipos + csize/2, jpos + csize);

      XDrawLine(xwindow[i]->display,xwindow[i]->window,xwindow[i]->hue[sourcecolor],
               ipos + csize/2, jpos + csize, ipos + csize, jpos + csize/2);

      XDrawLine(xwindow[i]->display,xwindow[i]->window,xwindow[i]->hue[sourcecolor],
               ipos + csize/2, jpos + csize/2, ipos + csize/2, jpos + csize);

      outdated[xdest][ydest] = ALL;
    }
    if (enable_storage && !i)
    {
      storedrawsquare(SQUARE(board,xdest,ydest),squaresize[0], FALSE);
      fprintf (fpout, "I%c%c%c%c", xdest, ydest, csize, sourcecolor);
    } 
  }
}


/*************************************************************************/
/**     visible                                                         **/
/**  Checks viewer has controlling troops within VIEWRANGE of SQUARE    **/
/**     Mark Lauer (elric@cs.su.oz.au)                                  **/
/**     Greg Lesher (lesher@park.bu.edu)                                **/
/*************************************************************************/
visible(board, activecolor, x, y)
  square_type *board;
  int activecolor,
      x, y;
{
  int i, j, i1, i2, j1, j2,
      ipos, jpos;

  /**** return TRUE if player owns square or horizon disabled ****/
  if (!enable_personalhorizon[activecolor])
    return (TRUE);
  if (SQUARE(board,x,y)->color == activecolor)
    return (TRUE);

  if (!enable_hex)
  {
    i1 = x-viewrange[activecolor];
    i2 = x+viewrange[activecolor];
    j1 = y-viewrange[activecolor];
    j2 = y+viewrange[activecolor];
  }

  /**** search for nearby squares of color color ****/
  if (enable_wrap[activecolor])
  {
    for (i=i1; i<=i2; i++)
      for (j=j1; j<=j2; j++)
        if (SQUARE(board,MODX(i),MODY(j))->value[activecolor] > 0)
          return (TRUE);
  }
  else
  {
    if (enable_hex)
    {
      for (i=0; i<hex_horizon_number; i++)
      {

        if (x%2 == 0)
        {
          ipos = x + hex_horizon_even[i][0];
          jpos = y + hex_horizon_even[i][1];
        }
        else
        {
          ipos = x + hex_horizon_odd[i][0];
          jpos = y + hex_horizon_odd[i][1];
        }

        ipos = (ipos<0) ? 0 : ipos;
        ipos = (ipos>boardsizex-1) ? boardsizex-1 : ipos;
        jpos = (jpos<0) ? 0 : jpos;
        jpos = (jpos>boardsizey-1) ? boardsizey-1 : jpos;

        if (SQUARE(board,ipos,jpos)->value[activecolor] > 0)
          return (TRUE);
      }
    }
    else
    {
      i1 = (i1<0) ? 0 : i1;
      i2 = (i2>boardsizex-1) ? boardsizex-1 : i2;
      j1 = (j1<0) ? 0 : j1;
      j2 = (j2>boardsizey-1) ? boardsizey-1 : j2;

      for (i=i1; i<=i2; i++)
        for (j=j1; j<=j2; j++)
          if (SQUARE(board,i,j)->value[activecolor] > 0)
            return (TRUE);
    }
  }
  return (FALSE);
}


/*************************************************************************/
/**     outdatehorizon                                                  **/
/**  Make area outdated for (possible) redrawing			**/
/**     Mark Lauer (elric@cs.su.oz.au)                                  **/
/**     Greg Lesher (lesher@park.bu.edu)                                **/
/*************************************************************************/
outdatehorizon (board, x, y, currentcolor)
  square_type *board;
  int x, y,
      currentcolor;
{
  int i, j, i1, i2, j1, j2, ci, cj,
      ipos, jpos,
      range,
      activecolor;

  if (currentcolor >= nsides)
  {
    currentcolor = ALL;
    range = maxviewrange;
  }
  else
    range = viewrange[currentcolor]; 

  i1 = x-range;
  i2 = x+range;
  j1 = y-range;
  j2 = y+range;

  /**** outdate VIEWRANGE area ****/
  if (enable_anywrap)
  {
    for (i=i1; i<=i2; i++)
      for (j=j1; j<=j2; j++)
      {
        ci = MODX(i);
        cj = MODY(j);

        /**** outdate differently colored squares ****/
        activecolor = SQUARE(board,ci,cj)->color;
        if (activecolor != none && activecolor != currentcolor)
        {
          if (outdated[ci][cj] == OK)
            outdated[ci][cj] = currentcolor;
          else if (outdated[ci][cj] != currentcolor)
            outdated[ci][cj] = ALL;
        }
        else if (enable_anylocalmap && activecolor == none)
        {
          if (outdated[ci][cj] == OK)
            outdated[ci][cj] = currentcolor;
          else if (outdated[ci][cj] != currentcolor)
            outdated[ci][cj] = ALL;
        }
        else if (activecolor == none && SQUARE(board,ci,cj)->growth > 50)
        {
          if (outdated[ci][cj] == OK)
            outdated[ci][cj] = currentcolor;
          else if (outdated[ci][cj] != currentcolor)
            outdated[ci][cj] = ALL;
        }
        else if (currentcolor != ALL)
        {
          if (!SQUARE(board,ci,cj)->seen[currentcolor])
          {
            if (activecolor == none && enable_terrain) 
            {
              if (enable_sea && !enable_hills && !enable_forest)
              {
                if (SQUARE(board,ci,cj)->value[none] <= 0)
                  outdated[ci][cj] = currentcolor;
              }
              else
                outdated[ci][cj] = currentcolor;
            }
          }
        }
        
        if (currentcolor != ALL)
          SQUARE(board,ci,cj)->seen[currentcolor] = TRUE;
      }
  }
  else
  {
    if (enable_hex)
    {
      i1 = 0;
      i2 = 0;
      j1 = 0;
      j2 = hex_horizon_number-1;
    }
    else
    {
      i1 = (i1<0) ? 0 : i1;
      i2 = (i2>boardsizex-1) ? boardsizex-1 : i2;
      j1 = (j1<0) ? 0 : j1;
      j2 = (j2>boardsizey-1) ? boardsizey-1 : j2;
    }

    for (i=i1; i<=i2; i++)
      for (j=j1; j<=j2; j++)
      {
        if (enable_hex)
        {
          if (x%2 == 0)
          {
            ipos = x + hex_horizon_even[j][0];
            jpos = y + hex_horizon_even[j][1];
          }
          else
          {
            ipos = x + hex_horizon_odd[j][0];
            jpos = y + hex_horizon_odd[j][1];
          }

          ipos = (ipos<0) ? 0 : ipos;
          ipos = (ipos>boardsizex-1) ? boardsizex-1 : ipos;
          jpos = (jpos<0) ? 0 : jpos;
          jpos = (jpos>boardsizey-1) ? boardsizey-1 : jpos;
        }
        else
        {
          ipos = i;
          jpos = j;
        }

        activecolor = SQUARE(board,ipos,jpos)->color;
        if (activecolor != none && activecolor != currentcolor)
        {
          if (outdated[ipos][jpos] == OK)
            outdated[ipos][jpos] = currentcolor;
          else if (outdated[ipos][jpos] != currentcolor)
            outdated[ipos][jpos] = ALL;
        }
        else if (enable_anylocalmap && activecolor == none)
        {
          if (outdated[ipos][jpos] == OK)
            outdated[ipos][jpos] = currentcolor;
          else if (outdated[ipos][jpos] != currentcolor)
            outdated[ipos][jpos] = ALL;
        }
        else if (activecolor == none && SQUARE(board,ipos,jpos)->growth > 50)
        {
          if (outdated[ipos][jpos] == OK)
            outdated[ipos][jpos] = currentcolor;
          else if (outdated[ipos][jpos] != currentcolor)
            outdated[ipos][jpos] = ALL;
        }
        else if (currentcolor != ALL)
        {
          if (!(SQUARE(board,ipos,jpos)->seen[currentcolor]))
          {
            if (activecolor == none && enable_terrain) 
            {
              if (enable_sea && !enable_hills && !enable_forest)
              {
                if (SQUARE(board,ipos,jpos)->value[none] <= 0)
                  outdated[ipos][jpos] = currentcolor;
              }
              else
                outdated[ipos][jpos] = currentcolor;
            }
          }
        }

        if (currentcolor != ALL)
          SQUARE(board,ipos,jpos)->seen[currentcolor] = TRUE;
      }
  }
}


#if VARMOUSE
/*************************************************************************/
/**   moveon                                                          	**/
/** set the move command fields in the square                         	**/
/**   J Greely <jgreely@cis.ohio-state.edu>                           	**/
/*************************************************************************/
moveon(square,dir)
  square_type *square;
  int dir[DIRS];
{
  int i;

  /**** set direction flags ****/
  square->move = 0;
  for (i=0;i<directions;i++)
  {
    if (dir[i])
      square->dir[i] = 1;;
    square->move += square->dir[i];
  }
}
#else
/*************************************************************************/
/**	setmove								**/
/** set the move command fields in the square				**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/*************************************************************************/
setmove(square,dir)
  square_type *square;
  int dir[DIRS];
{
  int i;

  /**** set direction flags ****/
  square->move = 0;
  for (i=0;i<directions;i++)
  {
    if (dir[i])
    {
      if (square->dir[i])
        square->dir[i] = 0;
      else
        square->dir[i] = 1;
    }
    if (square->dir[i])
    {
      square->move ++;
    }
  }
}
#endif


#if VARMOUSE
/*************************************************************************/
/**   moveoff                                                         	**/
/** unset the move command fields in the square				**/
/**   J Greely <jgreely@cis.ohio-state.edu>                           	**/
/*************************************************************************/
moveoff(square,dir)
  square_type *square;
  int dir[DIRS];
{
  int i;

  /**** set direction flags ****/
  square->move = 0;
  for (i=0;i<directions;i++)
  {
    if (dir[i])
      square->dir[i] = 0;
    square->move += square->dir[i];
  }
}
#else
/*************************************************************************/
/**	forcemove							**/
/** clear existing move commands and set the new one.			**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/*************************************************************************/
forcemove(square,dir)
  square_type *square;
  int dir[DIRS];
{
  int i;

  /**** set direction flags ****/
  square->move = 0;
  for (i=0; i<directions; i++)
  {
    if (dir[i])
      square->dir[i] = 1;
    else
      square->dir[i] = 0;
    if (square->dir[i])
    {
      square->move ++;
    }
  }
}
#endif
