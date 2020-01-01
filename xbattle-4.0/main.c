#include <stdio.h>
  
/**** x include files ****/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include "global.h"

#if UNIX
#include <sys/types.h>
#include <sys/time.h>
#endif

/*************************************************************************/
/**	xbattle								**/
/**									**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/**     Greg Lesher (lesher@park.bu.edu)				**/
/** with horizon and sea contribution from				**/
/**     Mark Lauer (elric@cs.su.oz.au)                                  **/
/**									**/
/*************************************************************************/

main(argc,argv)
  int argc;
  char *argv[];
{ 
  int i,j,t,p,
      winxsize[MAXSIDES],
      winysize[MAXSIDES];

  long eventmask;
  XEvent event;
  char displayname[MAXPLAYERS][80],
       gtitle[20], htitle[20];
  square_type *board;
#if UNIX
  int fd, maxfd;
  int selectback;
  unsigned long newtime, oldtime, targettime;
  fd_set rfds, disp_fds;
  struct timeval tval;
  struct timeval tp_old;
  struct timezone tzp;
#endif
  
  /**** getarg functions ****/
  int intval(),is_arg();
  double doubleval();
  int stringval();

  /**** parse the command line ****/

  init_defaults();
  checkoptions(argc,argv);
  get_displaynames(displayname,colorarray,rcolorarray,argc,argv);
  init_options(displayname,argc,argv);
  clean_options (displayname);

  if (enable_hex)
  {
    init_hex (squaresize[0]);

    for (i=0; i<nsides; i++)
    {
      for (j=0; j<nsides; j++)
#if MULTITEXT
      textyh[i][j]=boardsizey*2*hex_vert + hex_vert + 12 + 16*j,
#else
      textyh[i][j]=boardsizey*2*hex_vert + hex_vert + 12,
      textyl[i][j]=boardsizey*2*hex_vert + hex_vert + 28;
#endif

      winxsize[i] = (boardsizex-1)*3*(hex_halfside) + 2*hex_side;

#if MULTITEXT
      winysize[i] = boardsizey*2*hex_vert + hex_vert + nsides*TEXTSIZE;
#else
      winysize[i] = boardsizey*2*hex_vert + hex_vert + 2*TEXTSIZE;
#endif
    }
  }
  else
  { 
    for (i=0; i<nsides; i++)
    {
      for (j=0; j<nsides; j++)
#if MULTITEXT
      textyh[i][j]=boardsizey*squaresize[i]+12+16*j,
#else
      textyh[i][j]=boardsizey*squaresize[i]+12,
      textyl[i][j]=boardsizey*squaresize[i]+28;
#endif

      winxsize[i] = boardsizex*squaresize[i];
#if MULTITEXT
      winysize[i] = boardsizey*squaresize[i] + nsides*TEXTSIZE;
#else
      winysize[i] = boardsizey*squaresize[i] + 2*TEXTSIZE;
#endif
    }
  } 

  /**** allocate the boards ****/
  board  = (square_type *)(malloc(boardsizex*boardsizey*sizeof(square_type)));

  init_board (board);
  
  /**** open x windows ****/
#if UNIX
  maxfd = 0;
  FD_ZERO(&disp_fds);
#endif

  if (enable_bound)
    eventmask = ButtonPressMask|ButtonReleaseMask|ButtonMotionMask|KeyPressMask|ExposureMask;
  else
    eventmask = ButtonPressMask|ButtonMotionMask|KeyPressMask|ExposureMask;

  for (p=0; p<nplayers; p++)
  {
    xwindow[p]  = (xwindow_type*)malloc(sizeof(xwindow_type));

    strcpy (gtitle, graytitle[color2bw[sidemap[colorarray[p]]]]);
    strcpy (htitle, huetitle[sidemap[colorarray[p]]]);

    if (lettermap[colorarray[p]][0])
      strcpy (gtitle, htitle);

    if (enable_manpos[colorarray[p]])
      open_xwindow(xwindow[p],displayname[p],-1,-1,
		          winxsize[colorarray[p]],winysize[colorarray[p]],
                          gtitle, htitle, eventmask,squaresize[colorarray[p]]);
    else
      open_xwindow(xwindow[p],displayname[p],XPOS,YPOS,
		          winxsize[colorarray[p]],winysize[colorarray[p]],
                          gtitle, htitle, eventmask,squaresize[colorarray[p]]);

#if UNIX
    if ((fd = ConnectionNumber (xwindow[p]->display)) > maxfd)
      maxfd = fd;
    FD_SET (fd, &disp_fds); 
#endif
  }  

  if (enable_edit)
    edit_board(board);

  /**** replay game ****/
  if (enable_replay)
    replaygame(squaresize[0], board);
  
  /************* main event loop *************/
#if UNIX
  XSetIOErrorHandler(1);
  gettimeofday (&tp_old, &tzp);
  oldtime = (tp_old.tv_sec%10000)*1000000 + tp_old.tv_usec;
  targettime = oldtime + (delay/10000)*1000000 + (delay%10000)*100;

  while (TRUE)
  {
    /**** check for events ****/
    rfds = disp_fds;

    tval.tv_sec = (targettime-oldtime)/1000000;
    tval.tv_usec = (targettime-oldtime)%1000000;

    selectback = select (maxfd+1, &rfds, NULL, NULL, &tval);

    gettimeofday (&tp_old, &tzp);
    newtime = (tp_old.tv_sec%10000)*1000000 + tp_old.tv_usec;

    if (newtime < oldtime)
      targettime = newtime - 1;

    switch (selectback)
    {
      case -1:
        perror("select()");
        exit (1);
        break;

      case 0:
        for (i=0; i<nplayers; i++)
        {
          if (winopen[i])
          {
            XFlush(xwindow[i]->display);
            while (XEventsQueued(xwindow[i]->display, QueuedAfterReading) > 0)
            {
	      XNextEvent(xwindow[i]->display, &event);
	      process_event(event,board,colorarray,i);
            }

#if MULTIFLUSH
            XFlush(xwindow[i]->display);
#endif
          }
        }
        break;

      default:
        for (i=0; i<nplayers; i++)
        {
          if (winopen[i])
          {
            if (FD_SET(fd = ConnectionNumber(xwindow[i]->display), &rfds))
            {
              while (winopen[i] &&
                    XEventsQueued(xwindow[i]->display, QueuedAfterReading) > 0)
         
              {
	        XNextEvent(xwindow[i]->display, &event);
	        process_event(event,board,colorarray,i);
              }

#if MULTIFLUSH
              XFlush(xwindow[i]->display);
#endif

              if (!winopen[i])
                FD_CLR (fd, &disp_fds);
            }
          }
        }
        break;
    }

    if (newtime > targettime)
    {
      if (enable_march)
        march(board);
      update_board(board);
      targettime = newtime + (delay/10000)*1000000 + (delay%10000)*100;
    }
    oldtime = newtime;  
  }
#else
  t = 0;
  while (TRUE)
  {
    /**** check for events ****/
    for (i=0; i<nplayers; i++)
    {
      if (!winopen[i])
        continue;
      if (XEventsQueued(xwindow[i]->display, QueuedAfterReading) > 0)
      {
	XNextEvent(xwindow[i]->display, &event);
	process_event(event,board,colorarray,i);
      }
      else
        XFlush(xwindow[i]->display);
    }
    
    /**** update the game board ****/
    if (t++>delay)
    {
      t = 0;
      if (enable_march)
        march(board);
      update_board(board);
    }
  }
#endif
}


 
/*************************************************************************/
/**	process_event							**/
/** Process the x event from the xwindow of 'player'			**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
process_event(event,board,colorarray,player)
  XEvent event;
  square_type *board;
  int colorarray[MAXPLAYERS], player;
{
  int i, j, k, x, y,
      xpos, ypos,
      textcount,
      limit,
      tdir[DIRS],
      xlow, xhigh, ylow, yhigh,
      status,
      color,
      control,
      shift;
  char text[10],
       line[10];
  double val;
  KeySym key;
  char keyvector[32];
  unsigned int bitindex,
               byteindex;
  KeyCode code;
  square_type *square2;
  
  /**** Process events ****/
  switch (event.type)
  {
    /**** process expose event ****/
    case Expose:
      drawboard(board, player, FALSE);
      break;
    
    /**** process button presses ****/
    case ButtonPress:

      if (winwatch[player])
        break;

#if PAUSE
      if (paused)
        break;
#endif
      
      /**** left button ****/
      switch (event.xbutton.button)
      {
        case Button1:
        case Button2:
          shift = event.xbutton.state & ShiftMask;

          if (enable_personalmarch[colorarray[player]] && shift)
          {
            oldx[player] = -1;
            oldy[player] = -1;

            getsquare(event.xbutton.x,event.xbutton.y,&x,&y,tdir,squaresize[colorarray[player]],board,shift);
               
            color = colorarray[player];

            if (enable_sea && SQUARE(board,x,y)->color==none && SQUARE(board,x,y)->value[none]<=0)
              break;
            else if (SQUARE(board,x,y)->color == color &&
                                     SQUARE(board,x,y)->value[color] > 0) 
            {
              SQUARE(board,x,y)->anymarch = ACTIVE;
              SQUARE(board,x,y)->marchcolor = color;
              SQUARE(board,x,y)->marchcount = 0;
            }
            else
            {
              SQUARE(board,x,y)->march[color] = PASSIVE;
            }

            for (i=0; i<directions; i++)
              SQUARE(board,x,y)->marchdir[color][i] = 0;

            if (event.xbutton.button == Button1)
              SQUARE(board,x,y)->marchtype[color] = SET;
            else
              SQUARE(board,x,y)->marchtype[color] = FORCE;

            for (i=0; i<directions; i++)
              if (tdir[i])
              {
                SQUARE(board,x,y)->marchdir[color][i] = 1;
                i = 5;
              }
            if (SQUARE(board,x,y)->march[color] == PASSIVE)
              outdated[x][y] = ALL;
            break;
          }

          getsquare(event.xbutton.x,event.xbutton.y,&x,&y,dir[player],squaresize[colorarray[player]],
                                                              board,shift);

          if (enable_personalbound[colorarray[player]])
          {
            oldx[player] = x;
            oldy[player] = y;
            break;
          }
        
          /**** move ****/
          if (SQUARE(board,x,y)->color == colorarray[player] ||
               (!enable_disrupt[colorarray[player]] && SQUARE(board,x,y)->color==FIGHT &&
                                SQUARE(board,x,y)->oldcolor == colorarray[player]))
          {
#if VARMOUSE
            if (event.xbutton.button == Button1)
            {
	      moveon(SQUARE(board, x,y),dir[player]);
              dirtype[player] = SET;
            }
            else
            {
	      moveoff(SQUARE(board, x,y),dir[player]);
              dirtype[player] = FORCE;
            }
#else
            if (event.xbutton.button == Button1)
            {
	      setmove(SQUARE(board, x,y),dir[player]);
              dirtype[player] = SET;
            }
            else
            {
	      forcemove(SQUARE(board, x,y),dir[player]);
              dirtype[player] = FORCE;
            }
#endif

            dumpsquare (board, SQUARE(board,x,y));
          }

          if (enable_personalmarch[colorarray[player]])
            if (SQUARE(board,x,y)->march[colorarray[player]] || SQUARE(board,x,y)->anymarch)
            {
              if (SQUARE(board,x,y)->anymarch == ACTIVE &&
                             SQUARE(board,x,y)->marchcolor == colorarray[player])
              {
                SQUARE(board,x,y)->anymarch = FALSE;
                SQUARE(board,x,y)->march[colorarray[player]] = FALSE;
              }
              else
              {
                SQUARE(board,x,y)->march[colorarray[player]] = FALSE;
                outdated[x][y] = ALL;
                dumpsquare (board, SQUARE(board,x,y));
              }
            }

          break;
  
        case Button3:
  
          if (enable_artillery[colorarray[player]] || enable_paratroops[colorarray[player]] ||
                                              enable_repeat[colorarray[player]])
          {
            getsquare(event.xbutton.x,event.xbutton.y,&x,&y,tdir,squaresize[colorarray[player]], board,shift);

            if (SQUARE(board,x,y)->color == colorarray[player])
            {
              shift = event.xbutton.state & ShiftMask;
              control = event.xbutton.state & ControlMask;

              if (shift && enable_paratroops[colorarray[player]])
                para (board, event.xbutton.x, event.xbutton.y, x, y, colorarray);
              else if (control && enable_artillery[colorarray[player]])
                fire (board, event.xbutton.x, event.xbutton.y, x, y, colorarray);
              else if (enable_repeat[colorarray[player]]) 
              {
#if VARMOUSE
                if (dirtype[player] == SET)
	          moveon(SQUARE(board, x,y),dir[player]);
                else
	          moveoff(SQUARE(board, x,y),dir[player]);
#else
                if (dirtype[player] == SET)
	          setmove(SQUARE(board, x,y),dir[player]);
                else
	          forcemove(SQUARE(board, x,y),dir[player]);
#endif
              dumpsquare (board, SQUARE(board,x,y));
              }
            }
          }
          break;
      }
      break;

    case ButtonRelease:
      if (!enable_personalbound[colorarray[player]])
        break;

      if ((oldx[player] == -1) && (oldy[player] == -1))
        break;

      getsquare(event.xbutton.x,event.xbutton.y,&x,&y,tdir,squaresize[colorarray[player]], board,shift);

      if (y >= boardsizey)
        y = boardsizey-1;
      if (x >= boardsizex)
        x = boardsizex-1;

      if (oldx[player] < x)
      {
        xlow = oldx[player]; 
        xhigh = x;
      }
      else
      {
        xlow = x;
        xhigh = oldx[player]; 
      }

      if (oldy[player] < y)
      {
        ylow = oldy[player]; 
        yhigh = y;
      }
      else
      {
        ylow = y;
        yhigh = oldy[player]; 
      }

      switch (event.xbutton.button)
      {
        case Button1:
        case Button2:

          for (i=xlow; i<=xhigh; i++)
          {
            for (j=ylow; j<=yhigh; j++)
            {
              if (SQUARE(board,i,j)->color == colorarray[player] ||
                     (!enable_disrupt[colorarray[player]] && SQUARE(board,i,j)->color==FIGHT &&
                     SQUARE(board,i,j)->oldcolor == colorarray[player]))
              {
#if VARMOUSE
                if (event.xbutton.button == Button1)
	          moveon(SQUARE(board, i,j),dir[player]);
                else
	          moveoff(SQUARE(board, i,j),dir[player]);
#else
                if (event.xbutton.button == Button1)
	          setmove(SQUARE(board, i,j),dir[player]);
                else
	          forcemove(SQUARE(board, i,j),dir[player]);
#endif

                dumpsquare (board, SQUARE(board,i,j));

              }
            }
          }

          break;

        case Button3:
          break;
      }
      break;
  
      /**** process key press events ****/
    case KeyPress:
      textcount = XLookupString(&event, text, 10, &key, NULL);
      if (textcount != 0)
      {
        if ((enable_hex && (event.xbutton.y < boardsizey*2*hex_vert + hex_vert)) ||
               (!enable_hex && (event.xbutton.y < boardsizey*squaresize[colorarray[player]])))
        {
          getsquare(event.xbutton.x,event.xbutton.y,&x,&y,dir[player],
                                     squaresize[colorarray[player]], board,shift);
          if (winwatch[player])
            break;
          switch (text[0])
          {
#if PAUSE
            case CTRLS:
              paused = TRUE;
              break;
            case CTRLQ:
              paused = FALSE;
              break;
#endif
            case 'a':
            case 'A':
              color = colorarray[player];

              if (!enable_attack[color])
                break;

              if (SQUARE(board,x,y)->color == none || SQUARE(board,x,y)->color == color)
                break;

              for (i=0; i<directions; i++)
                tdir[i] = 0;

              for (i=0; i<directions; i++)
              {
                square2 = SQUARE(board,x,y)->connect[i];

                if (square2 != SQUARE(board,x,y) && square2->color == color)
                {
                  tdir[(i+directions/2)%directions] = 1;
                  forcemove (square2, tdir);
                  tdir[(i+directions/2)%directions] = 0;
                }
              }

              break;

            case 'z':
            case 'Z':
              color = colorarray[player];
              if (SQUARE(board,x,y)->color == color)
              {
                for (i=0; i<directions; i++)
                  tdir[i] = 0;
                forcemove (SQUARE(board,x,y), tdir);
              }
              break;

            case 'p':
            case 'P':
              if (!enable_paratroops[colorarray[player]])
                break;

              getsquare(event.xbutton.x,event.xbutton.y,&x,&y,tdir,squaresize[colorarray[player]], board,shift);
              if (SQUARE(board,x,y)->color == colorarray[player])
                para (board, event.xbutton.x, event.xbutton.y, x, y, colorarray);
              break;

            case 'g':
            case 'G':
              if (!enable_artillery[colorarray[player]])
                break;

              getsquare(event.xbutton.x,event.xbutton.y,&x,&y,tdir,squaresize[colorarray[player]], board,shift);
              if (SQUARE(board,x,y)->color == colorarray[player])
                fire (board, event.xbutton.x, event.xbutton.y, x, y, colorarray);
              break;

            case 'd':
            case 'D':
              if (!enable_dig[colorarray[player]])
                break;

              if (SQUARE(board,x,y)->color == colorarray[player])
              {
                if (SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] > digcost &&
                          SQUARE(board,x,y)->growth < 60 )
                {
                  if (SQUARE(board,x,y)->move == 0)
                  {
                    SQUARE(board,x,y)->value[none] -= 1; 

                    if (enable_sea)
                      SQUARE(board,x,y)->value[none] = 1 - fillnumber; 
                    else if (SQUARE(board,x,y)->value[none] < 0) 
                      SQUARE(board,x,y)->value[none] = 0; 

                    SQUARE(board,x,y)->angle = 0;
                    SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] = 0; 
                    SQUARE(board,x,y)->move = 0;
                    for (k=0; k<directions; k++)
                      SQUARE(board,x,y)->dir[k] = 0;
                    SQUARE(board,x,y)->lowbound = 0;
                    outdatehorizon (board, x, y, SQUARE(board,x,y)->color);
                    SQUARE(board,x,y)->color = none; 
                    outdated[x][y] = ALL;
                  }
                  else if (SQUARE(board,x,y)->move == 1)  /** dig partially filled square **/
                  {
                    for (i=0; i<directions; i++)
                    {
                      if (SQUARE(board,x,y)->dir[i])
                        square2 = SQUARE(board,x,y)->connect[i];
                    }

                    if (square2->color == none && square2->value[none] < 0)
                    {
                      square2->value[none] += 1;

                      SQUARE(board,x,y)->move = 0;
                      SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] = 0;
                      for (k=0; k<8; k++)
                        SQUARE(board,x,y)->dir[k] = 0;
                      SQUARE(board,x,y)->lowbound = 0;
                      outdatehorizon (board, square2->x, square2->x, ALL);
                      outdated[x][y] = ALL;
                      outdated[square2->x][square2->y] = ALL;
                    }
                  }
                }
              }
              break;
  
            case 'f':
            case 'F':
              if (!enable_fill[colorarray[player]])
                break;
              if (SQUARE(board,x,y)->color == colorarray[player])
              {
                if (SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] > fillcost &&
                          SQUARE(board,x,y)->move <= 1)
                {
                  if (SQUARE(board,x,y)->move == 1)
                  {
                    for (i=0; i<directions; i++)
                    {
                      if (SQUARE(board,x,y)->dir[i])
                        square2 = SQUARE(board,x,y)->connect[i];
                    }

                    if (square2->color == none && square2->value[none] <= 0)
                    {
                      square2->value[none] -= 1;

                      if (square2->value[none] <= (-fillnumber))
                        square2->value[none] = 1;

                      SQUARE(board,x,y)->move = 0;
                      SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] = 0;
                      for (k=0; k<directions; k++)
                        SQUARE(board,x,y)->dir[k] = 0;
                      SQUARE(board,x,y)->lowbound = 0;
                      outdatehorizon (board, square2->x, square2->y, ALL);
                      outdated[x][y] = ALL;
                      outdated[square2->y][square2->y] = ALL;
                    }
                  }
                  else if (enable_hills || enable_forest)
                  {
                    if (enable_hills)
                      limit = NHILLTONES-1;
                    else if (enable_forest)
                      limit = NHILLTONES-1;
                    else if (enable_sea)
                      break;

                    if (SQUARE(board,x,y)->value[none] < limit)
                    {
                      SQUARE(board,x,y)->value[none] += 1;

                      SQUARE(board,x,y)->move = 0;
                      SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] = 0;
                      for (k=0; k<8; k++)
                        SQUARE(board,x,y)->dir[k] = 0;
                      SQUARE(board,x,y)->lowbound = 0;
                      outdatehorizon (board, x, y, ALL);
                      outdated[x][y] = ALL;
                    }
                  }
                }
              }
              break;

            case 's':
            case 'S':
              if (!enable_scuttle[colorarray[player]])
                break;
              if (SQUARE(board,x,y)->color == colorarray[player])
              {
                if (SQUARE(board,x,y)->growth >= 60 && !enable_build[colorarray[player]])
                {
                  val = SQUARE(board,x,y)->value[SQUARE(board,x,y)->color];
                 
                  SQUARE(board,x,y)->growth -= val; 
                  SQUARE(board,x,y)->oldgrowth = SQUARE(board,x,y)->growth;

                  if (SQUARE(board,x,y)->growth < 60)
                  {
                    SQUARE(board,x,y)->growth = 0;
                    SQUARE(board,x,y)->oldgrowth = 0;
                    SQUARE(board,x,y)->angle = 0;
                  }

                  SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] -= val/2;

                  outdatehorizon (board, x, y, SQUARE(board,x,y)->color);
                  outdated[x][y] = ALL;
                }
                else if (enable_build[colorarray[player]] && SQUARE(board,x,y)-> angle > 0 &&
                  SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] >= scuttlerate[colorarray[player]])
                {
                  SQUARE(board,x,y)->angle -= buildrate[colorarray[player]];
                  SQUARE(board,x,y)->growth = 0;
                  if (SQUARE(board,x,y)->angle < 40)
                  {
                    SQUARE(board,x,y)->angle = 0; 
                    SQUARE(board,x,y)->growth = 0; 
                    SQUARE(board,x,y)->oldgrowth = 0; 
                  }
    
                  SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] -= scuttlerate[colorarray[player]]; 
                  if (SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] <= 0)
                  {
                      SQUARE(board,x,y)->move = 0;
                      SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] = 0; 
                      for (k=0; k<directions; k++)
                        SQUARE(board,x,y)->dir[k] = 0;
                      SQUARE(board,x,y)->lowbound = 0;
                  }

                  outdatehorizon (board, x, y, SQUARE(board,x,y)->color);
                  outdated[x][y] = ALL;
                }
              }
              break;
  
            case 'b':
            case 'B':
              if (!enable_build[colorarray[player]])
                break;
              if (SQUARE(board,x,y)->color == colorarray[player])
              {
                if (SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] > (MAXVAL-2) &&
                      SQUARE(board,x,y)->angle < 23040)
                {
                  if (SQUARE(board,x,y)->oldgrowth < 55)
                    SQUARE(board,x,y)->oldgrowth = 100;
                  SQUARE(board,x,y)->angle += buildrate[colorarray[player]];
                  if (SQUARE(board,x,y)->angle >= 23000)
                  {
                    SQUARE(board,x,y)->angle = 23040; 
                    SQUARE(board,x,y)->growth = SQUARE(board,x,y)->oldgrowth;
                  }
  
                  SQUARE(board,x,y)->value[SQUARE(board,x,y)->color] = 0; 
                  SQUARE(board,x,y)->lowbound = 0;
                  outdatehorizon (board, x, y, SQUARE(board,x,y)->color);
                  outdated[x][y] = ALL;
                }
              }
              break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
              if (!enable_reserve[colorarray[player]])
                break;
              if (SQUARE(board,x,y)->color == colorarray[player])
              {
                val = (double)(text[0] - '0')*MAXVAL/10.0;
                SQUARE(board,x,y)->lowbound = (int)(val);

                status = xwindow[player]->drawletter[player];
                strcpy (line, xwindow[player]->letter[player]);

                xwindow[player]->drawletter[player] = TRUE;

                text[1] = '\0';
                strcpy (xwindow[player]->letter[player], text);

                drawsquare(xwindow[player],SQUARE(board,x,y),colorarray[player],
                                                       squaresize[colorarray[player]]);

                strcpy (xwindow[player]->letter[player], line);
                xwindow[player]->drawletter[player] = status;
              }
              break;
  
            default:
              break;
          }
        }
        else
          print_message(text,textcount,colorarray[player],player,board);
      }
      break;
  }
}


