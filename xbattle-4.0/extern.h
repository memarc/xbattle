/**** game static parameters ****/

#define BOARDSIZE 	15
#define SQUARESIZE 	45
#define MAXBOARDSIZE 	50
#define MAXSQUARESIZE 	75
#define TEXTSIZE 	16
#define CSIZE 		5
#define DSIZE 		7
#define TEXTX 		0
#define NGREYTONES 	3
#define NHILLTONES 	5
#define NFORESTTONES 	5
#define NSEATONES 	2
#define SEAVALUE	0.87840
#define MAXPLAYERS 	11
#define MAXSIDES   	11
#define MAXHUES   	11

#define NOPTS		49
#define POPTS		26
#define PVOPTS		6

#define SET		0
#define FORCE		1

#define ACTIVE		1
#define PASSIVE		2
#define TEMP		3

/**** game dynamic parameters ****/
#define MAXVAL 		20
#define FIREPOWER	5.0
#define SLOWFACTOR  	3.0
#define SHELL		1
#define SHELL_COST	1
#define PARA		1
#define PARA_COST	3
#define VIEWRANGE  	2
#define BASERANGE	2
#define MARCH		3
#define ERODETHRESH	0.04

#define SQ3D2		0.8660254

/**** x window default parameters ****/
#define FONT  		"-adobe-courier-bold-r-normal--*-100-*-*-*-*-iso8859-1"
#define BORDER 		1
#define XPOS		0
#define YPOS		0

/**** necessary constants ****/
#define MAXCOLORS  	MAXSIDES+NHILLTONES+3
#define ALL		MAXHUES+2
#define TRUE	   	1
#define FALSE	   	0
#define OK		-1
#define GLOBAL		MAXSIDES+2
#define NOT_TAKEN	-1

/**** COMPILE TIME OPTIONS ****/

#define UNIX		TRUE
#define INVERT		TRUE
#define PAUSE		TRUE
#define VARMOUSE	FALSE
#define NODRAND48	FALSE
#define SHOWFLOW	TRUE
#define MULTITEXT	TRUE
#define NEWCOLORMAP	FALSE
#define MULTIFLUSH	FALSE

/**** special flags ****/
#define DIRS   		6

#define HEX_UP  	0
#define HEX_RIGHT_UP 	1
#define HEX_RIGHT_DOWN 	2
#define HEX_DOWN	3
#define HEX_LEFT_DOWN 	4
#define HEX_LEFT_UP 	5

#define UP  		0
#define DOWN	  	2
#define RIGHT 		1
#define LEFT  		3

/**
#define UP  		0
#define DOWN	  	2
#define RIGHT 		1
#define LEFT  		3
**/

/**** board square access routine ****/
#define SQUARE(board,x,y) ((board)+(y)*boardsizex+(x))


/**** window structure ****/
typedef struct{
    Display 		*display;
    Window 		window;
    int			xsize,ysize;
    XSizeHints 		hint;
    XWMHints 		xwmh;
    int 		depth,screen;
    GC 			hue[MAXSIDES+3], flip, gc_clear, gc_or;
    Pixmap		terrain[NHILLTONES];
    XFontStruct 	*font_struct;
    Colormap 		cmap;
    XColor 		xcolor[MAXCOLORS];
    char		drawletter[MAXSIDES+1];
    char		letter[MAXSIDES+1][2];
    unsigned long	charwidth, charheight;
} xwindow_type;

/**** game square structure ****/
typedef struct SquareType {
    char 		color;
    char 		move,dir[DIRS];
    char		anymarch, marchcolor, marchcount;
    char		march[MAXSIDES],marchdir[MAXSIDES][DIRS],marchtype[MAXSIDES];
    short 		value[MAXHUES+3];
    short		lowbound;
    char 		growth;
    char 		oldgrowth;
    char	 	oldcolor;
    char		age;
    char 		x, y;
    int			xpos, ypos;
    struct SquareType	*connect[DIRS];
    char		seen[MAXSIDES];
    int			angle;
} square_type;

/**** square colors ****/
#define NONE  		0
#define BLACK 		1
#define GRAY3 		2
#define GRAY2 		3
#define GRAY1 		4
#define WHITE 		5
#define RED		6
#define CYAN		7
#define MAGENTA		8
#define GREEN		9
#define BLUE		10
#define YELLOW		11
#define FIGHT  		MAXHUES+1

/**** wraparound routine ****/
#define MODX(x)  (x+boardsizex)%boardsizex
#define MODY(y)  (y+boardsizey)%boardsizey

