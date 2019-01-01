#include <stdio.h>
#include <math.h>

#include "constant.h"
  
/**** x include files ****/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#if USE_LONGJMP
#include <setjmp.h>
#endif

#include "extern.h"


/******************************************************************************
  init_board ()

  Create the game board and initialize each cell of the board.
******************************************************************************/

init_board ()
{
  int i, j, k,
      side;

  cell_type *cell;

  /** Initialize statistics **/

  for (side=0; side<Config->side_count; side++)
  {
    Config->stats[side] = (statistic_type *)(malloc(sizeof(statistic_type)));

    Config->stats[side]->build_count =	0;
    Config->stats[side]->troop_count =	0;
    Config->stats[side]->cell_count =	0;
  }

  /** Initialize random number generator **/

  srand((long)Config->value_int_all[OPTION_SEED]);

  printf ("seed: %d\n", Config->value_int_all[OPTION_SEED]);

  /** Initialize the global <Board> structure **/

  Board  = (board_type *)(malloc(sizeof(board_type)));
  Board->cell_count = 0;

  /** Allocate all cells and their dynamic components **/

  for (j=0; j<Config->board_y_size; j++)
    for (i=0; i<Config->board_x_size; i++)
    {
      cell = (cell_type *)(malloc(sizeof(cell_type)));
      Board->cells[j][i] = cell;
      Board->list[Board->cell_count++] = cell;

      cell->dir =	(s_char *)(malloc(sizeof(s_char)*(Config->direction_count)));
      cell->value =	(s_char *)(malloc(sizeof(s_char)*(Config->side_count)));
      cell->march =	(s_char *)(malloc(sizeof(s_char)*(Config->side_count)));
      cell->march_type =(s_char *)(malloc(sizeof(s_char)*(Config->side_count)));
      cell->march_dir =	(s_char *)(malloc(sizeof(s_char)*(Config->side_count)));
      cell->seen =	(s_char *)(malloc(sizeof(s_char)*(Config->side_count)));
      cell->draw_level = (s_char *)(malloc(sizeof(s_char)*(Config->side_count)));

      cell->x_center =	(short *)(malloc(sizeof(short)*(Config->side_count)));
      cell->y_center =	(short *)(malloc(sizeof(short)*(Config->side_count)));
    }

  /** Initialize each cell's values **/

  for (j=0; j<Config->board_y_size; j++)
  {
    /** For each column **/
  
    for (i=0; i<Config->board_x_size; i++)
    {
      cell = CELL2(i,j);

      /** Initialize salient variables **/

      cell->x		 	= i;
      cell->y		 	= j;
      cell->side	 	= SIDE_NONE;
      cell->old_side	 	= SIDE_NONE;
      cell->lowbound	 	= 0;
      cell->growth	 	= 0;
      cell->old_growth		= 0;
      cell->angle	 	= 0;
      cell->age	 		= 0;
      cell->any_march	 	= FALSE;
      cell->move	 	= 0;
      cell->manage_update 	= FALSE;
      cell->manage_x	 	= 0;
      cell->manage_y	 	= 0;
      cell->level		= 0;
      cell->side_count		= 0;
      cell->shape_index		= 0;

      cell->outdated	 	= OUTDATE_ALL;
      cell->redraw_status	= REDRAW_NORMAL;
      cell->old_value	 	= -1;

      /** Set all parameters of side-varying nature **/

      for (side=0; side<Config->side_count; side++)
      {
        cell->value[side] 	= 0;
        cell->march[side] 	= FALSE;
        cell->seen[side] 	= TRUE;
        cell->draw_level[side] 	= 0;
      }

      /** Null out all direction vectors **/

      for (k=0; k<Config->direction_count; k++)
        cell->dir[k]	 	= 0;

      /** NOTE: cell position defined in shape_initialize() **/
    }
  }

  /** Load board from file if specified **/

  if (Config->enable_all[OPTION_LOAD])
    load_board (Config->use_brief_load);

  /** Initialize all the tiling-dependent aspects of the board **/

  shape_initialize ();

  /** If using interactive edit, create some terrain if using overwrite **/

  if (Config->enable_all[OPTION_EDIT])
  {
    if (Config->enable_all[OPTION_OVERWRITE] &&
			!Config->enable_all[OPTION_LOAD])
      init_terrain ();
    return;
  }

  if (Config->enable_terrain && !Config->enable_all[OPTION_LOAD])
    init_terrain ();

  /** Initialize var self-explanatory aspects of the board **/

  if (Config->enable_all[OPTION_FARMS])
    init_farms ();

  if (Config->enable_all[OPTION_TOWNS])
    init_towns ();

  if (Config->enable_all[OPTION_BASES])
    init_bases_or_armies (OPTION_BASES);

  if (Config->enable_all[OPTION_RBASES])
    init_rbases ();
 
  if (Config->enable_all[OPTION_ARMIES])
    init_bases_or_armies (OPTION_ARMIES);

  if (Config->enable_all[OPTION_MILITIA])
    init_militia (TRUE);

  /** Initialize visibility of each cell **/

  if (!Config->enable_all[OPTION_LOAD] || Config->enable_all[OPTION_OVERWRITE])
    init_unseen ();
}



