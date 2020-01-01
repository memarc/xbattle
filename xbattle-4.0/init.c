#include <stdio.h>
  
/**** x include files ****/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include "extern.h"

/*************************************************************************/
/**	initboard							**/
/** Initialize the squares of the game board				**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/*************************************************************************/
init_board(board)
  square_type *board;
{
  int i, j, k,
      even;

  /**** initialize random number generator ****/
#if NODRAND48
  srand((long)seed);
#else
  srand48((long)seed);
#endif

  printf ("seed: %d\n", seed);

  /**** initialize the board values ****/
  for (j=0; j<boardsizey; j++)
  {
    even = TRUE;

    for (i=0; i<boardsizex; i++)
    {
      SQUARE(board,i,j)->x           = i;
      SQUARE(board,i,j)->y           = j;
      SQUARE(board,i,j)->color       = none;
      SQUARE(board,i,j)->oldcolor    = none;
      SQUARE(board,i,j)->value[none] = 0;
      for (k=0; k<(nsides+2); k++)
        SQUARE(board,i,j)->value[k]  = 0;
      SQUARE(board,i,j)->lowbound    = 0;
      SQUARE(board,i,j)->growth      = 0;
      SQUARE(board,i,j)->oldgrowth   = 0;
      SQUARE(board,i,j)->angle       = 0;
      SQUARE(board,i,j)->age         = 0;
      SQUARE(board,i,j)->anymarch    = FALSE;
      for (k=0; k<nsides; k++)
        SQUARE(board,i,j)->march[k]  = FALSE;
      SQUARE(board,i,j)->move        = 0;
      for (k=0; k<directions; k++)
        SQUARE(board,i,j)->dir[k]    = 0;

      for (k=0; k<nsides; k++)
        SQUARE(board,i,j)->seen[k]   = TRUE;

      /** define centers **/

      if (enable_hex)
      {
        if (even)
          SQUARE(board,i,j)->ypos = j*2*hex_vert + hex_vert;
        else
          SQUARE(board,i,j)->ypos = j*2*hex_vert + 2*hex_vert;

        SQUARE(board,i,j)->xpos = hex_side + i*(3*hex_side/2);

        even = !even;
      }
      else
      {
        SQUARE(board,i,j)->xpos = i * squaresize[0] + squaresize[0]/2;
        SQUARE(board,i,j)->ypos = j * squaresize[0] + squaresize[0]/2;
      }
    }
  }

  if (enable_hex)
    set_hex_connections (board);
  else
    set_square_connections (board);

  if (enable_edit)
  {
    for (i=0; i<boardsizex; i++)
      for (j=0; j<boardsizey; j++)
      {
        if (enable_hills)
          SQUARE(board,i,j)->value[none] = NHILLTONES/2;
        else if (enable_forest)
          SQUARE(board,i,j)->value[none] = NFORESTTONES/2;
        else if (enable_sea)
          SQUARE(board,i,j)->value[none] = 1;
      }

    return;
  }

  if (enable_load)
  {
    load_board (board, mapname, FALSE);
  }
  else
  {
    if (enable_terrain)
      init_hills(board, seed);
  }

  if (enable_farms)
    init_farms(board, farms);

  if (enable_towns)
    init_towns(board,towns,seed);

  if (enable_bases)
    init_bases(board,bases);

  if (enable_rbases)
    init_rbases(board,rbases,seed);

  if (enable_armies)
    init_armies(board,armies);

  if (enable_militia)
    init_militia(board,militia,seed);

  init_unseen(board);

  if (enable_horizon && enable_hex)
    init_horizon (board);
}


/*************************************************************************/
/**	init_horizon							**/
/** Initialize horizon for hexagonal board				**/
/**	Greg Lesher (lesher@cns.bu.edu)					**/
/*************************************************************************/
init_horizon (board)
  square_type *board;
{
  int i, j, k,
      ibase, jbase,
      half,
      index;

  square_type *square;

  half = boardsizex/2;
  if (half%2 == 0)
    square = SQUARE (board, half, boardsizey/2);
  else
    square = SQUARE (board, half+1, boardsizey/2);

  ibase = square->x;
  jbase = square->y;

  index = 0;

  for (i=0; i<viewrange[0]; i++)
  {
    square = square->connect[HEX_UP];

    for (j=0; j<6; j++)
    {
      for (k=0; k<(i+1); k++)
      {
        square = square->connect[(HEX_RIGHT_DOWN+j)%6];

        hex_horizon_even[index][0] = square->x - ibase;
        hex_horizon_even[index][1] = square->y - jbase;
        index++;
      }
    }
  }

  if (half%2 == 0)
    square = SQUARE (board, half+1, boardsizey/2);
  else
    square = SQUARE (board, half, boardsizey/2);

  ibase = square->x;
  jbase = square->y;

  index = 0;

  for (i=0; i<viewrange[0]; i++)
  {
    square = square->connect[HEX_UP];

    for (j=0; j<6; j++)
    {
      for (k=0; k<(i+1); k++)
      {
        square = square->connect[(HEX_RIGHT_DOWN+j)%6];

        hex_horizon_odd[index][0] = square->x - ibase;
        hex_horizon_odd[index][1] = square->y - jbase;
        index++;
      }
    }
  }

  hex_horizon_number = index;
}


