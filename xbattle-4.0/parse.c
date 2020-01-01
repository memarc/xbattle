#include <stdio.h>
  
/**** x include files ****/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include "extern.h"

/*************************************************************************/
/**	get_displaynames						**/
/** extract the display names from the command line arguments		**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
get_displaynames(displayname,colorarray,rcolorarray,argc,argv)
  char displayname[MAXPLAYERS][80],
       *argv[];
  int argc,
      colorarray[MAXPLAYERS],
      rcolorarray[MAXHUES];
{
  int i, j, k,
      extra,
      index=0, count=0,
      primary, secondary,
      colon,
      sidetaken[MAXSIDES+1];

  char *ptr;

  char line[100];

  nplayers = 0;
  for (j=1;j<=MAXSIDES; j++)
    sidetaken[j] = FALSE;

  for (j=0;j<MAXHUES; j++)
    rcolorarray[j] = NOT_TAKEN;

  /**** initialize message strings ****/
  sprintf (messagestr,"");

  for (i=0; i<MAXPLAYERS; i++)
  {
    winopen[i] = TRUE;
    winwatch[i] = FALSE;
  }

  /**** set other displays ****/
  for (i=1; i<argc; )
  {
    if (goodoption(argv[i]))
    {
      extra = installoption (argc, argv, i, GLOBAL);
      i += extra;
    }
    else
    {
      primary = -1;
      for (j=1;j<=MAXHUES && primary<0; j++)
      {
        /**** find primary (c1) color ****/
        if (matchstr(&(argv[i][1]), huename[j]))
        {
          primary = j;
          secondary = -1;

          /**** find secondary (c2) color ****/
          if (strlen(argv[i]) <= strlen(huename[j])+1)
          {
            /**** no secondary color ****/
          }
          else
          {
            ptr = &(argv[i][1+strlen(huename[primary])]);
            for (k=1;k<=MAXHUES; k++)
            {
              if (matchstr(ptr,grayname[k]))
              {
                secondary = k;
              }
            }
          }

          if (argv[i+1][0] == '{')
          {
            i+=2;
            while (argv[i][0] != '}')
            {
              if (goodoption(argv[i]))
              {
                if (rcolorarray[primary] != NOT_TAKEN)
                  extra = installoption (argc, argv, i, rcolorarray[primary]);
                else
                  extra = installoption (argc, argv, i, count);
                i += extra;
              }
              else
              {
                printf ("BAD OPTION IN BRACKETS\n");
                exit (0);
              }
            }
          }

          if(strcmp(argv[i+1],"me") == 0)
            strcpy(displayname[index],"");
          else
          {
            /**** if no :x.y given in display name add :0.0 ****/
            colon = FALSE;
            for (k=0; k<strlen(argv[i+1]); k++)
              if (argv[i+1][k] == ':')
                colon = TRUE;
            strcpy (line, argv[i+1]);
            if (!colon)
              strcat (line, ":0.0");
  
            strcpy(displayname[index],line);
          }
  
          if (strcmp(argv[i+1],"you") == 0)
          {
            /**** dummy player ****/
          }
          else
          {
            /**** set up player to color mapping ****/
            if (!sidetaken[primary])
            {
              colorarray[index]=count;
              rcolorarray[primary]=count;
            }
            else
            {
              for (k=0; k<count; k++)
                if (sidemap[k] == primary)
                  colorarray[index]=k;
            }
            index++;
          }
  
          /**** set up side to color mapping ****/
          if (!sidetaken[primary])
          {
            sidemap[count] = primary;
            if (secondary >= 0)
              color2bw[primary] = secondary;
  
            if (secondary < 0 && primary > WHITE)
            {
              lettermap[count][0] = huename[primary][0] - 'a' + 'A';
              lettermap[count][1] = '\0';
            }
            else
              lettermap[count][0] = FALSE;
  
            count++;
          }
          sidetaken[primary] = TRUE;
        }
      }
      if (primary == -1)
      {
        printf ("Invalid command    : %s\n", argv[i]);
        i++;
      }
      else
        i+=2;
    }
  }

  nplayers = index;
  nsides = count;
  none = nsides;
  dark = nsides+1;
  light = nsides+2;

  if (nsides < 1 && !is_arg("-replay",argc,argv))
  {
    printf (usage);
    exit (0);
  }
}


