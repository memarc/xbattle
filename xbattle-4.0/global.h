#include <stdio.h>
  
/**** x include files ****/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include "extern.h"

/**** global variables ****/
xwindow_type *xwindow[MAXPLAYERS];

int colorarray[MAXPLAYERS],
    rcolorarray[MAXHUES],
    sidemap[MAXPLAYERS];

char lettermap[MAXSIDES][2];

int winopen[MAXPLAYERS],
    winwatch[MAXPLAYERS];

FILE *fpout, *fopen();
char filename[50];

/**** default mappings from color to b/w ****/
int color2bw[MAXHUES+1] = {NONE,
                           BLACK,
                           GRAY3,
                           GRAY2,
                           GRAY1,
                           WHITE,
                           BLACK,
                           WHITE,
                           BLACK,
                           WHITE,
                           BLACK,
                           WHITE};

double 	sea,	
 	farms,			
	gamespeed=5.0,
	towns;

double	slowfactor[MAXSIDES],
        artillery[MAXSIDES],
        paratroops[MAXSIDES],
        firepower[MAXSIDES],
        militia[MAXSIDES],
        hillfactor[MAXSIDES],	
	forest[MAXSIDES],	
        areavalue[MAXVAL+5],
	decayrate[MAXSIDES],
	eroderate[MAXSIDES],
        shuntval[MAXSIDES];

int     delay=10000,
	boardsizex=BOARDSIZE,
	boardsizey=BOARDSIZE;

int 	nplayers,
	nsides,	
	none,	
	dark,
 	light,
        maxviewrange,
	fillcost=MAXVAL-2,	
	digcost=MAXVAL-2,
        fillnumber,
        directions,
        paused=FALSE,
        dir[MAXPLAYERS][DIRS],
        dirtype[MAXPLAYERS],
        oldx[MAXPLAYERS],
        oldy[MAXPLAYERS],
	seed;

int	armies[MAXSIDES],
        bases[MAXSIDES],
        rbases[MAXSIDES],
        buildrate[MAXSIDES],
        scuttlerate[MAXSIDES],
        marchrate[MAXSIDES],
	viewrange[MAXSIDES],
	squaresize[MAXSIDES],
        textyh[MAXSIDES][MAXSIDES],
        textyl[MAXSIDES][MAXSIDES];

int 	enable_anywrap    = FALSE,
        enable_horizon    = FALSE,
        enable_march      = FALSE,
        enable_bound      = FALSE,
	enable_hills      = FALSE,
	enable_forest     = FALSE,
	enable_sea        = FALSE,
	enable_border     = FALSE,
	enable_terrain    = FALSE,
        enable_decay	  = FALSE,
        enable_area	  = FALSE,
        enable_anylocalmap = FALSE,
        enable_storage	  = FALSE,
        enable_replay     = FALSE,
        enable_hex        = FALSE,
        enable_edit       = FALSE,
        enable_load       = FALSE,
        enable_farms      = FALSE,
        enable_armies     = FALSE,
        enable_militia    = FALSE,
        enable_bases      = FALSE,
        enable_rbases     = FALSE,
        enable_towns      = FALSE;

int	enable_dig[MAXSIDES],
   	enable_fill[MAXSIDES],
   	enable_scuttle[MAXSIDES],
   	enable_build[MAXSIDES],
	enable_artillery[MAXSIDES],
	enable_paratroops[MAXSIDES],
        enable_personalmarch[MAXSIDES],
        enable_grid[MAXSIDES],
        enable_manpos[MAXSIDES],
        enable_wrap[MAXSIDES],
	enable_hidden[MAXSIDES],
	enable_attack[MAXSIDES],
	enable_nospigot[MAXSIDES],
        enable_disrupt[MAXSIDES],
        enable_personalhorizon[MAXSIDES],
        enable_personalbound[MAXSIDES],
        enable_erode[MAXSIDES],
        enable_repeat[MAXSIDES],
        enable_map[MAXSIDES],
        enable_basemap[MAXSIDES],
        enable_localmap[MAXSIDES],
        enable_reserve[MAXSIDES],
        enable_digin[MAXSIDES];

char outdated[MAXBOARDSIZE][MAXBOARDSIZE],
     oldvalue[MAXBOARDSIZE][MAXBOARDSIZE];

int hex_side,
    hex_halfside,
    hex_quarterside,
    hex_3quarterside,
    hex_vert,
    hex_halfvert,
    hex_chart[2*MAXSQUARESIZE][2*MAXSQUARESIZE][2],
    hex_horizon_even[200][2], hex_horizon_odd[200][2],
    hex_horizon_number;
double hex_slope;
XPoint hex_points[7];

double drand48();
void srand48();

char graytitle[MAXSIDES+1][40] = {"blank",
                                  "xbattle BLACK SIDE",
                                  "xbattle DARK SIDE",
                                  "xbattle GRAY2 SIDE",
                                  "xbattle LIGHT SIDE",
                                  "xbattle WHITE SIDE"};
char huetitle[MAXSIDES+1][40] = {"blank",
                                 "xbattle BLACK SIDE",
                                 "xbattle DARK SIDE",
                                 "xbattle GRAY2 SIDE",
                                 "xbattle LIGHT SIDE",
                                 "xbattle WHITE SIDE",
                                 "xbattle RED SIDE",
                                 "xbattle CYAN SIDE",
                                 "xbattle MAGENTA SIDE",
                                 "xbattle GREEN SIDE",
                                 "xbattle BLUE SIDE",
                                 "xbattle YELLOW SIDE"};