/*************************************************************************/
/**	init_hex							**/
/** Initialize array for use in getsquare with hexagonal board		**/
/**	Greg Lesher (lesher@cns.bu.edu)					**/
/*************************************************************************/
init_hex (squaresize)
  int squaresize;
{
  int i, j,
      int_angle, mod_angle;

  double xpos, ypos,
         angle,
         atan2();

  hex_side = squaresize;
  hex_halfside = hex_side/2;
  hex_quarterside = hex_side/4;
  hex_3quarterside = (3*hex_side)/4;
  hex_vert = (int)(SQ3D2 * hex_side + 0.5);
  hex_halfvert = hex_vert/2;

  hex_slope = ((double)(hex_vert))/hex_halfside;

  hex_points[0].x = 0;
  hex_points[0].y = 0;

  hex_points[1].x = hex_side;
  hex_points[1].y = 0;

  hex_points[2].x = hex_halfside;
  hex_points[2].y = hex_vert;

  hex_points[3].x = -hex_halfside;
  hex_points[3].y = hex_vert;

  hex_points[4].x = -hex_side;
  hex_points[4].y = 0;

  hex_points[5].x = -hex_halfside;
  hex_points[5].y = -hex_vert;

  hex_points[6].x = hex_halfside;
  hex_points[6].y = -hex_vert;

  for (i=0; i<2*hex_side; i++)
  {
    for (j=0; j<2*hex_side; j++)
    {
      xpos = i - hex_side;
      ypos = j - hex_side;

      if ((i - hex_side) == 0)
      {
        if (ypos > 0)
          angle = 90.0;
        else
          angle = 270.0;
      }
      else
      {
        angle = atan2 (ypos, xpos);
        angle = angle * 180.0 / 3.1415927;
      }
      
      if (angle < 0)
        angle = 360.0 + angle;

      int_angle = (int)(angle + 0.5);
      mod_angle = int_angle/60;

      switch (mod_angle)
      {
        case 0:
          hex_chart[i][j][0] = HEX_RIGHT_UP;
          break;
        case 1:
          hex_chart[i][j][0] = HEX_UP;
          break;
        case 2:
          hex_chart[i][j][0] = HEX_LEFT_UP;
          break;
        case 3:
          hex_chart[i][j][0] = HEX_LEFT_DOWN;
          break;
        case 4:
          hex_chart[i][j][0] = HEX_DOWN;
          break;
        case 5:
          hex_chart[i][j][0] = HEX_RIGHT_DOWN;
          break;
      }

      hex_chart[i][j][1] = -1;
      mod_angle = int_angle/15;

      switch (mod_angle)
      {
        case 0:
          hex_chart[i][j][1] = HEX_RIGHT_DOWN;
          break;
        case 3:
          hex_chart[i][j][1] = HEX_UP;
          break;
        case 4:
          hex_chart[i][j][1] = HEX_RIGHT_UP;
          break;
        case 7:
          hex_chart[i][j][1] = HEX_LEFT_UP;
          break;
        case 8:
          hex_chart[i][j][1] = HEX_UP;
          break;
        case 11:
          hex_chart[i][j][1] = HEX_LEFT_DOWN;
          break;
        case 12:
          hex_chart[i][j][1] = HEX_LEFT_UP;
          break;
        case 15:
          hex_chart[i][j][1] = HEX_DOWN;
          break;
        case 16:
          hex_chart[i][j][1] = HEX_LEFT_DOWN;
          break;
        case 19:
          hex_chart[i][j][1] = HEX_RIGHT_DOWN;
          break;
        case 20:
          hex_chart[i][j][1] = HEX_DOWN;
          break;
        case 23:
          hex_chart[i][j][1] = HEX_RIGHT_UP;
          break;
      }

      if (xpos*xpos + ypos*ypos < 2*CSIZE*CSIZE)
        hex_chart[i][j][0] = -1;

      if (xpos*xpos + ypos*ypos < 2*CSIZE*CSIZE)
        hex_chart[i][j][1] = -1;
    }
  }
}