/*************************************************************************/
/**	init_defaults							**/
/** initialize default option values					**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
init_defaults()
{
  int i, j;

  char str[100];

  for (i=0; i<MAXSIDES; i++)
  {
    slowfactor[i] = SLOWFACTOR;
    artillery[i] = 0;
    paratroops[i] = 0;
    bases[i] = 0;
    rbases[i] = 0;
    militia[i] = 0;
    armies[i] = 0;
    firepower[i] = FIREPOWER / (1.0 + gamespeed/5.0);
    forest[i] = 0;
    hillfactor[i] = 0;
    decayrate[i] = 0;
    eroderate[i] = 0;
    viewrange[i] = VIEWRANGE;
    squaresize[i] = SQUARESIZE;
    enable_fill[i] = FALSE;
    enable_dig[i] = FALSE;
    enable_scuttle[i] = FALSE;
    enable_build[i] = FALSE;
    enable_personalmarch[i] = FALSE;
    marchrate[i] = MARCH;
    enable_artillery[i] = FALSE;
    enable_paratroops[i] = FALSE;
    enable_grid[i] = TRUE;
    enable_manpos[i] = FALSE;
    enable_hidden[i] = FALSE;
    enable_disrupt[i] = FALSE;
    enable_map[i] = FALSE;
    enable_basemap[i] = FALSE;
    enable_localmap[i] = FALSE;
    enable_personalhorizon[i] = FALSE;
    enable_personalbound[i] = FALSE;
    enable_erode[i] = FALSE;
    enable_digin[i] = FALSE;
    enable_repeat[i] = FALSE;
    enable_reserve[i] = FALSE;
    enable_attack[i] = FALSE;
    enable_nospigot[i] = FALSE;

    for (j=0; j<4; j++)
      dir[i][j] = 0;
    dirtype[i] = FORCE;
  }
  delay = 5000;
  seed = getpid() + time(NULL);
  maxviewrange = 0;
  enable_anylocalmap = FALSE;
  enable_bound = FALSE;
  enable_march = FALSE;
  enable_edit = FALSE;
  enable_area = FALSE;
  fillnumber = 1;
  directions = 4;

  sprintf(str,"seed=%d ",seed);
  strcat(messagestr,str);
}


/*************************************************************************/
/**	installoption							**/
/** install current option						**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
installoption (argc, argv, pos, side)
  int argc;
  char *argv[];
  int pos,
      side;
{
  double djunk,
         atof();
  int i, j,
      extra,
      ijunk;

  char str[80],
       filename[80],
       line[80];

  strcpy (line, argv[pos]);

  if (strcmp(line,"-seed") == 0)
    return (2);

  if (strcmp(line,"-hills") == 0)
  {
    djunk = atof (argv[pos+1]);
    enable_hills = TRUE;
    enable_terrain = TRUE;
    sprintf(str,"hills=%d ", (int)(djunk));
    strcat(messagestr,str);

    if (side == GLOBAL)
    {
      for (i=0; i<MAXSIDES; i++)
        hillfactor[i] = djunk*0.2;
    }
    else
      hillfactor[side] = djunk*0.2;

    return (2);
  }

  if (strcmp(line,"-forest") == 0)
  {
    djunk = atof (argv[pos+1]);
    enable_forest = TRUE;
    enable_terrain = TRUE;
    sprintf(str,"forest=%d ", (int)(djunk));
    strcat(messagestr,str);

    if (side == GLOBAL)
    {
      for (i=0; i<MAXSIDES; i++)
        forest[i] = djunk*0.2;
    }
    else
      forest[side] = djunk*0.2;

    return (2);
  }

  if (strcmp(line,"-sea") == 0)
    return (2);

  if (strcmp(line,"-move") == 0)
  {
    djunk = atof (argv[pos+1]);
    sprintf(str,"move=%d ", (int)(djunk));
    strcat(messagestr,str);
    if (side == GLOBAL)
    {
      for (i=0; i<MAXSIDES; i++)
        slowfactor[i] = (10.0-djunk)*0.6;
    }
    else
      slowfactor[side] = (10.0-djunk)*0.6;
    return (2);
  }

  if (strcmp(line,"-digin") == 0)
  {
    djunk = atof (argv[pos+1]);
    sprintf(str,"digin=%d ", (int)(djunk));
    strcat(messagestr,str);
    if (side == GLOBAL)
    {
      for (i=0; i<MAXSIDES; i++)
      {
        enable_digin[i] = TRUE;
        shuntval[i] = djunk;
      }
    }
    else
    {
      enable_digin[side] = TRUE;
      shuntval[side] = djunk;
    }
    return (2);
  }

  if (strcmp(line,"-guns") == 0)
  {
    djunk = atof (argv[pos+1]);
    sprintf(str,"guns=%d ", (int)(djunk));
    strcat(messagestr,str);

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
      {
        enable_artillery[i] = TRUE;
        artillery[i] = djunk*0.05;
      }
    else
    {
      artillery[side] = djunk*0.05;
      enable_artillery[side] = TRUE;
    }
    return (2);
  }

  if (strcmp(line,"-para") == 0)
  {
    djunk = atof (argv[pos+1]);
    sprintf(str,"para=%d ", (int)(djunk));
    strcat(messagestr,str);

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
      {
        paratroops[i] = djunk*0.05;
        enable_paratroops[i] = TRUE;
      }
    else
    {
      paratroops[side] = djunk*0.05;
      enable_paratroops[side] = TRUE;
    }
    return (2);
  }

  if (strcmp(line,"-farms") == 0)
    return (2); 

  if (strcmp(line,"-hex") == 0)
    return (1); 

  if (strcmp(line,"-border") == 0)
    return (1); 

  if (strcmp(line,"-towns") == 0)
    return (2);

  if (strcmp(line,"-bases") == 0)
  {
    enable_bases = TRUE;
    ijunk = atoi (argv[pos+1]);
    sprintf(str,"bases=%d ", ijunk);
    strcat(messagestr,str);

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        bases[i] = ijunk;
    else
      bases[side] = ijunk;
    return (2);
  }

  if (strcmp(line,"-rbases") == 0)
  {
    enable_rbases = TRUE;
    ijunk = atoi (argv[pos+1]);
    sprintf(str,"rbases=%d ", ijunk);
    strcat(messagestr,str);

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        rbases[i] = ijunk;
    else
      rbases[side] = ijunk;
    return (2);
  }

  if (strcmp(line,"-armies") == 0)
  {
    enable_armies = TRUE;
    ijunk = atoi (argv[pos+1]);
    sprintf(str,"armies=%d ", ijunk);
    strcat(messagestr,str);

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        armies[i] = ijunk;
    else
      armies[side] = ijunk;
    return (2);
  }

  if (strcmp(line,"-militia") == 0)
  {
    enable_militia = TRUE;
    djunk = atof (argv[pos+1]);
    sprintf(str,"militia=%d ", (int)(djunk));
    strcat(messagestr,str);

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        militia[i] = djunk;
    else
      militia[side] = djunk;
    return (2);
  }

  if (strcmp(line,"-speed") == 0)
    return (2);

  if (strcmp(line,"-fight") == 0)
  {
    djunk = atof (argv[pos+1]);
    sprintf(str,"fight=%d ", (int)(djunk));
    strcat(messagestr,str);

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        firepower[i] = (djunk)/(1.0+(gamespeed/5.0));
    else
      firepower[side] = (djunk)/(1.0+(gamespeed/5.0));
    return (2);
  }

  if (strcmp(line,"-decay") == 0)
  {
    enable_decay = TRUE;
    djunk = atof (argv[pos+1]);
    sprintf(str,"decay=%d ", (int)(djunk));
    strcat(messagestr,str);

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        decayrate[i] = djunk/(10.0*(double)(MAXVAL));
    else
      decayrate[side] = djunk/(10.0*(double)(MAXVAL));

    return (2);
  }


  if (strcmp(line,"-erode") == 0)
  {
    djunk = atof (argv[pos+1]);
    sprintf(str,"erode=%d ", (int)(djunk));
    strcat(messagestr,str);

    if (side == GLOBAL)
    {
      for (i=0; i<MAXSIDES; i++)
      {
        enable_erode[i] = TRUE;
        eroderate[i] = djunk*10.0;
      }
    }
    else
    {
      enable_erode[side] = TRUE;
      eroderate[side] = djunk*10.0;
    }

    return (2);
  }

  if (strcmp(line,"-fill") == 0)
  {
    djunk = atof (argv[pos+1]);
    sprintf(str,"fill=%d ", (int)(djunk));
    strcat(messagestr,str);

    if (side == GLOBAL)
    {
      for (i=0; i<MAXSIDES; i++)
      {
        enable_fill[i] = TRUE;
        fillnumber = (int)(djunk);
      }
    }
    else
    {
      enable_fill[side] = TRUE;
      fillnumber = (int)(djunk);
    }

    return (2);
  }

  if (strcmp(line,"-dig") == 0)
  {
    djunk = atof (argv[pos+1]);
    sprintf(str,"dig=%d ", (int)(djunk));
    strcat(messagestr,str);

    if (side == GLOBAL)
    {
      for (i=0; i<MAXSIDES; i++)
      {
        enable_dig[i] = TRUE;
        fillnumber = (int)(djunk);
      }
    }
    else
    {
      enable_dig[side] = TRUE;
      fillnumber = (int)(djunk);
    }
    return (2);
  }

  if (strcmp(line,"-scuttle") == 0)
  {
    strcat(messagestr,"scuttle ");
    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        enable_scuttle[i] = TRUE;
    else
      enable_scuttle[side] = TRUE;
    return (1);
  }

  if (strcmp(line,"-build") == 0)
  {
    ijunk = atoi (argv[pos+1]);
    sprintf(str,"build=%d ", ijunk);
    strcat(messagestr,str);


    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
      {
        enable_build[i] = TRUE;
        buildrate[i] = 23040/ijunk;
        scuttlerate[i] = MAXVAL/ijunk;
      }
    else
    {
      enable_build[side] = TRUE;
      buildrate[side] = 23040/ijunk;
      scuttlerate[side] = MAXVAL/ijunk;
    }
    return (2);
  }

  if (strcmp(line,"-march") == 0)
  {
    ijunk = atoi (argv[pos+1]);
    enable_march = TRUE;
    sprintf(str,"march=%d ", ijunk);
    strcat(messagestr,str);

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
      {
        enable_personalmarch[i] = TRUE;
        marchrate[i] = ijunk;
      }
    else
    {
      enable_personalmarch[side] = TRUE;
      marchrate[side] = ijunk;
    }
    return (2);
  }

  if (strcmp(line,"-reserve") == 0)
  {
    strcat(messagestr,"reserve ");
    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        enable_reserve[i] = TRUE;
    else
      enable_reserve[side] = TRUE;
    return (1);
  }

  if (strcmp(line,"-bound") == 0)
  {
    strcat(messagestr,"bound ");
    enable_bound = TRUE;
    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        enable_personalbound[i] = TRUE;
    else
      enable_personalbound[side] = TRUE;
    return (1);
  }

  if (strcmp(line,"-area") == 0)
  {
    strcat(messagestr,"area ");
    enable_area = TRUE;
    return (1);
  }

  if (strcmp(line,"-repeat") == 0)
  {
    strcat(messagestr,"repeat ");
    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        enable_repeat[i] = TRUE;
    else
      enable_repeat[side] = TRUE;
    return (1);
  }

  if (strcmp(line,"-attack") == 0)
  {
    strcat(messagestr,"attack ");
    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        enable_attack[i] = TRUE;
    else
      enable_attack[side] = TRUE;
    return (1);
  }

  if (strcmp(line,"-nospigot") == 0)
  {
    strcat(messagestr,"nospigot ");
    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        enable_nospigot[i] = TRUE;
    else
      enable_nospigot[side] = TRUE;
    return (1);
  }

  if (strcmp(line,"-wrap") == 0)
  {
    enable_anywrap = TRUE;
    strcat(messagestr,"wrap ");

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
      {
        enable_wrap[i] = TRUE;
      }
    else
      enable_wrap[side] = TRUE;

    return (1);
  }

  if (strcmp(line,"-nogrid") == 0)
  {
    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
      {
        enable_grid[i] = FALSE;
      }
    else
      enable_grid[side] = FALSE;
    return (1);
  }

  if (strcmp(line,"-horizon") == 0)
  {
    enable_horizon = TRUE;
    if (pos < argc-1)
    {
      if (argv[pos+1][0] >= '0' && argv[pos+1][0] <= '9')
      {
        ijunk = atoi (argv[pos+1]);
        extra = 2;
      }
      else
      {
        ijunk = VIEWRANGE;
        extra = 1;
      }
    }
    else
    {
      ijunk = VIEWRANGE;
      extra = 1;
    }
    sprintf(str,"horizon=%d ", ijunk);
    strcat(messagestr,str);

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
      {
        viewrange[i] = ijunk;
        enable_personalhorizon[i] = TRUE;
      }
    else
    {
      viewrange[side] = ijunk;
      enable_personalhorizon[side] = TRUE;
    }

    return (extra);
  }

  if (strcmp(line,"-hidden") == 0)
  {
    strcat(messagestr,"hidden ");

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        enable_hidden[i] = TRUE;
    else
      enable_hidden[side] = TRUE;
    return (1);
  }

  if (strcmp(line,"-map") == 0)
  {
    strcat(messagestr,"map ");

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        enable_map[i] = TRUE;
    else
      enable_map[side] = TRUE;
    return (1);
  }

  if (strcmp(line,"-basemap") == 0)
  {
    strcat(messagestr,"basemap ");

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        enable_basemap[i] = TRUE;
    else
      enable_basemap[side] = TRUE;
    return (1);
  }

  if (strcmp(line,"-localmap") == 0)
  {
    strcat(messagestr,"localmap ");
    enable_anylocalmap = TRUE;

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        enable_localmap[i] = TRUE;
    else
      enable_localmap[side] = TRUE;
    return (1);
  }

  if (strcmp(line,"-disrupt") == 0)
  {
    strcat(messagestr,"disrupt ");

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        enable_disrupt[i] = TRUE;
    else
      enable_disrupt[side] = TRUE;
    return (1);
  }

  if (strcmp(line,"-board") == 0)
    return (2);

  if (strcmp(line,"-boardx") == 0)
    return (2);

  if (strcmp(line,"-boardy") == 0)
    return (2);

  if (strcmp(line,"-square") == 0)
  {
    ijunk = atoi (argv[pos+1]);

    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
        squaresize[i] = ijunk;
    else
      squaresize[side] = ijunk;
    return (2);
  }

  if (strcmp(line,"-manpos") == 0)
  {
    if (side == GLOBAL)
      for (i=0; i<MAXSIDES; i++)
      {
        enable_manpos[i] = TRUE;
      }
    else
        enable_manpos[side] = TRUE;
    return (1);
  }

  if (strcmp(line,"-store") == 0)
  {
    if (!stringval ("-store",argc,argv,filename))
      return (1);
    else
      return (2);
  }

  if (strcmp(line,"-replay") == 0)
  {
    if (!stringval ("-replay",argc,argv,filename))
      return (1);
    else
      return (2);
  }

  if (strcmp(line,"-load") == 0)
  {
    if (!stringval ("-load",argc,argv,filename))
      return (1);
    else
      return (2);
  }

  if (strcmp(line,"-edit") == 0)
  {
    if (!stringval ("-edit",argc,argv,filename))
      return (1);
    else
      return (2);
  }

  if (strcmp(line,"-options") == 0)
  {
    if (pos == argc-1)
    {
      strcpy (filename, "default.xbo");
      extra = 1;
    }
    else if (argv[pos+1][0] == '-' || argv[pos+1][0] == '}')
    {
      strcpy (filename, "default.xbo");
      extra = 1;
    }
    else
    {
      strcpy (filename, argv[pos+1]);
      extra = 2;
    }

    loadoptions (filename, side);

    return (extra);
  }
}


/*************************************************************************/
/**	loadoptions							**/
/** load options from file						**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
loadoptions(filename, side)
  char filename[];
  int side;
{
  int ptr1,
      ptr2;
      
  char line[100],
       *option[3],
       junkdisplayname[MAXPLAYERS][80];

  FILE *fpopts,
       *fopen();

  option[0] = (char *)(malloc(50*sizeof(char)));
  option[1] = (char *)(malloc(50*sizeof(char)));
  option[2] = (char *)(malloc(50*sizeof(char)));

  if ((fpopts = fopen (filename, "r")) == NULL)
  {
    printf ("Unable to open option file %s\n", filename);
    exit (0);
  }

  while (fgets (line, 100, fpopts) != NULL)
  {
    option[0][0] = '\0';

    for (ptr1=0; line[ptr1] != ' ' && line[ptr1] != '\0' && line[ptr1] != '\n'; ptr1++)
      option[1][ptr1] = line[ptr1];
    option[1][ptr1] = '\0';  

    for (; line[ptr1] == ' ' && line[ptr1] != '\0' && line[ptr1] != '\n'; ptr1++);

    for (ptr2=0; line[ptr1] != '\0' && line[ptr1] != '\n'; ptr1++, ptr2++)
      option[2][ptr2] = line[ptr1];
    option[2][ptr2] = '\0';  

    if (option[2][0] == '\0')
    {
      strcpy (option[2], "-blah");
    }

    if (goodoption(option[1]))
    {
      installoption (3, option, 1, side);
      init_options (junkdisplayname, 3, option);
    }
    else
    {
      printf ("illegal argument in option file:   %s\n", line);
    }
  }

  fclose (fpopts);
}


/*************************************************************************/
/**	goodoption							**/
/** Return TRUE if string is valid option				**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
goodoption(line)
  char line[];
{
  int i, j,
      valid,
      allvalid,
      ptr;

  valid = FALSE;
  for (i=0; i<NOPTS && !valid; i++)
  {
    if (strcmp(line,options[i]) == 0)
    {
      valid = TRUE;
    }
  }
  return (valid);
}


/*************************************************************************/
/**	checkoptions							**/
/** Make sure all command line arguments are valid			**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
checkoptions(argc,argv)
  int argc;
  char *argv[];
{
  int i, j,
      valid,
      allvalid,
      inbracket,
      ptr;

  char lastside[100];

  ptr = 1;
  allvalid = TRUE;
  inbracket = FALSE;

  while (ptr < argc)
  {
    valid = FALSE;
    for (i=0; i<NOPTS && !valid; i++)
    {
      if (strcmp(argv[ptr],options[i]) == 0)
      {
        valid = TRUE;
        if (i < POPTS)
        {
          if (ptr+1 == argc)
          {
            allvalid = FALSE; 
            printf ("missing argument:     %s\n", argv[ptr]); 
            ptr++;
          }
          else if (argv[ptr+1][0] == '-' || (inbracket && argv[ptr+1][0] == '}'))
          {
            allvalid = FALSE; 
            printf ("missing argument:     %s\n", argv[ptr]); 
            ptr++;
          }
          else
            ptr += 2;
        }
        else if (i < POPTS + PVOPTS)
        {
          if (ptr+1 == argc)
            ptr++;
          else if (argv[ptr+1][0] == '-' || (inbracket && argv[ptr+1][0] == '}'))
            ptr++;
          else
            ptr += 2;
        }
        else
        {
          if (ptr+1 != argc)
          {
            if (argv[ptr+1][0] != '-' && !(inbracket && argv[ptr+1][0] == '}'))
            {
              allvalid = FALSE; 
              printf ("extra argument:       %s %s\n", argv[ptr], argv[ptr+1]); 
              ptr += 2;
            }
            else
              ptr++;
          }
          else
            ptr++;
        }
      }
    }

    if (!valid && inbracket)
    {
      if (argv[ptr][0] == '}')
      {
        valid = TRUE;
        if (ptr+1 == argc)
        {
          allvalid = FALSE;
          printf ("missing screen name:  %s\n", lastside);
          ptr++;
        }      
        else if (argv[ptr+1][0] == '-')
        {
          allvalid = FALSE;
          printf ("missing screen name:  %s\n", lastside);
          ptr++;
        }      
        else
        {
          ptr+=2;
        }

        inbracket = FALSE;
      }
    }
    else if (!valid)
    {
      for (j=1; j<=MAXHUES && !valid; j++)
      {
        if (matchstr(&(argv[ptr][1]), huename[j]))
        {
          valid = TRUE;
          if (ptr+1 == argc)
          {
            allvalid = FALSE; 
            printf ("missing argument:     %s\n", argv[ptr]); 
            ptr++;
          }
          else if (argv[ptr+1][0] == '-')
          {
            allvalid = FALSE; 
            printf ("missing argument:     %s\n", argv[ptr]); 
            ptr++;
          }
          else
          {
            if (argv[ptr+1][0] == '{')
            {
              inbracket = TRUE;
              strcpy (lastside, argv[ptr]);
            }
            ptr += 2;
          }
        }
      }
    }

    if (!valid)
    {
      allvalid = FALSE; 
      if (ptr+1 == argc)
      {
        printf ("illegal argument:     %s\n", argv[ptr]); 
        ptr++;
      }
      else if (argv[ptr+1][0] == '-')
      {
        printf ("illegal argument:     %s\n", argv[ptr]); 
        ptr++;
      }
      else
      {
        printf ("illegal argument:     %s %s\n", argv[ptr], argv[ptr+1]); 
        ptr += 2;
      }
    }
  }

  if (inbracket)
  {
    allvalid = FALSE;
    printf ("incomplete side:      %s\n", lastside);
  }

  if (!allvalid)
    exit (0);
}
      

/*************************************************************************/
/**     storeparameters                                                 **/
/**  store relevant game parameters at head of storage file		**/
/**     Greg Lesher (lesher@park.bu.edu)                                **/
/*************************************************************************/
storeparameters ()
{
  int i;

  fprintf (fpout, "%c", squaresize[0]);
  fprintf (fpout, "%c", boardsizex);
  fprintf (fpout, "%c", boardsizey);
  fprintf (fpout, "%c", enable_hex);
  fprintf (fpout, "%c", enable_hills);
  fprintf (fpout, "%c", enable_forest);
  fprintf (fpout, "%c", enable_sea);
  fprintf (fpout, "%c", enable_terrain);
  fprintf (fpout, "%c", nsides);
  fprintf (fpout, "%c", fillnumber);
  for (i=0; i<nsides; i++)
    fprintf (fpout, "%c", sidemap[i]);
  for (i=0; i<nsides; i++)
    fprintf (fpout, "%c", color2bw[sidemap[i]]);
}