/******************************************************************************
  init_unseen ()

  If using any type of map-as-you-go option set, render all cells unseen.
******************************************************************************/

init_unseen ()
{
  int i,
      side;

  for (side=0; side<Config->side_count; side++)
  {
    if (Config->enable[OPTION_MAP][side] ||
		Config->enable[OPTION_LOCALMAP][side])
    {
      for (i=0; i<Board->cell_count; i++)
        CELL(i)->seen[side] = FALSE;
    }
  }
}


/******************************************************************************
  init_terrain ()

  Initialize hills, forests, and seas across the game board using a peak
  algorithm developed by Jack Bennetto (bennetto@physics.rutgers.edu), with
  analogous trough algorithm tacked on.
******************************************************************************/

init_terrain ()
{
  int i, j, k,
      x_peak[MAX_PEAKS],
      y_peak[MAX_PEAKS],
      z_peak[MAX_PEAKS],
      elevation_bin[ELEVATION_BINS],
      elevation_to_level[ELEVATION_BINS],
      peak_count,
      cell_count,
      level_count,
      cell_limit,
      level,
      bin_index,
      dist_squared;

  double denominator,
         elevation[MAX_BOARDSIZE][MAX_BOARDSIZE],
         fractions[100],
         fraction,
         pow(),
         sea_base_fraction;

  cell_type *cell;

  /** Determine the number of peaks (and troughs **/

  peak_count = (int)(Config->value_int_all[OPTION_PEAKS] *
			Board->cell_count * PEAK_MULTIPLIER);

  if (peak_count > MAX_PEAKS)
    peak_count = MAX_PEAKS;

  /** For each peak, set random position and height **/

  for (i=0; i<peak_count; i++)
  {
    x_peak[i] = get_random (Config->board_x_size);
    y_peak[i] = get_random (Config->board_y_size);
    z_peak[i] = get_random (ELEVATION_BINS) - ELEVATION_OFFSET;
  }

  /** Zero out the elevation bins **/

  for (i=0; i<ELEVATION_BINS; i++)
    elevation_bin[i] = 0;

  /** For each board position **/

  for (i=0; i<Config->board_x_size; i++)
    for (j=0; j<Config->board_y_size; j++)
    {
      cell = CELL2(i,j);
      elevation[i][j] = 0.0;
      denominator = 0.0;

      /** For each peak, add elevation to cell, normalizing by the	**/
      /** square root of the distance to the peak.			**/

      for (k=0; k<peak_count; k++)
      {
        dist_squared = PEAK_X_FACTOR*(i-x_peak[k])*(i-x_peak[k]) +
			PEAK_Y_FACTOR*(j-y_peak[k])*(j-y_peak[k]);
        if (dist_squared == 0)
        {
          elevation[i][j] += z_peak[k];
          denominator += 1.0;
          break;
        }
        else
        {
          elevation[i][j] += z_peak[k]/dist_squared;
          denominator += 1.0/dist_squared;
        }
      }

      elevation[i][j] /= (denominator + 0.05);

      elevation_bin[ELEVATION_OFFSET + (int)(elevation[i][j])]++;
    }

  /** Now we start the complicated routines to normalize the various	**/
  /** levels using a power biasing routine.				**/

  /** Compute the fraction of cells that should be at sea levels **/

  sea_base_fraction = Config->value_double_all[OPTION_SEA]/10.0;
  fraction = 0.0;

  /** Compute the biased fraction of cells at each sea level **/

  if (Config->level_min < 0)
  {
    level_count = -Config->level_min;
    for (level = -1; level >= Config->level_min; level--)
    {
      fraction = pow ((double)(-level),
		Config->value_double_all[OPTION_TROUGH_BIAS])/level_count;
      fractions[level - Config->level_min] = fraction;
    }
 
    /** Normalize values **/

    fraction = sea_base_fraction/fractions[0];
    for (level = Config->level_min; level < -1; level++)
      fractions[level - Config->level_min] = sea_base_fraction -
      		fraction * fractions[level - Config->level_min + 1];
    fractions[level - Config->level_min] = sea_base_fraction;
  }

  /** Compute the biased fraction of cells at each non-sea level **/

  level_count = Config->level_max + 1;
  for (level = 0; level <= Config->level_max; level++)
  {
    fraction = pow ((double)(level+1),
		Config->value_double_all[OPTION_PEAK_BIAS])/level_count;
    fractions[level - Config->level_min] = fraction;
  }

  /** Normalize values **/

  fraction = (1.0 - sea_base_fraction)/
		fractions[Config->level_max - Config->level_min];
  for (level = 0; level <= Config->level_max; level++)
    fractions[level - Config->level_min] = sea_base_fraction +
		 fraction * fractions[level - Config->level_min];

  cell_count = 0;
  bin_index = 0;

  /** Set the actual levels by incrementally stepping through the	**/
  /** elevation bins and comparing count to fractional levels specified	**/
  /** by above computations.						**/

  for (level = Config->level_min; level <= Config->level_max; level++)
  {
    cell_limit = fractions[level - Config->level_min] * Board->cell_count;

    for (; cell_count < cell_limit &&
		bin_index < ELEVATION_BINS; bin_index++)
    {
      cell_count += elevation_bin[bin_index];
      elevation_to_level[bin_index] = level;
    }
  }

  /** Set the remainder of the bins to the highest level **/

  for (; bin_index<ELEVATION_BINS; bin_index++)
    elevation_to_level[bin_index] = Config->level_max;

  /** Set level of each cell **/

  for (i=0; i<Config->board_x_size; i++)
    for (j=0; j<Config->board_y_size; j++)
    {
      cell = CELL2(i,j);
      cell->level =
	elevation_to_level[ELEVATION_OFFSET + (int)(elevation[i][j])];

      if (Config->enable_all[OPTION_SEA_BLOCK] && cell->level < 0)
        cell->level = Config->level_min;
    }
}