/*************************************************************************/
/**	set_square_connections						**/
/** Initialize connections for square board				**/
/**	Greg Lesher (lesher@cns.bu.edu)					**/
/*************************************************************************/
set_square_connections (board)
  square_type *board;
{
  int i, j;

  for (j=0; j<boardsizey; j++)
  {
    for (i=0; i<boardsizex; i++)
    {

      if (j != 0)
        SQUARE(board,i,j)->connect[UP] = SQUARE(board,i,j-1);
      else
        SQUARE(board,i,j)->connect[UP] = SQUARE(board,i,j);

      if (j != boardsizey-1)
        SQUARE(board,i,j)->connect[DOWN] = SQUARE(board,i,j+1);
      else
        SQUARE(board,i,j)->connect[DOWN] = SQUARE(board,i,j);

      if (i != 0)
        SQUARE(board,i,j)->connect[LEFT] = SQUARE(board,i-1,j);
      else
        SQUARE(board,i,j)->connect[LEFT] = SQUARE(board,i,j);

      if (i != boardsizex-1)
        SQUARE(board,i,j)->connect[RIGHT] = SQUARE(board,i+1,j);
      else
        SQUARE(board,i,j)->connect[RIGHT] = SQUARE(board,i,j);
    }
  }

  if (enable_anywrap)
  {
    for (i=0; i<boardsizex; i++)
    {
      SQUARE(board,i,0)->connect[UP] = SQUARE(board,i,boardsizey-1);
      SQUARE(board,i,boardsizey-1)->connect[DOWN] = SQUARE(board,i,0);
    }

    for (j=0; j<boardsizey; j++)
    {
      SQUARE(board,0,j)->connect[LEFT] = SQUARE(board,boardsizex-1,j);
      SQUARE(board,boardsizex-1,j)->connect[RIGHT] = SQUARE(board,0,j);
    }
  }
}


/*************************************************************************/
/**	set_hex_connections						**/
/** Initialize connections for hexagonal board				**/
/**	Greg Lesher (lesher@cns.bu.edu)					**/
/*************************************************************************/
set_hex_connections (board)
  square_type *board;

