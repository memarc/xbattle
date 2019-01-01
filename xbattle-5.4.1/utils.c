#include <stdio.h>

#include "constant.h"
  
/**** x include files ****/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>


#include "extern.h"


/******************************************************************************
  int
  match_color (red, green, blue)

  Given the RGB components (<red>, <green>, <blue>) search through the custom
  colors for the color which most closely matches.  Return the inverse index.
  This routine used for finding out what inverse should be used for X colors.
******************************************************************************/

int
match_color (red, green, blue)
  int red, green, blue;
{
  int hue,
      dist,
      minimum, min_index;

  minimum = 1000000; 

  red = red>>8;
  green = green>>8;
  blue = blue>>8;

  for (hue=1; hue<Config->hue_count; hue++)
  {
    dist = (Config->palette[hue][0] - red)*(Config->palette[hue][0] - red) + 
           (Config->palette[hue][1] - green)*(Config->palette[hue][1] - green) + 
           (Config->palette[hue][2] - blue)*(Config->palette[hue][2] - blue);

    if (dist < minimum)
    {
      min_index = hue;
      minimum = dist;
    }
  }

  return (Config->hue_to_inverse[min_index]);
}



/******************************************************************************
  int
  match_color_name (color_name, default_hue)

  Match <color_name> with a palette entry and return the index, or just
  return <default_hue>.
******************************************************************************/

match_color_name (color_name, default_hue)
  char *color_name;
{
  int hue;

  for (hue=0; hue<Config->hue_count; hue++)
    if (!strcmp (color_name, Config->hue_name[hue]))
      return (hue);

  return (default_hue);
}



/******************************************************************************
  matchstr (line, word)

  Given a baseline string <word>, return TRUE if <line> is a subset of it.
  Used solely to match colors like "red_blue" to custom colors like "red"
  without getting confused by the "_blue" part.  Probably should just use
  strncmp (line, word, strlen(word)).
******************************************************************************/

matchstr (line, word)
  char line[],
       word[];
{
  int i;

  for (i=0; i<strlen(word); i++)
    if (line[i] == '\0' || line[i] != word[i])
      return (FALSE);
  return (TRUE);
}



/******************************************************************************
  int
  get_random (base)

  Use rand() to return a a number between 0 and <base>-1.
******************************************************************************/

int
get_random (base)
  int base;
{
  return (rand() % base);
}