/******************************************************************************
  init_towns ()

  Initialize troop producing towns randomly across the game board.
******************************************************************************/

init_towns ()
{
  int i;

  for (i=0; i<Board->cell_count; i++)
    if (get_random (100) <=
		Config->value_int_all[OPTION_TOWNS]*TOWN_MULTIPLIER)
    {
       if (CELL(i)->level >= 0)
       {
         CELL(i)->growth = TOWN_MIN +
			get_random (TOWN_MAX-TOWN_MIN);
         CELL(i)->old_growth = CELL(i)->growth;
         CELL(i)->angle = ANGLE_FULL; 
       }
     }
}



/******************************************************************************
  init_farms ()

  Initialize troop producing farms uniformly across the game board.
******************************************************************************/

init_farms ()
{
  int i, j;

  for (i=0; i<Board->cell_count; i++)
    if (CELL(i)->level >= 0)
    {
      if (CELL(i)->growth < TOWN_MIN)
        CELL(i)->growth = (int)(Config->value_int_all[OPTION_FARMS]);
    }
}



/******************************************************************************
  init_rbases ()

  Initialize troop producing bases (occupied big cities) randomly across game
  board, never putting opposite side bases so close together that they can
  be seen within the horizon.
******************************************************************************/

init_rbases ()
{
  int i, j, k, x, y,
      i1, i2, j1, j2,
      side,
      range,
      max_rbase_count,
      done,
      count;

  /** Determine maximum number of rbases **/

  max_rbase_count = 0;
  for (side=0; side<Config->side_count; side++)
    if (Config->value_int[OPTION_RBASES][side] > max_rbase_count)
      max_rbase_count = Config->value_int[OPTION_RBASES][side];

  /** Set the minimum distance between rbases of opposing side **/

  if (Config->view_range_max < Config->value_int_all[OPTION_RBASE_RANGE])
    range = Config->value_int_all[OPTION_RBASE_RANGE];
  else
    range = Config->view_range_max;

  /** Create rbases, alternating between sides **/

  for (k=0; k<max_rbase_count; k++)
  {
    for (side=0; side<Config->side_count; side++)
    {
      if (!Config->enable[OPTION_RBASES][side])
        continue;

      if (k >= Config->value_int[OPTION_RBASES][side])
        continue;

      /** Keep trying to create an rbase far enough away from enemies **/

      done = FALSE;
      count = 0;

      while (!done)
      {
        done = TRUE;

        /** Pick a random location on the game board **/

        i = get_random (Config->board_x_size);
        j = get_random (Config->board_y_size);

        /** If location is already a base or town, forget it **/

        if (CELL2(i,j)->growth > TOWN_MIN || CELL2(i,j)->level < 0)
          done = FALSE;

        /** Check neighboring locations for enemy bases **/

        i1 = i-range;
        i2 = i+range;
        j1 = j-range;
        j2 = j+range;

        if (Config->enable_all[OPTION_WRAP])
        {
          for (x=i1; x<=i2; x++)
            for (y=j1; y<=j2; y++)
              if (CELL2(MODX(x),MODY(y))->side != side &&
			CELL2(MODX(x),MODY(y))->side != SIDE_NONE) 
                done = FALSE;
        }
        else
        {
          i1 = (i1<0) ? 0 : i1;
          i2 = (i2>Config->board_x_size-1) ? Config->board_x_size-1 : i2;
          j1 = (j1<0) ? 0 : j1;
          j2 = (j2>Config->board_y_size-1) ? Config->board_y_size-1 : j2;

          for (x=i1; x<=i2; x++)
            for (y=j1; y<=j2; y++)
              if (CELL2(x,y)->side != side &&
			CELL2(x,y)->side != SIDE_NONE)
                done = FALSE;
        }
        count++;

        /** If haven't found a valid location after 1000 tries, exit **/

        if (count > 1000)
          done = TRUE;
      }

      /** If found a valid location, set up base **/

      if (count < 1000)
        init_single_cell (CELL2(i,j), side, 1, 100);
    }
  }
}