{
  int i, j,
      even;

  for (j=0; j<boardsizey; j++)
  {
    even = TRUE;

    for (i=0; i<boardsizex; i++)
    {
      if (j != 0)
        SQUARE(board,i,j)->connect[HEX_UP] = SQUARE(board,i,j-1);
      else
        SQUARE(board,i,j)->connect[HEX_UP] = SQUARE(board,i,j);

      if (j != boardsizey-1)
        SQUARE(board,i,j)->connect[HEX_DOWN] = SQUARE(board,i,j+1);
      else
        SQUARE(board,i,j)->connect[HEX_DOWN] = SQUARE(board,i,j);

      if (i != 0)
      {
        if (j != 0)
        {
          if (even)
            SQUARE(board,i,j)->connect[HEX_LEFT_UP] = SQUARE(board,i-1,j-1);
          else
            SQUARE(board,i,j)->connect[HEX_LEFT_UP] = SQUARE(board,i-1,j);
        }
        else if (!even)
          SQUARE(board,i,j)->connect[HEX_LEFT_UP] = SQUARE(board,i-1,j);
        else
          SQUARE(board,i,j)->connect[HEX_LEFT_UP] = SQUARE(board,i,j);

        if (j != boardsizey-1)
        {
          if (even)
            SQUARE(board,i,j)->connect[HEX_LEFT_DOWN] = SQUARE(board,i-1,j);
          else
            SQUARE(board,i,j)->connect[HEX_LEFT_DOWN] = SQUARE(board,i-1,j+1);
        }
        else if (even)
          SQUARE(board,i,j)->connect[HEX_LEFT_DOWN] = SQUARE(board,i-1,j);
        else
          SQUARE(board,i,j)->connect[HEX_LEFT_DOWN] = SQUARE(board,i,j);
      }
      else
      {
        SQUARE(board,i,j)->connect[HEX_LEFT_UP] = SQUARE(board,i,j);
        SQUARE(board,i,j)->connect[HEX_LEFT_DOWN] = SQUARE(board,i,j);
      }

      if (i != boardsizex-1)
      {
        if (j != 0)
        {
          if (even)
            SQUARE(board,i,j)->connect[HEX_RIGHT_UP] = SQUARE(board,i+1,j-1);
          else
            SQUARE(board,i,j)->connect[HEX_RIGHT_UP] = SQUARE(board,i+1,j);
        }
        else if (!even)
          SQUARE(board,i,j)->connect[HEX_RIGHT_UP] = SQUARE(board,i+1,j);
        else
          SQUARE(board,i,j)->connect[HEX_RIGHT_UP] = SQUARE(board,i,j);

        if (j != boardsizey-1)
        {
          if (even)
            SQUARE(board,i,j)->connect[HEX_RIGHT_DOWN] = SQUARE(board,i+1,j);
          else
            SQUARE(board,i,j)->connect[HEX_RIGHT_DOWN] = SQUARE(board,i+1,j+1);
        }
        else if (even)
          SQUARE(board,i,j)->connect[HEX_RIGHT_DOWN] = SQUARE(board,i+1,j);
        else
          SQUARE(board,i,j)->connect[HEX_RIGHT_DOWN] = SQUARE(board,i,j);

      }
      else
      {
        SQUARE(board,i,j)->connect[HEX_RIGHT_UP] = SQUARE(board,i,j);
        SQUARE(board,i,j)->connect[HEX_RIGHT_DOWN] = SQUARE(board,i,j);
      }

      even = !even;
    }
  }

  if (enable_anywrap)
  {
    for (i=0; i<boardsizex; i++)
    {
      SQUARE(board,i,0)->connect[HEX_UP] = SQUARE(board,i,boardsizey-1);
      SQUARE(board,i,boardsizey-1)->connect[HEX_DOWN] = SQUARE(board,i,0);

      if (i%2 == 1)
      {
        if (i!=0)
          SQUARE(board,i,0)->connect[HEX_LEFT_UP] = SQUARE(board,i-1,boardsizey-1);

        if (i!=boardsizex-1)
          SQUARE(board,i,0)->connect[HEX_RIGHT_UP] = SQUARE(board,i+1,boardsizey-1);

        if (i!=0)
          SQUARE(board,i,boardsizey-1)->connect[HEX_LEFT_DOWN] = SQUARE(board,i-1,0);

        if (i!=boardsizex-1)
          SQUARE(board,i,boardsizey-1)->connect[HEX_RIGHT_DOWN] = SQUARE(board,i+1,0);
      }
    }

    if (boardsizex%2 == 0)
    {
      for (j=0; j<boardsizey; j++)
      {
        if (j==0)
          SQUARE(board,0,j)->connect[HEX_LEFT_UP] = SQUARE(board,boardsizex-1,boardsizey-1);
        else
          SQUARE(board,0,j)->connect[HEX_LEFT_UP] = SQUARE(board,boardsizex-1,j-1);

        SQUARE(board,0,j)->connect[HEX_LEFT_DOWN] = SQUARE(board,boardsizex-1,j);

        SQUARE(board,boardsizex-1,j)->connect[HEX_RIGHT_UP] = SQUARE(board,0,j);

        if (j == boardsizey-1)
          SQUARE(board,boardsizex-1,j)->connect[HEX_RIGHT_DOWN] = SQUARE(board,0,0);
        else
          SQUARE(board,boardsizex-1,j)->connect[HEX_RIGHT_DOWN] = SQUARE(board,0,j+1);
      }
    }
  }
}


/*************************************************************************/
/**	init_unseen							**/
/** Make all squares unseen						**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
init_unseen (board)
  square_type *board;
{
  int i, j, k;

  for (i=0; i<boardsizex; i++)
    for (j=0; j<boardsizey; j++)
      for (k=0; k<nsides; k++)
        if (enable_map[k] || enable_localmap[k])
          SQUARE(board,i,j)->seen[k] = FALSE;
}


/*************************************************************************/
/**	init_hills							**/
/** set the hills on the game board.  Hills take energy  to climb, but	**/
/** return it again on the way down,  so they can  be used to tactical	**/
/** advantage.								**/
/**									**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
init_hills(board,seed)
  square_type *board;
  int seed;
{
  int i, j,
      offset;
  double limit;

  if (enable_hills)
    limit = (double)(NHILLTONES);
  if (enable_forest)
    limit = (double)(NFORESTTONES);
  else if (enable_sea && !enable_hills && !enable_forest)
    limit = sea;

  /**** set initial random values ****/
  for (i=0; i<boardsizex; i++)
  {
    for (j=0; j<boardsizey; j++)
    {
      SQUARE(board,i,j)->value[none] = (int)(drand48()*limit);
    }
  }

  if (enable_border && enable_sea)
  {
    for (i=0; i<boardsizex; i++)
    {
     SQUARE(board,i,0)->value[none] = 0;
     SQUARE(board,i,boardsizey-1)->value[none] = 0;
    }
    for (j=0; j<boardsizey; j++)
    {
     SQUARE(board,0, j)->value[none] = 0;
     SQUARE(board,boardsizex-1,j)->value[none] = 0;
    }

    offset = 2;
  }
  else
    offset = 1;
  
  

  /**** smooth out the hills ****/
  for (j=offset; j<boardsizey-offset; j++)
  {
    for (i=offset; i<boardsizex-offset; i++)
    {
      if (SQUARE(board,i,j+1)->value[none] == SQUARE(board,i,j-1)->value[none])
	SQUARE(board,i,j)->value[none] = SQUARE(board,i,j-1)->value[none];
      if (SQUARE(board,i+1,j)->value[none] == SQUARE(board,i-1,j)->value[none])
	SQUARE(board,i,j)->value[none] = SQUARE(board,i-1,j)->value[none];
    }
  }

  /**** if just sea enabled, flatten all hills to 2nd lowest level ****/
  if (enable_sea && !enable_hills && !enable_forest)
    for (i=0; i<boardsizex; i++)
      for (j=0; j<boardsizey; j++)
        if (SQUARE(board,i,j)->value[none] == 0)
          SQUARE(board,i,j)->value[none] = 0;
        else
          SQUARE(board,i,j)->value[none] = 1;
}