char grayname[MAXHUES+1][10] = {"blank",
                                "black",
                                "dark",
                                "gray2",
                                "light",
                                "white",
                                "junk",
                                "junk",
                                "junk",
                                "junk",
                                "junk",
                                "junk"};
char huename[MAXHUES+1][10] = {"blank",
			       "black",
                               "dark",
                               "gray2",
                               "light",
                               "white",
                               "red",
                               "cyan",
                               "magenta",
                               "green",
                               "blue",
                               "yellow"};

char messagestr[512],
     personal_messagestr[MAXSIDES][512],
     storagename[100],
     replayname[100],
     mapname[100],
     blankline[]   = {"                                                                                          "};

int palette[MAXSIDES+1][3] = {{210, 220, 150},
			      {  0,   0,   0},
			      {140, 140, 140},
			      {160, 160, 160},
			      {180, 180, 180},
			      {255, 255, 255},
			      {255,   0,   0},
			      {100, 255, 210},
			      {255,   0, 255},
			      {130, 255,   0},
			      {  0,   0, 175},
			      {250, 250, 130}};

double hillslope[3] = {5.0, -9.0, 0.0};
double hillintersect[3] = {175.0, 220.0, 100};

int hillpalette[3] =   {165, 170,  90},
    forestpalette[3] = {  0, 130,   0},
    seapalette[3] =    { 80, 152, 224};
/**
    seapalette[3] =    {  0,   0, 150};
**/

char *usage = "\
USAGE: xbattle\n\
                -<c1>      <display>      display name side uses color c1 with\n\
                                          default color to b/w mapping\n\
                -<c1><c2>  <display>      display name side uses color c1 with\n\
                                          b/w terminals using b/w shade c2\n\
                -hex                      Use hexagonal display \n\
                -bases     <n>            number of bases per side\n\
                -rbases    <n>            number of randomly placed bases per side\n\
                -armies    <n>            initial number of armies per side\n\
                -militia   <n>            density of random militia\n\
                -hills     <1-10>         hill steepness\n\
                -forest    <1-10>         forest thickness\n\
                -sea       <1-10>         sea density\n\
                -farms     <1-10>         max farm production rate\n\
                -decay     <1-10>         troop decay rate\n\
                -erode     <1-10>         empty square erode rate\n\
                -towns     <1-10>         density of towns\n\
                -guns      <1-10>         range of artillery\n\
                -para      <1-10>         range of paratroopers\n\
                -fight     <1-10>         fire power\n\
                -speed     <1-10>         game speed\n\
                -move      <1-10>         terrain roughness\n\
                -digin     <1-10>         enable shunting\n\
                -seed      <n>            random number seed for hills etc\n\
                -horizon   <n>            limit range of vision to n spaces\n\
                -map                      no preknown map -  drawn during play \n\
                -basemap                  remember position of towns/bases/rbases \n\
                -localmap                 no terrain memory \n\
                -dig       <n>            allow digging of sea, n full squares \n\
                -fill      <n>            allow filling of sea, n full squares \n\
                -scuttle                  allow scuttling of bases \n\
                -reserve                  allow troop reserves \n\
                -build     <n>            build base with <n> segments \n\
                -march     <n>            allow troop march, speed <n> \n\
                -hidden	                  obscure enemy movement arrows\n\
                -area	                  use areal display mode for square size\n\
                -bound	                  allow bounding box\n\
                -repeat	                  allow repeat of last command via right button\n\
                -disrupt                  attack disrupts movement\n\
                -wrap                     wraparound mode\n\
                -border                   border of sea\n\
                -nogrid                   do not draw grid\n\
                -manpos                   do not automatically position screen\n\
                -square    <n>            size of game square in pixels\n\
                -board     <n>            number of game squares on a side\n\
                -boardx    <n>            number of game squares on x side\n\
                -boardy    <n>            number of game squares on y side\n\
                -store     <file>         store game in <file> for replay\n\
                -replay    <file>         replay game in <file>\n\
                -options   <file>         use options from <file>\n\
                -load      <file>         use map from <file>\n\
                -edit      <file>         enter terrain editing mode\n";

char options[NOPTS][10] = 	{"-bases",
				 "-rbases",
				 "-armies",
				 "-militia",
				 "-hills",
				 "-forest",
				 "-sea",
				 "-farms",
				 "-decay",
				 "-erode",
				 "-towns",
				 "-guns",
				 "-para",
				 "-march",
				 "-fight",
				 "-speed",
				 "-move",
				 "-digin",
				 "-seed",
				 "-dig",
				 "-fill",
				 "-square",
				 "-board",
				 "-boardx",
				 "-boardy",
				 "-build",	/** end of non flags **/
				 "-horizon",
				 "-options",
				 "-load",
				 "-edit",
				 "-store",
				 "-replay",     /** end of variable flags **/
				 "-hex",
				 "-map",
				 "-basemap",
				 "-localmap",
				 "-scuttle",
                                 "-reserve",
				 "-hidden",
				 "-area",
				 "-disrupt",
				 "-bound",
				 "-repeat",
				 "-wrap",
				 "-border",
				 "-attack",
				 "-nospigot",
				 "-nogrid",
				 "-manpos"};