/******************************************************************************
  init_bases_or_armies (option)

  Initialize regular lines of armies or bases (depending on <option>), using a
  tangential-circular paradigm.
******************************************************************************/

init_bases_or_armies (option)
{
  int side,
      center_x, center_y,
      range_x, range_y,
      base_x, base_y,
      x, y,
      offset,
      count_limit,
      growth,
      value,
      count;

  double angle,
         angle_step,
         angle_tangent,
         cos(), sin();

  cell_type *cell,
            *last_cell;

  /** Set values of cells to be initialized **/

  if (option == OPTION_BASES)
  {
    growth = 100;
    value = 1;
    offset = 2;
  }
  else
  {
    growth = 0;
    offset = 3;
  }

  /** Set circular angle based on number of sides **/

  if (Config->side_count == 3)
    angle_step = 2.0 * PI / 4.0;
  else
    angle_step = 2.0 * PI / Config->side_count;

  /** Find center coordinates and determine radius of big circle **/

  center_x = Board->size[0].x/2;
  center_y = Board->size[0].y/2;

  cell = CELL2(offset, Config->board_y_size/2);
  range_x = center_x - cell->x_center[0];

  cell = CELL2(Config->board_x_size/2, offset);
  range_y = center_y - cell->y_center[0];

  /** For each side **/

  for (side=0, angle=0.0; side<Config->side_count; side++,angle+=angle_step)
  {
    if (!Config->enable[option][side])
      continue;

    /** Compute and initialize first base/army location **/

    base_x = center_x + (int)(range_x * sin (angle));
    base_y = center_y + (int)(range_y * cos (angle));

    last_cell = get_cell (base_x, base_y, NULL, 0, FALSE);

    if (option != OPTION_BASES)
      value = Board->shapes[side][last_cell->shape_index]->max_value;

    init_single_cell (last_cell, side, value, growth);

    angle_tangent = PI/2.0 + angle;

    count = 0;
    count_limit = Config->value_int[option][side]/2;

    /** For each other army/base to the clockwise, keep inching over	**/
    /** until a new, unitialized cell is found.				**/

    for (offset=0; count<count_limit; offset++)
    {
      x = base_x + (int)(offset * sin (angle_tangent));
      y = base_y + (int)(offset * cos (angle_tangent));
      cell = get_cell (x, y, NULL, 0, FALSE);

      if (cell == NULL)
      {
        count = count_limit;
        continue;
      }

      if (last_cell != cell)
      {
        if (option != OPTION_BASES)
          value = Board->shapes[side][cell->shape_index]->max_value;
        init_single_cell (cell, side, value, growth);
        last_cell = cell;
        count++;
      }
    }

    last_cell = get_cell (base_x, base_y, NULL, 0, FALSE);
    count = 0;
    count_limit = (Config->value_int[option][side]-1)/2;

    /** For each other army/base to the counterclockwise, keep inching	**/
    /** over until a new, unitialized cell is found.			**/

    for (offset=0; count<count_limit; offset++)
    {
      x = base_x + (int)(-offset * sin (angle_tangent));
      y = base_y + (int)(-offset * cos (angle_tangent));
      cell = get_cell (x, y, NULL, 0, FALSE);

      if (cell == NULL)
      {
        count = count_limit;
        continue;
      }

      if (last_cell != cell)
      {
        if (option != OPTION_BASES)
          value = Board->shapes[side][cell->shape_index]->max_value;
        init_single_cell (cell, side, value, growth);
        last_cell = cell;
        count++;
      }
    }
  }
}