/*************************************************************************/
/**	init_towns							**/
/** set the towns on the game board.  Towns are sources of troops, and	**/
/** appear as circles  whose radius  is  proportional  to the  rate of	**/
/** troop production.		  					**/
/**									**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
init_towns(board,val,seed)
  square_type *board;
  double val;
  int seed;
{
  int i, j;

  /**** set the random towns ****/
  for (i=0; i<boardsizex; i++)
  {
    for (j=0; j<boardsizey; j++)
    {
      if (drand48() <= val/40.0)
      {
         if (!(enable_sea && SQUARE(board,i,j)->value[none] <= 0))
         {
	   SQUARE(board,i,j)->growth =  (int)(drand48()*40.0+0.5)+60;
	   SQUARE(board,i,j)->oldgrowth = SQUARE(board,i,j)->growth;
	   SQUARE(board,i,j)->angle = 23040; 
         }
       }
    }
  }
}


/*************************************************************************/
/**	init_farms							**/
/** set the  farms on  the   game board.  Farms   produce troops  at a	**/
/** constant slow rate val, and  must be cultivated  in large areas to	**/
/** be effective.							**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/*************************************************************************/
init_farms(board,val)
  square_type *board;
  double val;
{
  int i, j;

  /**** set growth values ****/
  for (i=0; i<boardsizex; i++)
  {
    for (j=0; j<boardsizey; j++)
    {
      if (!(enable_sea && SQUARE(board,i,j)->value[none] <= 0))
      {
        if (SQUARE(board,i,j)->growth < 50);
          SQUARE(board,i,j)->growth = (int)val;
      }
    }
  }
}


/*************************************************************************/
/**	init_bases							**/
/** set the bases on the game board.  Bases provide a steady supply of	**/
/** a large  number  of troops, and thus   are   of enormous strategic	**/
/** advantage.								**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
init_bases(board,n)
  square_type *board;
  int n[];
{
  int i, sep[MAXSIDES];
  
  /**** separation between bases ****/
  for (i=0; i<nsides; i++)
    sep[i] = (boardsizex-n[i])/2;

  /**** initialize the bases ****/
  if (nsides >= 1)
  {
    for (i=sep[0]; i<sep[0]+n[0]; i++)
    {
      if (!(enable_sea && SQUARE(board,i,1)->value[none] <= 0))
      {
        SQUARE(board,i,1)->color  = 0;
        SQUARE(board,i,1)->value[0]  = 1;
        SQUARE(board,i,1)->growth = 100;
        SQUARE(board,i,1)->oldgrowth = 100;
        SQUARE(board,i,1)->angle = 23040; 
      }
    }
  }

  if (nsides >= 2)
  {
    for (i=sep[1]; i<sep[1]+n[1]; i++)
    {
      if (!(enable_sea && SQUARE(board,i,boardsizey-2)->value[none] <= 0))
      {
        SQUARE(board,i,boardsizey-2)->color  = 1;
        SQUARE(board,i,boardsizey-2)->value[1]  = 1;
        SQUARE(board,i,boardsizey-2)->growth = 100;
        SQUARE(board,i,boardsizey-2)->oldgrowth = 100;
        SQUARE(board,i,boardsizey-2)->angle = 23040; 
      }
    }
  }

  if (nsides >= 3)
  {
    for (i=sep[2]; i<sep[2]+n[2]; i++)
    {
      if (!(enable_sea && SQUARE(board,1,i)->value[none] <= 0))
      {
        SQUARE(board,1,i)->color  = 2;
        SQUARE(board,1,i)->value[2]  = 1;
        SQUARE(board,1,i)->growth = 100;
        SQUARE(board,1,i)->oldgrowth = 100;
        SQUARE(board,1,i)->angle = 23040; 
      }
    }
  }

  if (nsides >= 4)
  {
    for (i=sep[3]; i<sep[3]+n[3]; i++)
    {
      if (!(enable_sea && SQUARE(board,boardsizex-2,i)->value[none] <= 0))
      {
        SQUARE(board,boardsizex-2,i)->color  = 3;
        SQUARE(board,boardsizex-2,i)->value[3]  = 1;
        SQUARE(board,boardsizex-2,i)->growth = 100;
        SQUARE(board,boardsizex-2,i)->oldgrowth = 100;
        SQUARE(board,boardsizex-2,i)->angle = 23040; 
      }
    }
  }
}


