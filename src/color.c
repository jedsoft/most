/*
 This file is part of MOST.

 Copyright (c) 2021 John E. Davis <jed@jedsoft.org>

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 of the License, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc., 675
 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "config.h"

#include <stdio.h>
#include <string.h>

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#include <ctype.h>

#define ENABLE_SLFUTURE_CONST
#include <slang.h>
#include "jdmacros.h"
#include "most.h"
#include "keyparse.h"
#include "display.h"
#include "color.h"

#define DEFAULT_COLOR_HANDLE 16

/* These are used for FG ESC[30m - ESC[37m, and ESC[90 - ESC[97, and BG=FG+10 */
static int Xterm256_Map[256] =
{
   0x000000, 0xCD0000, 0x00CD00, 0xCDCD00, 0x0000EE, 0xCD00CD, 0x00CDCD, 0xE5E5E5,
   0x7F7F7F, 0xFF0000, 0x00FF00, 0xFFFF00, 0x5C5CFF, 0xFF00FF, 0x00FFFF, 0xFFFFFF,
   0x000000, 0x00005F, 0x000087, 0x0000AF, 0x0000D7, 0x0000FF, 0x005F00, 0x005F5F,
   0x005F87, 0x005FAF, 0x005FD7, 0x005FFF, 0x008700, 0x00875F, 0x008787, 0x0087AF,
   0x0087D7, 0x0087FF, 0x00AF00, 0x00AF5F, 0x00AF87, 0x00AFAF, 0x00AFD7, 0x00AFFF,
   0x00D700, 0x00D75F, 0x00D787, 0x00D7AF, 0x00D7D7, 0x00D7FF, 0x00FF00, 0x00FF5F,
   0x00FF87, 0x00FFAF, 0x00FFD7, 0x00FFFF, 0x5F0000, 0x5F005F, 0x5F0087, 0x5F00AF,
   0x5F00D7, 0x5F00FF, 0x5F5F00, 0x5F5F5F, 0x5F5F87, 0x5F5FAF, 0x5F5FD7, 0x5F5FFF,
   0x5F8700, 0x5F875F, 0x5F8787, 0x5F87AF, 0x5F87D7, 0x5F87FF, 0x5FAF00, 0x5FAF5F,
   0x5FAF87, 0x5FAFAF, 0x5FAFD7, 0x5FAFFF, 0x5FD700, 0x5FD75F, 0x5FD787, 0x5FD7AF,
   0x5FD7D7, 0x5FD7FF, 0x5FFF00, 0x5FFF5F, 0x5FFF87, 0x5FFFAF, 0x5FFFD7, 0x5FFFFF,
   0x870000, 0x87005F, 0x870087, 0x8700AF, 0x8700D7, 0x8700FF, 0x875F00, 0x875F5F,
   0x875F87, 0x875FAF, 0x875FD7, 0x875FFF, 0x878700, 0x87875F, 0x878787, 0x8787AF,
   0x8787D7, 0x8787FF, 0x87AF00, 0x87AF5F, 0x87AF87, 0x87AFAF, 0x87AFD7, 0x87AFFF,
   0x87D700, 0x87D75F, 0x87D787, 0x87D7AF, 0x87D7D7, 0x87D7FF, 0x87FF00, 0x87FF5F,
   0x87FF87, 0x87FFAF, 0x87FFD7, 0x87FFFF, 0xAF0000, 0xAF005F, 0xAF0087, 0xAF00AF,
   0xAF00D7, 0xAF00FF, 0xAF5F00, 0xAF5F5F, 0xAF5F87, 0xAF5FAF, 0xAF5FD7, 0xAF5FFF,
   0xAF8700, 0xAF875F, 0xAF8787, 0xAF87AF, 0xAF87D7, 0xAF87FF, 0xAFAF00, 0xAFAF5F,
   0xAFAF87, 0xAFAFAF, 0xAFAFD7, 0xAFAFFF, 0xAFD700, 0xAFD75F, 0xAFD787, 0xAFD7AF,
   0xAFD7D7, 0xAFD7FF, 0xAFFF00, 0xAFFF5F, 0xAFFF87, 0xAFFFAF, 0xAFFFD7, 0xAFFFFF,
   0xD70000, 0xD7005F, 0xD70087, 0xD700AF, 0xD700D7, 0xD700FF, 0xD75F00, 0xD75F5F,
   0xD75F87, 0xD75FAF, 0xD75FD7, 0xD75FFF, 0xD78700, 0xD7875F, 0xD78787, 0xD787AF,
   0xD787D7, 0xD787FF, 0xD7AF00, 0xD7AF5F, 0xD7AF87, 0xD7AFAF, 0xD7AFD7, 0xD7AFFF,
   0xD7D700, 0xD7D75F, 0xD7D787, 0xD7D7AF, 0xD7D7D7, 0xD7D7FF, 0xD7FF00, 0xD7FF5F,
   0xD7FF87, 0xD7FFAF, 0xD7FFD7, 0xD7FFFF, 0xFF0000, 0xFF005F, 0xFF0087, 0xFF00AF,
   0xFF00D7, 0xFF00FF, 0xFF5F00, 0xFF5F5F, 0xFF5F87, 0xFF5FAF, 0xFF5FD7, 0xFF5FFF,
   0xFF8700, 0xFF875F, 0xFF8787, 0xFF87AF, 0xFF87D7, 0xFF87FF, 0xFFAF00, 0xFFAF5F,
   0xFFAF87, 0xFFAFAF, 0xFFAFD7, 0xFFAFFF, 0xFFD700, 0xFFD75F, 0xFFD787, 0xFFD7AF,
   0xFFD7D7, 0xFFD7FF, 0xFFFF00, 0xFFFF5F, 0xFFFF87, 0xFFFFAF, 0xFFFFD7, 0xFFFFFF,
   0x080808, 0x121212, 0x1C1C1C, 0x262626, 0x303030, 0x3A3A3A, 0x444444, 0x4E4E4E,
   0x585858, 0x626262, 0x6C6C6C, 0x767676, 0x808080, 0x8A8A8A, 0x949494, 0x9E9E9E,
   0xA8A8A8, 0xB2B2B2, 0xBCBCBC, 0xC6C6C6, 0xD0D0D0, 0xDADADA, 0xE4E4E4, 0xEEEEEE
};