/******************************************************************************
  init_militia (use_normalization)

  Initialize troops in a random pattern across game board.  Normalize the
  number of troops per side if <use_normalization> is set.
******************************************************************************/

init_militia (use_normalization)
  int use_normalization;
{
  int i, j,
      side,
      this_side,
      scramble,
      count,
      total, remain, amount;

  short max_value;

  /** FIX: Get rid of militia normalization kludge **/

  /** Override normalization if any militia is over 100 **/

  if (Config->value_int[OPTION_MILITIA][0] > 100)
  {
    for (side=0; side<Config->side_count; side++)
      Config->value_int[OPTION_MILITIA][side] -= 100;
    use_normalization = FALSE;
  }

  /** If routine should normalize total number of troops per side **/

  if (use_normalization)
  {
    /** Set total to 0.75 percent (default) of all possible troops **/

    total = (Config->board_x_size*Config->board_y_size * Config->max_max_value)
			* MILITIA_MULTIPLIER;

    /** For each side, assign troops **/

    for (side=0; side<Config->side_count; side++)
    {
      if (!Config->enable[OPTION_MILITIA][side])
        continue;

      /** Compute total number of troops per side **/

      remain = total*Config->value_int[OPTION_MILITIA][side];

      /** While not all troops have been allocated **/

      while (remain > 0)
      {
        /** Set some random amount of troops (or remainder) **/

        amount = get_random (Config->max_value[side]) + 1;
        if (amount > remain)
          amount = remain;

        count = 0;
        while (count < LOOP_LIMIT)
        {
          /** Randomly pick a cell **/

          i = get_random (Config->board_x_size);
          j = get_random (Config->board_y_size);

          /** If cell is not sea and not owned by another side **/

          if (CELL2(i,j)->level >= 0 && CELL2(i,j)->side == SIDE_NONE)
          {
            CELL2(i,j)->side = side;

            max_value =
		Board->shapes[side][CELL2(i,j)->shape_index]->max_value;

            if (amount > max_value)
              amount = max_value;

            CELL2(i,j)->value[side]  = amount;

            remain = remain - amount;
            count = LOOP_LIMIT+1;
          }
          else
            count++;
        }

        /** Show error message if there is just too many troops **/

        if (count == LOOP_LIMIT)
          throw_error ("Cannot fit militia on screen", NULL);
      }
    }
  }
  else
  {
    /** Else no normalization necessary **/

    scramble = 0;

    /** For each game board cell **/

    for (i=0; i<Config->board_x_size; i++)
      for (j=0; j<Config->board_y_size; j++)
      {
        /** Determine a random side to start with **/

        scramble = get_random (Config->side_count);
  
        for (side=0; side<Config->side_count; side++)
        {
          if (!Config->enable[OPTION_MILITIA][side])
            continue;

          /** Step through each side **/

          this_side = (scramble + side)%Config->side_count;

          /** If random value less than militia param, set militia **/

          if (get_random (100) <= Config->value_int[OPTION_MILITIA][this_side])
          {
            if (CELL2(i,j)->level >= 0)
            {
	      CELL2(i,j)->side =  this_side;
              max_value =
		Board->shapes[this_side][CELL2(i,j)->shape_index]->max_value;
	      CELL2(i,j)->value[this_side] = get_random (max_value) + 1;
              side = Config->side_count;
            }
          }
        }
      }
    }
}



/******************************************************************************
  init_single_cell (cell, side, value, growth)

  Initialize a single cell with troops and growth factors.
******************************************************************************/

init_single_cell (cell, side, value, growth)
  cell_type *cell;
  int side,
      value,
      growth;
{
  /** If a sea, force to ground level **/

  if (cell->level < 0)
    cell->level = 0;

  /** If it's not already a town **/

  if (cell->growth < TOWN_MIN)
  {
    cell->growth =	growth;
    cell->old_growth =	growth;
  }

  cell->side =		side;
  cell->value[side] =	value;

  /** Make sure there are no angle/growth mismatches **/

  if (growth > TOWN_MIN)
    cell->angle =	ANGLE_FULL;
  else
    cell->angle =	0;
}