/*************************************************************************/
/**	init_rbases							**/
/** set the bases on the game board.  Bases provide a steady supply of	**/
/** a large  number  of troops, and thus   are   of enormous strategic	**/
/** advantage.								**/
/**	Mark Lauer (elric@cs.su.oz.au)					**/
/** Modified to handle multiple players					**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
init_rbases(board,n,seed)
  square_type *board;
  int n[],
      seed;
{
  int i, j, k, l, x, y,
      i1, i2, j1, j2,
      range,
      maxn,
      done,
      count;

  /**** create randomly placed bases for each side ****/

  maxn = 0;
  for (i=0; i<nsides; i++)
    if (n[i] > maxn)
      maxn = n[i];

  if (maxviewrange < BASERANGE)
    range = BASERANGE;
  else
    range = maxviewrange;

  for (k=0; k<maxn; k++)
  {
    for (l=0; l<nsides; l++)
    {
      if (k >= n[l])
        continue;

      count = 0;
      do
      {
        done = TRUE;
        i = (int)(drand48()*boardsizex);
        j = (int)(drand48()*boardsizey);

        if (SQUARE(board,i,j)->growth > 50 || (enable_sea && SQUARE(board,i,j)->value[none] <= 0))
          done = FALSE+FALSE;

        i1 = i-range;
        i2 = i+range;
        j1 = j-range;
        j2 = j+range;

        /**** search for nearby squares of color color ****/
        if (enable_anywrap)
        {
          for (x=i1; x<=i2; x++)
            for (y=j1; y<=j2; y++)
              if (SQUARE(board,MODX(x),MODY(y))->color != l &&
                               SQUARE(board,MODX(x),MODY(y))->color != none) 
                done = FALSE;
        }
        else
        {
          i1 = (i1<0) ? 0 : i1;
          i2 = (i2>boardsizex-1) ? boardsizex-1 : i2;
          j1 = (j1<0) ? 0 : j1;
          j2 = (j2>boardsizey-1) ? boardsizey-1 : j2;

          for (x=i1; x<=i2; x++)
            for (y=j1; y<=j2; y++)
              if (SQUARE(board,x,y)->color != l && SQUARE(board,x,y)->color != none)
                done = FALSE;
        }
        count++;

        if (count > 50 && done!=(FALSE+FALSE))
          done = TRUE;

      } while (!done);

      SQUARE(board,i,j)->color = l;
      SQUARE(board,i,j)->value[l] = 1;
      SQUARE(board,i,j)->growth = 100;
      SQUARE(board,i,j)->oldgrowth = 100;
      SQUARE(board,i,j)->angle = 23040; 
    }
  }
}


/*************************************************************************/
/**	init_armies							**/
/** set the armies on the game board					**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
init_armies(board,n)
  square_type *board;
  int n[];
{
  int i, sep[MAXSIDES];
  
  for (i=0; i<nsides; i++)
    sep[i] = (boardsizex-n[i])/2;

  /**** initialize armies for each side ****/

  if (nsides >= 1)
  {
    for (i=sep[0]; i<sep[0]+n[0]; i++)
    {
      if (!(enable_sea && SQUARE(board,i,2)->value[none] <= 0))
      {
        SQUARE(board,i,2)->color  = 0;
        SQUARE(board,i,2)->value[0]  = MAXVAL;
      }
    }
  }

  if (nsides >= 2)
  {
    for (i=sep[1]; i<sep[1]+n[1]; i++)
    {
      if (!(enable_sea && SQUARE(board,i,boardsizey-3)->value[none] <= 0))
      {
        SQUARE(board,i,boardsizey-3)->color  = 1;
        SQUARE(board,i,boardsizey-3)->value[1]  = MAXVAL;
      }
    }
  }

  if (nsides >= 3)
  {
    for (i=sep[2]; i<sep[2]+n[2]; i++)
    {
      if (!(enable_sea && SQUARE(board,2,i)->value[none] <= 0))
      {
        SQUARE(board,2,i)->color  = 2;
        SQUARE(board,2,i)->value[2]  = MAXVAL;
      }
    }
  }

  if (nsides >= 4)
  {
    for (i=sep[3]; i<sep[3]+n[3]; i++)
    {
      if (!(enable_sea && SQUARE(board,boardsizex-3,i)->value[none] <= 0))
      {
        SQUARE(board,boardsizex-3,i)->color  = 3;
        SQUARE(board,boardsizex-3,i)->value[3]  = MAXVAL;
      }
    }
  }
}