/*************************************************************************/
/**	dumpsquare							**/
/** oft used routine for drawing updated square to all screens		**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/

dumpsquare (board, square)
  square_type *board,
              *square;
{
  int i;

  for (i=0; i<nplayers; i++)
  {
    if (!winopen[i])
      continue;
    /**** draw if visible to player i ****/
    if (visible(board,colorarray[i],square->x,square->y))
    {
      if (enable_hidden[colorarray[i]])
      {
        if (square->color == colorarray[i])
          drawsquare(xwindow[i],square,colorarray[i],squaresize[colorarray[i]]);
      }
      else
        drawsquare(xwindow[i],square,colorarray[i],squaresize[colorarray[i]]);
    }
  }
  /**** store if necessary ****/
  if (enable_storage)
    storedrawsquare(square,squaresize[0]);
}


/*************************************************************************/
/**	getsquare							**/
/** gets the board square corresponding to the window position		**/
/** and the direction from center within that square.  CSIZE is the 	**/
/** minimum distance from the center of the square for the move to be	**/
/** encoded.								**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/*************************************************************************/
getsquare(winx,winy,boardx,boardy,dir,squaresize, board, shift)
  int winx,winy,
      *boardx,*boardy,
      dir[DIRS],
      squaresize,
      shift;
  square_type *board;
{
  int i, j,
      done,
      dx, dy,
      row_even, col_even,
      modx, mody,
      xdiff, ydiff,
      xoff, yoff,
      xpos, ypos,
      col1, col2, row1, row2,
      min, minpos_i, minpos_j;

  double temp;

  if (enable_hex)
  {
    modx = (winx + hex_halfside)/(hex_side + hex_halfside);
    mody = (winy)/(hex_vert);

    if (mody%2 == 0)
      row_even = TRUE;
    else
      row_even = FALSE;

    if (modx%2 == 0)
      col_even = TRUE;
    else
      col_even = FALSE;

    col1 = modx - 1;
    col2 = modx;

    if (!row_even)
    {
      row1 = mody/2;
      row2 = row1;
    }
    else
    {
      if (col_even)
      {
        row1 = mody/2 - 1;
        row2 = mody/2;
      }
      else
      {
        row1 = mody/2;
        row2 = mody/2 - 1;
      }
    }

    xpos = modx*(hex_side + hex_halfside) + hex_quarterside;
    ypos = mody*hex_vert + hex_halfvert;

    xoff = winx - xpos;
    yoff = ypos - winy;

    temp = xoff * hex_slope;  

    if ((!col_even && !row_even) || (col_even && row_even))
    {
      if (yoff > temp)
      {
        *boardx = col1;
        *boardy = row1;
      }
      else
      {
        *boardx = col2;
        *boardy = row2;
      }
    }
    else
    {
      temp = -temp;

      if (yoff > temp)
      {
        *boardx = col2;
        *boardy = row2;
      }
      else
      {
        *boardx = col1;
        *boardy = row1;
      }
    }

    for (i=0; i<directions; i++)
      dir[i] = 0;

    done = FALSE;
    if (*boardx < 0)
    {
      *boardx = 0;
      done = TRUE;
    }
    else if (*boardx > boardsizex-1)
    {
      *boardx = boardsizex-1;
      done = TRUE;
    }

    if (*boardy < 0)
    {
      *boardy = 0;
      done = TRUE;
    }
    else if (*boardy > boardsizey-1)
    {
      *boardy = boardsizey-1;
      done = TRUE;
    }
  
    if (done)
      return;

    xoff = winx - SQUARE(board, *boardx, *boardy)->xpos + hex_side;
    yoff = SQUARE(board, *boardx, *boardy)->ypos - winy + hex_vert;

    if (hex_chart[xoff][yoff][0] >= 0)
    {
      dir[hex_chart[xoff][yoff][0]] = 1;
      if (shift && hex_chart[xoff][yoff][1] >= 0)
        dir[hex_chart[xoff][yoff][1]] = 1;
    }
  }
  else
  {
    /**** compute board position & relative position ****/
    *boardx = winx/squaresize;
    *boardy = (winy)/squaresize;
    dx = winx%squaresize - (squaresize/2);
    dy = (winy)%squaresize - (squaresize/2);
  
    /**** encode relative position as dir ****/
    if (dy<(-CSIZE)) dir[UP   ] = 1; else dir[UP   ] = 0;
    if (dy > CSIZE)  dir[DOWN ] = 1; else dir[DOWN ] = 0;
    if (dx > CSIZE)  dir[RIGHT] = 1; else dir[RIGHT] = 0;
    if (dx<(-CSIZE)) dir[LEFT ] = 1; else dir[LEFT ] = 0;

    if (abs(dx) < CSIZE && abs(dy) < CSIZE)
      for (i=0; i<directions; i++)
        dir[i] = 0;
  }
}