/*************************************************************************/
/**     loadparameters                                                  **/
/**  load relevant game parameters from replay file			**/
/**     Greg Lesher (lesher@park.bu.edu)                                **/
/*************************************************************************/
loadparameters (displayname)
  char displayname[MAXPLAYERS][80];
{
  int i;

  squaresize[0] = getc (fpout);
  boardsizex = getc (fpout);
  boardsizey = getc (fpout);
  enable_hex = getc (fpout);
  enable_hills = getc (fpout);
  enable_forest = getc (fpout);
  enable_sea = getc (fpout);
  enable_terrain = getc (fpout);
  nsides = getc (fpout);
  fillnumber = getc (fpout);
  for (i=0; i<nsides; i++)
    sidemap[i] = getc(fpout);
  for (i=0; i<nsides; i++)
    color2bw[sidemap[i]] = getc (fpout);

  for (i=0; i<nsides; i++)
    lettermap[i][0] = FALSE;

  colorarray[0] = 0;

  /**** if no command line display names ****/
  if (nplayers == 0)
  {
    nplayers = 1;
    strcpy (displayname[0], "");
  }

  none = nsides;
  dark = nsides+1;
  light = nsides+2;
}


/*************************************************************************/
/**	init_options							**/
/** initialize the game board options using global variables		**/
/**	Steve Lehar (slehar@park.bu.edu)				**/
/*************************************************************************/
init_options(displayname,argc,argv)
  char displayname[MAXPLAYERS][80];
  int argc;
  char *argv[];
{
  int i,
      pos;
  char str[80];
  extern double doubleval();

  /**** set random seed ****/
  if (is_arg("-seed",argc,argv))
  {
    seed = intval("-seed",argc,argv);
    strcat(messagestr,str);
  } 

  if (is_arg("-sea",argc,argv))
  {
    enable_sea = TRUE;
    sprintf(str,"sea=%2.0f ",doubleval("-sea",argc,argv));
    strcat(messagestr,str);
    enable_terrain = TRUE;
    sea = 20.0 - 2.0*doubleval("-sea",argc,argv);
    if (sea <= 0.0)
      sea = 2.0;
  }

  /**** farms ****/
  if (is_arg("-farms",argc,argv))
  {
    enable_farms = TRUE;
    farms = doubleval("-farms",argc,argv);

    sprintf(str,"farms=%2.0f ",doubleval("-farms",argc,argv));
    strcat(messagestr,str);
  }

  /**** border ****/
  if (is_arg("-border",argc,argv))
  {
    enable_border = TRUE;
  }

  /**** hex ****/
  if (is_arg("-hex",argc,argv))
  {
    enable_hex = TRUE;
    directions = 6;
  }

  /**** town ****/
  if (is_arg("-towns",argc,argv))
  {
    enable_towns = TRUE;
    towns = doubleval("-towns",argc,argv);

    sprintf(str,"towns=%d ", (int)(towns));
    strcat(messagestr,str);
  }

  /**** game speed ****/
  if (is_arg("-speed",argc,argv))
  {
    gamespeed = doubleval("-speed",argc,argv);
    if (gamespeed == 0.0)
      gamespeed = 0.5;
    delay = (int)(25000.0/gamespeed + 0.5);

    sprintf(str,"speed=%3.1f ",doubleval("-speed",argc,argv));
    strcat(messagestr,str);
  }

  if (is_arg("-board",argc,argv))
  {
    boardsizex = intval("-board",argc,argv);
    boardsizey = boardsizex;
  }

  if (is_arg("-boardx",argc,argv))
    boardsizex = intval("-boardx",argc,argv);

  if (is_arg("-boardy",argc,argv))
    boardsizey = intval("-boardy",argc,argv);

  /**** set up program for file storage ****/
  if (is_arg("-store",argc,argv))
  {
    enable_storage = TRUE;
    if (!stringval ("-store",argc,argv,storagename))
      strcpy (storagename, "xbattle.xba");
    strcat(messagestr,"store ");
  }

  if (is_arg("-load",argc,argv))
  {
    enable_load = TRUE;
    if (!stringval ("-load",argc,argv,mapname))
      strcpy (mapname, "xbattle.xbt");
    strcat(messagestr,"load ");
  }

  if (is_arg("-edit",argc,argv))
  {
    enable_edit = TRUE;
    if (!stringval ("-edit",argc,argv,mapname))
      strcpy (mapname, "xbattle.xbt");
    strcat(messagestr,"edit ");
  }

  /**** set up program for replay ****/
  if (is_arg("-replay",argc,argv))
  {
    enable_replay = TRUE;
    if (!stringval ("-replay",argc,argv,replayname))
      strcpy (replayname, "xbattle.xba");

    strcpy (messagestr, " REPLAY    REPLAY    REPLAY    REPLAY    REPLAY    REPLAY    REPLAY    REPLAY    REPLAY     REPLAY    REPLAY    REPLAY    REPLAY    REPLAY    REPLAY    REPLAY");
  }
}