/* 256 color support:
 *
 *   0-7   (standard colors)
 *   8-15  (high intensity colors)
 *  16-231 (6x6x6 cube -- 216 colors)
 * 232-255 (grayscale)
 *
 * It is not possible to represent all 256x256 fg+bg combinations in a fixed
 * palette size of 32K, which slang-2 provides.  However, 128x128 falls in the
 * supported range.
 *
 * Mapping: x-->y
 *  if (16 <= x <= 231) y = 16 + 2*((x-16)/2)
 *  y = x, othersize
 *
 * This implies 16 + 24 + 216/2 = 148 distinct values (21904 combinations)
 */
/* Returns -1 if no integer present or value >= 256, otherwise 0  */
static int parse_int (unsigned char **begp, unsigned char *end, int *ip)
{
   unsigned char *beg;
   int xx;

   beg = *begp;
   if ((beg >= end) || (0 == isdigit (*beg)))
     return -1;

   xx = *beg++ - '0';
   while ((beg < end) && isdigit (*beg))
     {
	xx = xx*10 + (*beg - '0');
	if (xx >= 256) return -1;
	beg++;
     }
   *begp = beg;
   *ip = xx;
   return 0;
}

/* This function maps the color number X for ESC[38;5;X to RGB */
static int map_256color_to_rgb (int c, int *rgbp)
{
   if ((c < 0) || (c > 255))
     {
	*rgbp = 0;
	return -1;
     }
   *rgbp = Xterm256_Map[c];
   return 0;
}

static int get_color_handle (int fg, int bg, int at, int *colorp);

static int map_fgbgat_to_color (int fg, int bg, int at, int *colorp)
{
   int xx;

   if (fg < 0)
     {
	if (bg < 0)
	  {
	     fg = -fg;
	     bg = -bg;
	     xx = fg + 17*(bg + 17*at);
	     if (xx != 0) xx += MOST_EMBEDDED_COLOR_OFFSET;
	     *colorp = xx;
	     return 0;
	  }

	if (fg != -DEFAULT_COLOR_HANDLE)
	  fg = Xterm256_Map[(-fg) & 0xFF];

	return get_color_handle (fg, bg, at, colorp);
     }

   if (bg < 0)
     {
	if (bg != -DEFAULT_COLOR_HANDLE)
	  bg = Xterm256_Map[(-bg) & 0xFF];
     }

   return get_color_handle (fg, bg, at, colorp);
}