/*************************************************************************/
/**	removeplayer (player)						**/
/**  Remove player from the game.  If all players have been removed     **/
/**  then quit the game.						**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
removeplayer (player, board)
  int player;
  square_type *board;
{
  int i, j,
      done,
      limit;

  char line[512];

  winopen[player] = FALSE;
  done = TRUE;
  for (i=0; i<nplayers; i++)
    if (winopen[i] && !winwatch[i])
      done = FALSE;
  
  /**** free up all constructs ****/
  for (j=0; j<nsides; j++)
    XFreeGC(xwindow[player]->display, xwindow[player]->hue[j]);
  XFreeGC(xwindow[player]->display, xwindow[player]->hue[none]);
  XFreeGC(xwindow[player]->display, xwindow[player]->hue[dark]);
  XFreeGC(xwindow[player]->display, xwindow[player]->hue[light]);

  if (enable_terrain)
  {
    if (enable_hills)
      limit = NHILLTONES;
    else if (enable_forest)
     limit = NFORESTTONES;
    else if (enable_sea)
      limit = NSEATONES;

    for (j=0; j<limit; j++)
      XFreePixmap(xwindow[player]->display, xwindow[player]->terrain[j]);
  }

  /**** remove player from game ****/
  close_xwindow(xwindow[player]);

  if (done)
  {
    for (i=0; i<nplayers; i++)
    {
      if (winopen[i])
      {
        removeplayer (i, board);
      }
    }

    if (enable_storage || enable_replay)
    {
      if (enable_storage)
        fprintf (fpout, "Q00");
      fclose (fpout);
    }
    exit (0);
  }
  else
  {
    sprintf (line, "%s has quit the game", huename[sidemap[colorarray[player]]]);
/**
    sprintf (line, "%s (%s) has quit the game", huename[sidemap[colorarray[player]]],
   			grayname[color2bw[sidemap[colorarray[player]]]]); 
**/

    print_message(line,strlen(line),colorarray[player],player,board);

    for (i=0; i<nplayers; i++)
      if (winopen[i])
        XBell(xwindow[i]->display,100);
  }
}