/**** ascii codes ****/
#define RETURN 13
#define BACKSPACE 8
#define DELETE 127
#define CTRLC 03
#define CTRLQ 17
#define CTRLG 07
#define CTRLW 23
#define SPACE 32
#define CTRLF 6
#define CTRLD 4
#define CTRLS 19
#define CTRLB 2
#define CTRLP 16

/**** global variables ****/
extern xwindow_type *xwindow[MAXPLAYERS];

extern int colorarray[MAXPLAYERS],
           rcolorarray[MAXHUES],
           sidemap[MAXPLAYERS];
 
extern char lettermap[MAXSIDES][2];

extern int winopen[MAXPLAYERS],
           winwatch[MAXPLAYERS];

extern FILE *fpout, *fopen();
extern char filename[50];

/**** default mappings from color to b/w ****/
extern int color2bw[MAXHUES+1];

extern double 	towns,
		sea,
		gamespeed,
 		farms;

extern double	slowfactor[MAXSIDES],
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

extern int	delay,
		boardsizex,
		boardsizey,
		paused,
		seed;

extern int 	nplayers,
		nsides,	
		none,
		dark,
		light,	
		maxviewrange,
		fillcost,
		digcost,
		fillnumber,
		directions,
                oldx[MAXPLAYERS],
                oldy[MAXPLAYERS],
                dir[MAXPLAYERS][DIRS],
                dirtype[MAXPLAYERS];

extern int	armies[MAXSIDES],
		bases[MAXSIDES],
		rbases[MAXSIDES],
		buildrate[MAXSIDES],
		scuttlerate[MAXSIDES],
		marchrate[MAXSIDES],
		viewrange[MAXSIDES],
		squaresize[MAXSIDES],
       		textyh[MAXSIDES][MAXSIDES],
                textyl[MAXSIDES][MAXSIDES];

extern int 	enable_anywrap,
                enable_horizon,
                enable_march,
                enable_bound,
	        enable_hills,
	        enable_forest,
	        enable_sea,
	        enable_hex,
	        enable_border,
	        enable_terrain,
		enable_decay,
		enable_anylocalmap,
                enable_storage,
                enable_replay,
                enable_area,
                enable_edit,
                enable_load,
		enable_farms,
		enable_towns,
		enable_rbases,
		enable_bases,
		enable_militia,
		enable_armies;

extern int 	enable_fill[MAXSIDES],
	        enable_dig[MAXSIDES],
	        enable_scuttle[MAXSIDES],
	        enable_build[MAXSIDES],
	        enable_artillery[MAXSIDES],
	        enable_paratroops[MAXSIDES],
                enable_personalmarch[MAXSIDES],
                enable_grid[MAXSIDES],
                enable_manpos[MAXSIDES],
                enable_wrap[MAXSIDES],
                enable_personalhorizon[MAXSIDES],
                enable_personalbound[MAXSIDES],
                enable_erode[MAXSIDES],
                enable_repeat[MAXSIDES],
                enable_map[MAXSIDES],
                enable_basemap[MAXSIDES],
                enable_localmap[MAXSIDES],
	        enable_hidden[MAXSIDES],
	        enable_attack[MAXSIDES],
	        enable_nospigot[MAXSIDES],
                enable_disrupt[MAXSIDES],
		enable_reserve[MAXSIDES],
		enable_digin[MAXSIDES];
        
extern char outdated[MAXBOARDSIZE][MAXBOARDSIZE],
            oldvalue[MAXBOARDSIZE][MAXBOARDSIZE];

double drand48();
void srand48();

extern char graytitle[MAXSIDES+1][40];
extern char huetitle[MAXSIDES+1][40];
extern char grayname[MAXHUES+1][10];
extern char huename[MAXHUES+1][10];
extern char personal_messagestr[MAXSIDES][512],
            messagestr[512],
            storagename[100],
            replayname[100],
            mapname[100],
            blankline[150];

extern int palette[MAXSIDES+1][3];
extern double hillslope[3];
extern double hillintersect[3];

extern int hillpalette[3],
           forestpalette[3],
           seapalette[3];
/**
           seapalette[3] =    {  0,   0, 150};
**/

extern char *usage;

extern char options[NOPTS][10];

extern int hex_side,
           hex_halfside,
           hex_quarterside,
           hex_3quarterside,
           hex_vert,
           hex_halfvert,
           hex_chart[2*MAXSQUARESIZE][2*MAXSQUARESIZE][2],
           hex_horizon_even[200][2], hex_horizon_odd[200][2],
           hex_horizon_number;
extern double hex_slope;
extern XPoint hex_points[7]; 