/*************************************************************************/
/**	init_militia							**/
/** set random militia on the game board. 				**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
init_militia(board,val,seed)
  square_type *board;
  double val[];
  int seed;
{
  int i, j, k, l,
      scramble;

  double roll,
         thresh;

  scramble = 0;
  thresh = 1.0/((double)(nsides));

  /**** set the random militia ****/
  for (i=0; i<boardsizex; i++)
  {
    for (j=0; j<boardsizey; j++)
    {
      scramble += 217;

      for (k=0; k<nsides; k++)
      {
        l = (scramble+k)%nsides;

        if (drand48() <= val[l]/40.0)
        {
          if (!(enable_sea && SQUARE(board,i,j)->value[none] <= 0))
          {
	    SQUARE(board,i,j)->color =  l;
	    SQUARE(board,i,j)->value[l]  = (int)(drand48()*(double)MAXVAL);
            k = nsides;
          }
        }
      }
    }
  }
}


edit_board (board)
  square_type *board;
{
  XEvent event;

  int i,
      player,
      currentcolor,
      value,
      x, y,
      textcount,
      tdir[DIRS];

  char text[20];

  KeySym key;

  square_type *square;

  player = 0;
  currentcolor = colorarray[player];

  drawboard (board, player, TRUE);

  while (TRUE)
  {
    XNextEvent(xwindow[player]->display, &event);

    getsquare(event.xbutton.x,event.xbutton.y,&x,&y,tdir,squaresize[colorarray[player]], board, FALSE);
    square = SQUARE(board,x,y);

    switch (event.type)
    {
      case Expose:
        drawboard (board, player, TRUE);
        break;

      case ButtonPress:
        switch (event.xbutton.button)
        {
          case Button1:
          case Button2:
          case Button3:
            if (event.xbutton.button == Button1)
              square->value[none] -= 1;
            else if (event.xbutton.button == Button2)
              square->value[none] += 1;
            else
            {
              if (square->value[none] != 0)
                square->value[none] = 0;
              else
              {
                if (enable_hills)
                  square->value[none] = NHILLTONES/2;
                else if (enable_forest)
                  square->value[none] = NFORESTTONES/2;
                else if (enable_sea)
                  square->value[none] = 1;
              }
            }

            if (square->value[none] < 0)
              square->value[none] = 0;
            else if (enable_hills)
            {
              if (square->value[none] >= NHILLTONES)
                square->value[none] = NHILLTONES-1;
            }
            else if (enable_forest)
            {
              if (square->value[none] >= NFORESTTONES)
                square->value[none] = NFORESTTONES-1;
            }
            else if (enable_sea)
            {
              if (square->value[none] >= NSEATONES)
                square->value[none] = NSEATONES-1;
            }
            else
              square->value[none] = 0;

            if (enable_sea && square->value[none] == 0)
            {
              if (square->color != none)
              {
                square->value[square->color] = 0;
                square->color = none;
              }

              square->growth = 0;
              square->oldgrowth = 0;
              square->angle = 0;
            }

            drawsquare(xwindow[player], square, colorarray[player],
                                        squaresize[colorarray[player]]);
            break;
        } 
        break;

      case KeyPress:
        
        textcount = XLookupString(&event, text, 10, &key, NULL);
        if (textcount == 0)
          break;

        switch (text[0])
        {
          case 'c':
          case 't':
          case 'v':
            if (enable_sea && square->value[none] <= 0)
              break;
          
            if (text[0] == 'c')
              square->growth = 100;
            else if (text[0] == 't')
              square->growth = 80;
            else
              square->growth = 60;

            square->oldgrowth = square->growth;
            square->angle = 23040;
            break;

          case 's':
            if (square->oldgrowth > 50)
            {
              square->growth = 0;
              square->angle -= 2304;

              if (square->angle < 10)
              {
                square->growth = 0;
                square->oldgrowth = 0;
                square->angle = 0;
              }
            }
            break;

          case 'b':
            if (square->oldgrowth > 50)
            {
              square->angle += 2304;

              if (square->angle > 23030)
              {
                square->growth = square->oldgrowth;
                square->angle = 23040;
              }
            }
            break;

          case 'j':
            if (square->growth > 50)
            {
              square->growth *= 0.95;
              square->oldgrowth = square->growth;
            }

            if (square->growth < 60)
            {
              square->growth = 0;
              square->oldgrowth = 0;
              square->angle = 0;
            }
            break;

          case 'k':
            if (square->growth > 50)
            {
              square->growth *= 1.05;
              square->oldgrowth = square->growth;
            }

            if (square->growth > 100)
            {
              square->growth = 100;
              square->oldgrowth = 100;
            }
            break;

          case 'r':
            currentcolor += 1;
            if (currentcolor >= nsides)
              currentcolor = 0;
            break;

          case 'f':
            if (square->color != none)
            {
              value = square->value[square->color];
              square->value[square->color] = 0;
              square->color += 1;
              if (square->color >= nsides)
                square->color = 0;
              square->value[square->color] = value;
            }
            break;

          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
          case '0':
          case '[':
          case ']':
            if (enable_sea && square->value[none] <= 0)
              break;

            if (square->color == none)
              square->color = currentcolor;

            if (text[0] == ']')
              square->value[square->color] += 1;
            else if (text[0] == '[')
            {
              if (square->value[square->color] > 0)
                square->value[square->color] -= 1;
            }
            else
              square->value[square->color] = (text[0] - '0')*MAXVAL/9;

            if (square->value[square->color] == 0)
              square->color = none;
            else if (square->value[square->color] > MAXVAL)
                square->value[square->color] = MAXVAL;
            break;

          case 'd':
            dump_board (board, mapname);
            drawboard (board, player, TRUE);
            break;

          case 'l':
            load_board (board, mapname, TRUE);
            drawboard (board, player, TRUE);
            break;

          case 'e':
            exit (0);
            break;
        }

        drawsquare(xwindow[player], square, colorarray[player],
                                        squaresize[colorarray[player]]);

        break;
    }
  }
}