gamestats (board)
  square_type *board;
{
  int i, j,
      color,
      force[MAXSIDES],
      area[MAXSIDES];

  for (i=0; i<nsides; i++)
  {
    force[i] = 0;
    area[i] = 0;
  }

  for (i=0; i<boardsizex; i++)
    for (j=0; j<boardsizey; j++)
    {
      color = SQUARE(board,i,j)->color;
      if (color != FIGHT && color != none)
      {
        force[color] += SQUARE(board,i,j)->value[color];
        area[color]++;
      }
    }

  for (i=0; i<nsides; i++)
    printf ("%s:  %3d squares   %5d force\n", huename[sidemap[i]], area[i], force[i]);
  printf ("\n");
}


/*************************************************************************/
/**	replaygame							**/
/** 	replay game from file (non-functionally)			**/	
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
replaygame(squaresize,board)
  int squaresize;
  square_type *board;
{
  int i, j, k, l,
      ipos, jpos,
      count, done,
      arg1, arg2, arg3, arg4,
      hafpsize, hafc,
      hafsquare,
      offset, terrain_type;

  static int xrand, yrand;

  char type;

  square_type *square;

  hafsquare = squaresize/2;
  done = FALSE;

  for (count=0; !done; count++)
  {
    /**** This is the hack to get REPLAY message to work.  Note that the ****/
    /**** number of loops to drawtime (500 below) appears to have an     ****/
    /**** allowable minimum of 2 or 3 hundred.  We're confounded.        ****/

    if (count == 500)
    {
      for (k=0; k<nplayers; k++)
      {
#if MULTITEXT
        for (l=0; l<nsides; l++)
          XDrawImageString(xwindow[k]->display,xwindow[k]->window,
       	               xwindow[k]->hue[light],
		       TEXTX,textyh[l],
		       messagestr,strlen(messagestr));
#else
        XDrawImageString(xwindow[k]->display,xwindow[k]->window,
       	               xwindow[k]->hue[light],
		       TEXTX,textyh[0],
		       messagestr,strlen(messagestr));
        XDrawImageString(xwindow[k]->display,xwindow[k]->window,
		       xwindow[k]->hue[light],
		       TEXTX,textyl[0],
		       messagestr,strlen(messagestr));
#endif
        XSync (xwindow[k]->display, 0);
        XFlush(xwindow[k]->display);
      }
    }

    /**** load command and arguments ****/
    type = fgetc (fpout); 
    i = fgetc (fpout);
    j = fgetc (fpout);
    square = SQUARE(board,i,j);
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

    if (type == 'A' || type == 'a' || type == 'D' || type == 'E' || type == 'F' || type == 'G' ||
        type == 'H' || type == 'I')
    {
      arg1 = fgetc (fpout);

      if (type == 'D' || type == 'E' || type == 'G' || type == 'H' || type == 'I' || type == 'F')
        arg2 = fgetc (fpout);

      if (type == 'E')
      {
        arg3 = fgetc (fpout);
        arg4 = fgetc (fpout);
      }
    }

    /**** for each window ****/
    for (k=0; k<nplayers; k++)
    {
      switch (type)
      {
        case 'A':
          terrain_type = arg1; 
          if (terrain_type > 128)
            terrain_type = terrain_type - 256;

          if (enable_hex)
          {
            offset = -2 * terrain_type;

            if (terrain_type < 0)
            {
              double_copy (xwindow[k], square, colorarray[k], squaresize, terrain_type, 0);
              terrain_type = 0;
            }
            else
              offset = 0;

            if (terrain_type == 1 && !enable_hills && !enable_forest)
              XFillArc (xwindow[k]->display,xwindow[k]->window, xwindow[k]->hue[none],
                     ipos-(hex_vert-1), jpos-(hex_vert-1), 2*(hex_vert-1), 2*(hex_vert-1), 0, 23040);
            else
              double_copy (xwindow[k], square, colorarray[k], squaresize, terrain_type, offset);
          }
          else
          {
            offset = ((double)(-terrain_type) * squaresize)/(2*fillnumber);

            if (terrain_type < 0)
              terrain_type = 0;
            else
              offset = 0;

            if (offset != 0)
              XCopyArea (xwindow[k]->display, xwindow[k]->terrain[1], xwindow[k]->window,
                  xwindow[k]->hue[dark], 0, 0, squaresize, squaresize, ipos, jpos);

            XCopyArea (xwindow[k]->display, xwindow[k]->terrain[terrain_type], xwindow[k]->window,
                          xwindow[k]->hue[dark], 0, 0, squaresize-2*offset, squaresize-2*offset,
                          ipos+offset, jpos+offset);
          }
          break;

        case 'B':
          if (enable_hex)
          {
            XFillArc (xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[none],
                 ipos-(hex_vert-1), jpos-(hex_vert-1), 2*(hex_vert-1), 2*(hex_vert-1), 0, 23040);
          }
          else
            XFillRectangle(xwindow[k]->display,xwindow[k]->window, xwindow[k]->hue[none],
                     ipos, jpos, squaresize, squaresize);
          break;

        case 'b':
          if (enable_hex)
            blanksquare(xwindow[k], square, colorarray[k], squaresize, TRUE);
          XSync (xwindow[k]->display, 0);
          break;
  
        case 'C':
          XDrawRectangle(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[dark],
                     ipos+hafsquare-CSIZE, jpos+hafsquare-CSIZE, 2*CSIZE, 2*CSIZE);
          break;
  
        case 'D':
          hafpsize = arg1/2;
          if (enable_hex)
          {
            XFillArc (xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg2],
                            ipos-hafpsize, jpos-hafpsize, arg1, arg1, 0, 23040);

          }
          else
          {
            XDrawRectangle(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[dark],
                       ipos+hafsquare-CSIZE, jpos+hafsquare-CSIZE, 2*CSIZE, 2*CSIZE);

            XFillRectangle(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg2],
                       ipos+hafsquare-hafpsize, jpos+hafsquare-hafpsize, arg1, arg1);
          }
          break;
  
        case 'E':

          if (enable_hex)
          {
            hafpsize = arg1/2;
            XFillArc (xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg2],
                      ipos-hafpsize, jpos-hafpsize, arg1, arg1, 0, 23040);
            hafpsize = arg3/2;
            XFillArc (xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg4],
                      ipos-hafpsize, jpos-hafpsize, arg3, arg3, 0, 23040);
          }
          else
          {
            hafpsize = arg1/2;
            XFillRectangle(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg2],
                       ipos+hafsquare-hafpsize,
                       jpos+hafsquare-hafpsize, arg1, arg1);
            hafpsize = arg3/2;
            XFillRectangle(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg4],
                       ipos+hafsquare-hafpsize,
                       jpos+hafsquare-hafpsize, arg3, arg3);
          }
          break;
  
        case 'F':
  
          if (enable_hex)
          {
            hafc = arg1/2;

            XDrawArc(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[dark],
                     ipos-hafc, jpos-hafc, arg1, arg1, 0, (arg2*23040)/255);

            XDrawArc(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[dark],
                     ipos-hafc + 1, jpos-hafc + 1, arg1 - 2, arg1 - 2, 0, (arg2*23040)/255);
          }
          else
          {
            hafc = (squaresize-arg1)/2;
            XDrawArc(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[dark],
                       ipos+hafc, jpos+hafc,
                       arg1, arg1, 0, (arg2*23040)/255);
  
            XDrawArc(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[dark],
                       ipos+hafc + 1, jpos+hafc + 1,
                       arg1 - 2, arg1 - 2, 0, (arg2*23040)/255);
          }
  
          break;
  
        case 'G':
          if (enable_hex)
          {
            if (arg2 == 'u')
            {
              XDrawLine(xwindow[k]->display,xwindow[k]->window, xwindow[k]->hue[arg1],
                                     ipos, jpos, ipos, jpos - hex_vert + 2);
              XDrawLine(xwindow[k]->display,xwindow[k]->window, xwindow[k]->hue[arg1],
                                     ipos+1, jpos, ipos+1, jpos - hex_vert + 2);
              XDrawLine(xwindow[k]->display,xwindow[k]->window, xwindow[k]->hue[arg1],
                                     ipos-1, jpos, ipos-1, jpos - hex_vert + 2);
            }
            if (arg2 == 'd')
            {
              XDrawLine(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg1],
                                     ipos, jpos, ipos, jpos + hex_vert - 2);
              XDrawLine(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg1],
                                     ipos+1, jpos, ipos+1, jpos + hex_vert - 2);
              XDrawLine(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg1],
                                     ipos-1, jpos, ipos-1, jpos + hex_vert - 2);

            }
            if (arg2 == 'R')
            {
              XDrawLine(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg1],
                           ipos, jpos, ipos + hex_3quarterside - 1, jpos - hex_halfvert + 1);
              XDrawLine(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg1],
                           ipos+1, jpos, ipos+1 + hex_3quarterside - 1, jpos - hex_halfvert + 1);
              XDrawLine(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg1],
                           ipos, jpos-1, ipos+1 + hex_3quarterside - 1, jpos-1 - hex_halfvert + 1);
            }
            if (arg2 == 'r')
            {
              XDrawLine(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg1],
                           ipos, jpos, ipos + hex_3quarterside - 1, jpos + hex_halfvert - 1);
              XDrawLine(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg1],
                           ipos+1, jpos, ipos+1 + hex_3quarterside - 1, jpos + hex_halfvert - 1);
              XDrawLine(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg1],
                           ipos, jpos+1, ipos + hex_3quarterside - 1, jpos+1 + hex_halfvert - 1);
            }
            if (arg2 == 'L')
            {
              XDrawLine(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg1],
                           ipos, jpos, ipos - hex_3quarterside + 1, jpos - hex_halfvert + 1);
              XDrawLine(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg1],
                           ipos-1, jpos, ipos-1 - hex_3quarterside + 1, jpos - hex_halfvert + 1);
              XDrawLine(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg1],
                           ipos, jpos-1, ipos - hex_3quarterside + 1, jpos-1 - hex_halfvert + 1);
            }
            if (arg2 == 'l')
            {
              XDrawLine(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg1],
                           ipos, jpos, ipos - hex_3quarterside + 1, jpos + hex_halfvert - 1);
              XDrawLine(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg1],
                           ipos-1, jpos, ipos-1 - hex_3quarterside + 1, jpos + hex_halfvert - 1);
              XDrawLine(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg1],
                           ipos, jpos+1, ipos - hex_3quarterside + 1, jpos+1 + hex_halfvert - 1);

            }
          }
          else
          {
            ipos = ipos+hafsquare;
            jpos = jpos+hafsquare;
            if (arg2 == 'u')
            {
              XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg1],
                       ipos-1, jpos, ipos-1, jpos-hafsquare);
              XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg1],
                       ipos, jpos, ipos, jpos-hafsquare);
              XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg1],
                       ipos+1, jpos, ipos+1, jpos-hafsquare);
            }
            else if (arg2 == 'd')
            {
              XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg1],
                       ipos-1, jpos, ipos-1, jpos+hafsquare-1);
              XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg1],
                       ipos, jpos, ipos, jpos+hafsquare-1);
              XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg1],
                       ipos+1, jpos, ipos+1, jpos+hafsquare-1);
  
            }
            else if (arg2 == 'r')
            {
              XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg1],
                       ipos, jpos-1, ipos+hafsquare-1, jpos-1);
              XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg1],
                       ipos, jpos, ipos+hafsquare-1, jpos);
              XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg1],
                       ipos, jpos+1, ipos+hafsquare-1, jpos+1);
            }
            else if (arg2 == 'l')
            {
              XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg1],
                       ipos, jpos-1, ipos-hafsquare, jpos-1);
              XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg1],
                       ipos, jpos, ipos-hafsquare, jpos);
              XDrawLine(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg1],
                       ipos, jpos+1, ipos-hafsquare, jpos+1);
            }
          }
          break;
  
        case 'H':
          if (enable_hex)
          {
            xrand = (xrand+2938345)%hex_side;
            yrand = (yrand+2398321)%hex_side;

            hafc = arg1/2;

            XFillArc(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg2],
                     ipos - hafc + xrand - hex_halfside,
                     jpos - hafc + yrand - hex_halfside,
                     arg1, arg1, 0, 23040);
          }
          else
          {
            xrand = (xrand+2938345)%(squaresize/2);
            yrand = (yrand+2398321)%(squaresize/2);
  
            hafc = (squaresize-arg1)/2; 
  
            XFillArc(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg2],
                     ipos+hafc+xrand-squaresize/4, jpos+hafc+yrand-squaresize/4,
                     arg1, arg1, 0, 23040);
          }
          break;
  
        case 'I':
          if (enable_hex)
          {
            xrand = (xrand+2938345)%hex_side;
            yrand = (yrand+2398321)%hex_side;

            hafc = arg1/2;

            XFillArc(xwindow[k]->display, xwindow[k]->window, xwindow[k]->hue[arg2],
                     ipos - hafc + xrand - hex_halfside, jpos - hafc + yrand - hex_halfside,
                     arg1, arg1, 0, 11520);
          }
          else
          {
            xrand = (xrand+2938345)%(squaresize/2);
            yrand = (yrand+2398321)%(squaresize/2);
  
            hafc = (squaresize-arg1)/2; 
  
            XFillArc(xwindow[k]->display,xwindow[k]->window,xwindow[k]->hue[arg2],
                     ipos+hafc+xrand-squaresize/4,
                     jpos+hafc+yrand-squaresize/4,
                     arg1, arg1, 0, 11520);
          }
          break;

        case 'J':
          XSync (xwindow[k]->display, 0);
          break;
  
        case 'Q':
          XSync (xwindow[k]->display, 0);
          done = TRUE;
          break;
  
        default:
          /**** try to recover from bad input ****/
          printf ("%c %d %d %d %d\n", type, i, j, arg1, arg2);
          while (type < 'A' || type > 'J' && type != 'Q')
            type = fgetc(fpout);
          ungetc (type, fpout);
          printf ("TROUBLE\n");
          exit (0);
          break;
      }
      /**** make sure window is flushed regularly ****/
      if ((count % 15) == 0)
        XSync (xwindow[k]->display, 0);
    }
  }

  while (TRUE);
}