/*************************************************************************/
/**	clean_options							**/
/** clean up incongruous options					**/
/**	Greg Lesher (lesher@park.bu.edu)				**/
/*************************************************************************/
clean_options(displayname)
  char displayname[MAXPLAYERS][80];
{
  int i;

  double full, base,
         offset, factor,
         sqrt();

  FILE *fptemp,
       *fopen();
      
  for (i=0; i<nsides; i++)
  {
    if (enable_map[i] || enable_localmap[i])
    {
      enable_horizon = TRUE;
      enable_personalhorizon[i] = TRUE;
    }

    if (enable_localmap[i])
    {
      enable_map[i] = TRUE;
      enable_basemap[i] = FALSE;
    }
  }

  if (enable_horizon)
  { 
    for (i=0; i<nsides; i++)
      if (enable_personalhorizon[i])
        if (viewrange[i] > maxviewrange)
          maxviewrange = viewrange[i];
  }

  if (enable_storage)
  {
    if ((fpout = fopen (storagename, "w")) == NULL)
    {
      printf ("Cannot open storage file %s\n", filename);
      exit (0);
    }
    storeparameters ();
  }

  if (enable_replay)
  {
    if ((fpout = fopen (replayname, "r")) == NULL)
    {
      printf ("Cannot open replay file %s\n", filename);
      exit (0);
    }
    loadparameters (displayname);
  }

  if (enable_load && !enable_edit)
  {
    if ((fptemp = fopen (mapname, "r")) == NULL)
    {
      printf ("Cannot open map file %s\n", mapname);
      exit (0);
    }

    fread (&boardsizex, sizeof(int), 1, fptemp);
    fread (&boardsizey, sizeof(int), 1, fptemp);
    fread (&enable_hex, sizeof(int), 1, fptemp);
    fread (&enable_hills, sizeof(int), 1, fptemp);
    fread (&enable_forest, sizeof(int), 1, fptemp);
    fread (&enable_sea, sizeof(int), 1, fptemp);
    fread (&enable_terrain, sizeof(int), 1, fptemp);

    fclose (fptemp);
  }

  if (enable_hex)
  {
    squaresize[0] = squaresize[0]/2;

    if (squaresize[0]%2 == 1)
      squaresize[0] -= 1;
  }

  if (enable_area)
  {
    full = ((double)(MAXVAL))/((MAXVAL+4)*(MAXVAL+4));

    offset = 0.18;
    factor = sqrt(full*MAXVAL)/(sqrt(full*MAXVAL)-offset); 


    for (i=0; i<=MAXVAL+4; i++)
    {
      areavalue[i] = (sqrt(full*(float)i)-offset)*factor;
      if (areavalue[i] < 0.0)
        areavalue[i] = 0.0;
    }
  }
  else
  {
    for (i=0; i<=MAXVAL+4; i++)
      areavalue[i] =  ((double)(i)) / (MAXVAL + 4);
  }

#if MULTITEXT
  for (i=0; i<nsides; i++)
    strcpy (personal_messagestr[i], messagestr);
#endif
}