dump_board (board, filename)
  square_type *board;
  char filename[];
{
  int i, j;

  FILE *fp,
       *fopen();

  if ((fp = fopen (filename, "w")) == NULL)
  {
    printf ("Unable to open map file: %s\n", filename);
    exit (0);
  }

  fwrite (&boardsizex, sizeof(int), 1, fp);
  fwrite (&boardsizey, sizeof(int), 1, fp);
  fwrite (&enable_hex, sizeof(int), 1, fp);
  fwrite (&enable_hills, sizeof(int), 1, fp);
  fwrite (&enable_forest, sizeof(int), 1, fp);
  fwrite (&enable_sea, sizeof(int), 1, fp);
  fwrite (&enable_terrain, sizeof(int), 1, fp);
  fwrite (&none, sizeof(int), 1, fp);

  for (i=0; i<boardsizex; i++)
    for (j=0; j<boardsizey; j++)
      fwrite (SQUARE(board,i,j), sizeof(square_type), 1, fp);

  fwrite (&nplayers, sizeof(int), 1, fp);

  fclose (fp);
}


load_board (board, filename, friendly)
  square_type *board;
  char filename[];
  int friendly;
{
  int i, j,
      temp_enable_hills,
      temp_enable_forest,
      temp_enable_sea,
      temp_enable_terrain,
      temp_none;

  FILE *fp,
       *fopen();

  if ((fp = fopen (filename, "r")) == NULL)
  {
    printf ("Unable to open map file: %s\n", filename);
    exit (0);
  }

  fread (&boardsizex, sizeof(int), 1, fp);
  fread (&boardsizey, sizeof(int), 1, fp);
  fread (&enable_hex, sizeof(int), 1, fp);
  fread (&temp_enable_hills, sizeof(int), 1, fp);
  fread (&temp_enable_forest, sizeof(int), 1, fp);
  fread (&temp_enable_sea, sizeof(int), 1, fp);
  fread (&temp_enable_terrain, sizeof(int), 1, fp);
  fread (&temp_none, sizeof(int), 1, fp);

  for (i=0; i<boardsizex; i++)
    for (j=0; j<boardsizey; j++)
      fread (SQUARE(board,i,j), sizeof(square_type), 1, fp);

  for (i=0; i<boardsizex; i++)
    for (j=0; j<boardsizey; j++)
    {
      SQUARE(board,i,j)->value[none] = SQUARE(board,i,j)->value[temp_none];

      if (none > temp_none)
        SQUARE(board,i,j)->value[temp_none] = 0;

      if (SQUARE(board,i,j)->color > none)
      {
        SQUARE(board,i,j)->value[SQUARE(board,i,j)->color] = 0;
        SQUARE(board,i,j)->color = none; 
      }
      else if (SQUARE(board,i,j)->color == temp_none)
      {
        SQUARE(board,i,j)->color = none; 
      }
    }

  if (!friendly)
  {
    enable_hills = temp_enable_hills;
    enable_forest = temp_enable_forest;
    enable_sea = temp_enable_sea;
    enable_terrain = temp_enable_terrain;
  }

  fclose (fp);
}