/* *begp follows ^[[38 or ^[[48.  We are looking for: ";5;X" or ";2;r;g;b" */
static int parse_setcolor_256 (unsigned char **begp, unsigned char *end, int *rgbp)
{
   unsigned char *beg = *begp;
   int xx;

   if ((beg >= end) || (*beg != ';')) return -1;
   beg++;

   if (-1 == parse_int (&beg, end, &xx))
     return -1;

   if ((beg >= end) || (*beg != ';'))
     return -1;
   beg++;

   if (xx == 5)
     {
	if (-1 == parse_int (&beg, end, &xx))
	  xx = 0;

	(void) map_256color_to_rgb (xx, rgbp);
	*begp = beg;
	return 0;
     }
   if (xx == 2)
     {
	int r, g, b;
	int rgb;

	if (-1 == parse_int (&beg, end, &r))
	  r = 0;
	if ((beg >= end) || (*beg != ';') || (r > 256))
	  return -1;
	beg++;

	if (-1 == parse_int (&beg, end, &g))
	  g = 0;
	if ((beg >= end) || (*beg != ';') || (g > 256))
	  return -1;
	beg++;

	if (-1 == parse_int (&beg, end, &b))
	  b = 0;
	if (b >= 256)  return -1;

	rgb = ((r << 16) | (g << 8) | b);
	*rgbp = rgb;
	*begp = beg;
	return 0;
     }

   return -1;
}


/* Here *begp points to the char after \e[.
 * The general escape sequence parsed here is assumed to look like:
 *   \e[ XX ; ... m
 * If 30 <= XX <= 37, then it specifies the foreground color
 * 38: 
 * 39: use default foreground
 * If 40 <= XX <= 47, then a background color is specified
 * 48: reserved
 * 49: use default background
 * If  0 <= XX <= 8, then an attribute (e.g, 8) is specified.
 * These numbers will be encoded as:
 *  offset + (FG-30) + 17*(BG-40 + 17*attribute)
 */
static int parse_color_escape_internal (unsigned char **begp, unsigned char *end,
					int *fgp, int *bgp, int *atp)
{
   unsigned char *beg = *begp;
   int fg = *fgp, bg = *bgp, at = *atp;
   int xx;

   /* Here we have *begp past "^[[".  Look for semi-colon separated sequence of 0 or more integers
    * followed by 'm', e.g., "m", "34", "48;5;212m, ...
    *
    * In the following, a negative value of fb or bg implies a non-rgb value
    */
   while (1)
     {
	if (-1 == parse_int (&beg, end, &xx))
	  xx = 0;

	if (xx <= 8)
	  at = xx;/* attributes */
	else if ((xx >= 30) && (xx <= 37))
	  fg = -(xx-30);		       /* foreground color : 0-7 */
	else if (xx == 39)
	  fg = -DEFAULT_COLOR_HANDLE;   /* default */
	else if ((xx >= 40) && (xx <= 47))
	  bg = -(xx-40);		       /* background color : 0-7 */
	else if (xx == 49)
	  bg = -DEFAULT_COLOR_HANDLE;   /* default */
	else if (xx == 38)
	  {
	     if (-1 == parse_setcolor_256 (&beg, end, &fg))
	       return -1;
	  }
	else if (xx == 48)
	  {
	     if (-1 == parse_setcolor_256 (&beg, end, &bg))
	       return -1;
	  }
	else if ((xx >= 20) && (xx <= 29))
	  at = 0;
	else if ((xx >= 90) && (xx <= 97))
	  fg = -(8 + xx-90);		       /* bright foreground color : 8-15 */
	else if ((xx >= 100) && (xx <= 107))
	  bg = -(8 + xx-100);		       /* bright background color : 8-15 */

	if ((beg < end) && (*beg == ';'))
	  {
	     beg++;
	     continue;
	  }

	if ((beg < end) && ((*beg == 'm') || (*beg == ']')))
	  {
	     *begp = beg + 1;
	     *fgp = fg;
	     *bgp = bg;
	     *atp = at;
	     return 0;
	  }
	return -1;
     }
}


/* Parse a color escape and update *fgp and *bgp accordingly.
 * Note that -1 for *fgp or *bgp indicates the default value
 */
int most_parse_color_escape (unsigned char **begp, unsigned char *end, int *colorp)
{
   unsigned char *start_beg, *beg = *begp;
   int fg = -DEFAULT_COLOR_HANDLE, bg = -DEFAULT_COLOR_HANDLE, at = 0;
   int numloops;

   start_beg = beg = *begp;

   if ((beg + 1 >= end) || (*beg != '['))
     return -1;

   beg++; /* skip [ */
   if (*beg == 'K')
     {
	if (colorp != NULL) *colorp = -1;
	*begp = beg + 1;
	return 0;
     }

   numloops = 0;
   while (1)
     {
	if (*beg == 'm')
	  {
	     if (colorp != NULL) *colorp = 0;
	     *begp = beg + 1;
	     return 0;
	  }

	if (-1 == parse_color_escape_internal (&beg, end, &fg, &bg, &at))
	  {
	     if (numloops == 0)
	       return -1;

	     break;
	  }
	numloops++;

	/* Reset start_beg and check for next color escape seq.  This allows
	 * a setfg followed by a setbg
	 */
	start_beg = beg;
	if ((beg + 2 >= end) || (beg[0] != 033) || (beg[1] != '['))
	  break;
	beg += 2;
     }

   if (colorp != NULL)
     (void) map_fgbgat_to_color (fg, bg, at, colorp);

   *begp = start_beg;
   return 0;
}

typedef struct
{
   const char *name;
   int rgb;
   double h, s, v;
}
Ansi_Color_Map_Type;

static Ansi_Color_Map_Type Ansi_Color_Map[16+1] =
{
   {"black",         0x000000, 0,0,0},
   {"red",           0xCD0000, 0,0,0},
   {"green",         0x00CD00, 0,0,0},
   {"brown",         0xCDCD00, 0,0,0},
   {"blue",          0x0000EE, 0,0,0},
   {"magenta",       0xCD00CD, 0,0,0},
   {"cyan",          0x00CDCD, 0,0,0},
   {"lightgray",     0xE5E5E5, 0,0,0},
   {"gray",          0x7F7F7F, 0,0,0},
   {"brightred",     0xFF0000, 0,0,0},
   {"brightgreen",   0x00FF00, 0,0,0},
   {"yellow",        0xFFFF00, 0,0,0},
   {"brightblue",    0x5C5CFF, 0,0,0},
   {"brightmagenta", 0xFF00FF, 0,0,0},
   {"brightcyan",    0x00FFFF, 0,0,0},
   {"white",         0xFFFFFF, 0,0,0},
   {"default",       0, 0,0,0} /* at offset 16 */
};


static void compute_hsv (int rgb, double *hp, double *sp, double *vp)
{
   double r = (rgb>>16)&0xFF;
   double g = (rgb>>8)&0xFF;
   double b = (rgb&0xFF);
   double h, s, v, u, d;

   r /= 255.0; g /= 255.0; b /= 255.0;
   u = v = r;
   if (g > v) v = g;
   if (g < u) u = g;
   if (b > v) v = b;
   if (b < u) u = b;

   d = v-u;
   if (v == u) h = 0.0;
   else if (v == r) h = (g-b)/d;
   else if (v == g) h = 2 + (b-r)/d;
   else h = 4 + (r-g)/d;

   h = (int)(60*h + 360) % 360;
   s = (v == 0) ? 0 : d/v;

   *hp = h;
   *sp = s*100;
   *vp = v*100;
}

static int map_rgb_to_ansi16 (int rgb)
{
   /* This is a naive algorithm that uses the euclidean distance */
   double h, s, v;
   int i, imin, dmin;

   if (rgb < 0)
     return DEFAULT_COLOR_HANDLE;

   compute_hsv (rgb, &h, &s, &v);
   imin = 0;
   dmin = 3*255*255;
   for (i = 0; i < 16; i++)
     {
	double dh, ds, dv, d;

	dh = h - Ansi_Color_Map[i].h;
	ds = s - Ansi_Color_Map[i].s;
	dv = v - Ansi_Color_Map[i].v;

	d = 4*dh*dh + 3*ds*ds + 1*dv*dv;

	if ((i == 0) || (d < dmin))
	  {
	     imin = i;
	     dmin = d;
	  }
     }
   return imin;
}

static int Max_Color;
typedef struct Color_Obj_Type_
{
   int color, fg, bg, at;
   struct Color_Obj_Type_ *next;
}
Color_Obj_Type;

#define COLOR_OBJECTS_SIZE (9973)
static Color_Obj_Type *Color_Objects[COLOR_OBJECTS_SIZE];
static int Has_True_Color = 0;

static int get_color_handle (int fg, int bg, int at, int *colorp)
{
   Color_Obj_Type *cot;
   unsigned long hash;
   char bgname[16], fgname[16];

   if (Has_True_Color == 0)
     {
	fg = map_rgb_to_ansi16 (fg);
	bg = map_rgb_to_ansi16 (bg);
	if (fg == bg)
	  {
	     fg = 15;
	     if (bg == 15) fg = 0;
	  }
	/* if ((fg == 0) && (bg == 8)) fg = 15; */
	/* if ((fg == 8) && (bg == 0)) fg = 15; */
	if ((fg == 0) && (bg == 8)) fg = 7;
	*colorp = MOST_EMBEDDED_COLOR_OFFSET + (fg + 17*(bg + 17*at));
	return 0;
     }

   hash = at;
   if ((fg == -1) || (Has_True_Color == 0))
     {
	strcpy (fgname, "default");
	fg = -1;
     }
   else
     {
	(void) snprintf (fgname, sizeof(fgname), "#%06X", fg&0xFFFFFF);
	hash = (fg << 8) ^ hash;
     }

   if ((bg == -1) || (Has_True_Color == 0))
     {
	strcpy (bgname, "default");
	bg = -1;
     }
   else
     {
	(void) snprintf (bgname, sizeof(bgname), "#%06X", bg&0xFFFFFF);
	hash = (bg << 3) ^ hash;
     }

   hash = hash % COLOR_OBJECTS_SIZE;
   cot = Color_Objects[hash];
   while (cot != NULL)
     {
	if ((cot->fg == fg) && (cot->bg == bg) && (cot->at == at))
	  {
	     *colorp = cot->color;
	     return 0;
	  }
	cot = cot->next;
     }

   cot = (Color_Obj_Type *) malloc (sizeof (Color_Obj_Type));
   if (cot == NULL)
     {
	*colorp = -1;		       /* use default */
	return -1;
     }

   cot->fg = fg;
   cot->bg = bg;
   cot->at = at;
   cot->color = Max_Color++;
   cot->next = Color_Objects[hash];
   Color_Objects[hash] = cot;

   if (0 == SLtt_set_color (cot->color, NULL, fgname, bgname))
     Max_Color++;

   *colorp = cot->color;
   return 0;
}


static SLtt_Char_Type Color_Attributes[10] =
{
   0,
   SLTT_BOLD_MASK,
   0,
   SLTT_ITALIC_MASK,
   SLTT_ULINE_MASK,
   SLTT_BLINK_MASK,
   SLTT_BLINK_MASK,
   SLTT_REV_MASK,
   0,
   0
};

int most_setup_embedded_colors (void)
{
   int i, c, at, fg, bg;
   Ansi_Color_Map_Type *cm, *cmmax;

   cm = Ansi_Color_Map;
   cmmax = cm + 16;
   while (cm < cmmax)
     {
	compute_hsv (cm->rgb, &cm->h, &cm->s, &cm->v);
	cm++;
     }

   /* ESC [ 30-37m, 40-47, 90-97, and 100-107 forms with attributes */
   for (at = 0; at < 10; at++)
     {
	SLtt_Char_Type attr = Color_Attributes[at];

	for (fg = 0; fg < 17; fg++)
	  {
	     for (bg = 0; bg < 17; bg++)
	       {
		  c = fg + 17*(bg + 17*at);   /* see most_parse_color_escape */
		  if (c == 0)
		    continue;
		  c += MOST_EMBEDDED_COLOR_OFFSET;
		  SLtt_set_color (c, NULL, Ansi_Color_Map[fg].name, Ansi_Color_Map[bg].name);
		  SLtt_add_color_attribute (c, attr);
	       }
	  }
     }

   /* Now add some of ESC[38;5;Xm and ESC[38;5;Xm but not all fg/bg combinations */
   /* Start at unused color handle */
   Max_Color = 17*17*10 + MOST_EMBEDDED_COLOR_OFFSET;

   /* First, test for true color */
   if (-1 == SLtt_set_color (Max_Color, NULL, "#FFFFFF", "#000000"))
     {
	Has_True_Color = 0;
	return 0;
     }

   Has_True_Color = 1;
   for (i = 0; i < 256; i++)
     {
	int rgb = Xterm256_Map[i];
	int color;
	(void) get_color_handle (-1, rgb, 0, &color);
	(void) get_color_handle (rgb, -1, 0, &color);
     }
   return 0;
}
