/*
 This file is part of MOST.

 Copyright (c) 1991, 1999, 2002, 2005-2021, 2022 John E. Davis

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

#include <ctype.h>
#include <slang.h>
#include "jdmacros.h"

#include "most.h"
#include "line.h"
#include "window.h"
#include "display.h"
#include "color.h"

int Most_Tab_Width = 8;

int Most_Selective_Display = 0;
int Most_Show_Wrap_Marker = 1;

#define IS_BYTE_PRINTABLE(b) \
   ((((b) >= ' ') && ((b) < 0x7F)) \
       || ((Most_UTF8_Mode == 0) && ((b) >= SLsmg_Display_Eight_Bit)))

/* take 16 binary characters and put them in displayable form */
static void binary_format_line (unsigned char *beg, unsigned char *end,
			       char *buf)
{
   unsigned char *b;
   char *s, *s1;
   unsigned char ch;
   int count;

   count = 0;
   b = beg;
   s = buf;

   while (b < end)
     {
	if (count == 4)
	  {
	     *s++ = ' ';
	     count = 0;
	  }
	count++;

	ch = *b++;

	if ((Most_V_Opt == 0)
	    || (ch & 0x80))
	  {
	     sprintf (s, "%02X", ch);
	     s += 2;
	     continue;
	  }

	if ((ch >= ' ') && (ch < 0x7F))
	  {
	     *s++ = ' ';
	     *s++ = (char) ch;
	     continue;
	  }

	*s++ = '^';
	if (ch < ' ') ch += '@';
	else ch = '?';
	*s++ = ch;
     }

   s1 = buf + (9 * 4) + 4;
   while (s < s1)
     *s++ = ' ';

   b = beg;
   while (b < end)
     {
        ch = *b++;
	if (IS_BYTE_PRINTABLE(ch))
	  {
	     *s++ = ch;
	     continue;
	  }
	*s++ = '.';
     }
   *s = 0;
}

static void output_binary_formatted_line (void)
{
   unsigned char *beg, *end;
   char buf[256];

   beg = Most_Beg + Most_C_Offset;
   end = beg + 16;

   if (end > Most_Eob) end = Most_Eob;

   sprintf (buf, "0x%08lX: ", (unsigned long) Most_C_Offset);
   binary_format_line (beg, end, buf + 12);
   SLsmg_write_string (buf);
   SLsmg_erase_eol ();
}

typedef struct
{
   unsigned char *bytes;
   unsigned char byte;		       /* used if bytes is NULL */
   unsigned int len;
   int color;
}
Multibyte_Cell_Type;

static int most_analyse_line (unsigned char *begg, unsigned char *endd,
			      Multibyte_Cell_Type *cells, unsigned int num_cols,
			      int *start_colorp, int *start_atp)
{
   unsigned char *beg, *end;
   unsigned int min_col, max_col, prev_width;
   unsigned int col, max_col_reached;
   int start_attr, start_at;
   Multibyte_Cell_Type *cell, *max_cell;

   beg = begg;
   end = endd;
   col = max_col_reached = 0;
   cell = cells;
   max_cell = cell;
   min_col = Most_Column - 1;
   max_col = min_col + num_cols;

   start_attr = *start_colorp;
   start_at = *start_atp;
   prev_width = 1;
   while (beg < end)
     {
	int attr = start_attr;
	unsigned char ch;
	unsigned char *pch = beg++;
	char buf[16];

	if ('\n' == (ch = *pch))
	  break;

	if ((ch == '\r') && (Most_V_Opt == 0))
	  {
	     if (col > max_col_reached) max_col_reached = col;
	     col = 0;
	     prev_width = 1;
	     continue;
	  }

	if ((ch == '\b') && (Most_V_Opt == 0))
	  {
	     if (col > max_col_reached) max_col_reached = col;
	     if (col < prev_width)
	       col = 0;
	     else
	       col -= prev_width;
	     continue;
	  }

	if (col < max_col_reached)		       /* overstrike */
	  {
	     attr = MOST_BOLD_COLOR;
	     if ((col >= min_col) && (col < max_col))
	       {
		  cell = cells + (col-min_col);
		  if (cell->bytes[0] == '_')
		    attr = MOST_ULINE_COLOR;
		  else if (ch == '_')
		    {
		       cell->color = MOST_ULINE_COLOR;
		       col++;
		       continue;
		    }
	       }
	     /* drop */
	  }

	if (IS_BYTE_PRINTABLE(ch))
	  {
	     if ((col >= min_col) && (col < max_col))
	       {
		  cell = cells + (col-min_col);
		  cell->bytes = pch;
		  cell->len = 1;
		  cell->color = attr;
		  if (cell >= max_cell)
		    max_cell = cell + 1;
	       }
	     col++;
	     prev_width = 1;
	     continue;
	  }

	if ((ch == '\t') && (Most_T_Opt == 0) && (Most_Tab_Width))
	  {
	     int nspaces = Most_Tab_Width * (col/Most_Tab_Width + 1) - col;
	     prev_width = nspaces;
	     while (nspaces > 0)
	       {
		  if ((col >= min_col) && (col < max_col))
		    {
		       cell = cells + (col-min_col);
		       cell->bytes = &cell->byte;
		       cell->byte = ' ';
		       cell->color = attr;
		       cell->len = 1;
		       if (cell >= max_cell)
			 max_cell = cell + 1;
		    }
		  col++;
		  nspaces--;
	       }
	     continue;
	  }
#if 1
	if ((ch == 033) && (Most_V_Opt == 0))
	  {
	     int color, at;
	     if (0 == most_parse_color_escape (&beg, end, &color, &at))
	       {
		  if (color != -1)
		    {
		       start_attr = color;
		       start_at = at;
		    }
		  continue;
	       }
	     /* drop */
	  }
#endif

	if (ch & 0x80)
	  {
	     SLwchar_Type wch;
	     if ((Most_UTF8_Mode)
		 && (NULL != SLutf8_decode (pch, end, &wch, NULL)))
	       {
		  int width = SLwchar_wcwidth (wch);
		  beg = SLutf8_skip_chars (pch, end, 1, NULL, 0);

		  prev_width = width;
		  if (width == 0)
		    {
		       if (col <= min_col) continue;

		       col--;
		       if (col < max_col)
			 {
			    cell = cells + (col-min_col);
			    cell->len += beg-pch;
			 }
		       col++;
		       continue;
		    }

		  if ((col >= min_col) && (col < max_col))
		    {
		       cell = cells + (col-min_col);
		       cell->bytes = pch;
		       cell->color = attr;
		       cell->len = beg - pch;
		       if (cell >= max_cell)
			 max_cell = cell + 1;
		    }
		  col++;
		  if (width > 1)
		    {
		       if ((col >= min_col) && (col < max_col))
			 {
			    cell = cells + (col-min_col);
			    cell->bytes = pch;
			    cell->color = attr;
			    cell->len = 0;
			    if (cell >= max_cell)
			      max_cell = cell + 1;
			 }
		       col++;
		    }
		  continue;
	       }

	     /* Otherwise, this displays as <XX> and takes up 4 character cells */
	     sprintf (buf, "<%02X>", (unsigned int) ch);
	     prev_width = 4;
	     /* drop */
	  }
	else
	  {
	     /* Otherwise we have a Ctrl-char displayed as ^X */
	     if (ch == 0x7F) ch = '?';
	     else ch += '@';

	     sprintf (buf, "^%c", ch);
	     prev_width = 2;
	  }

	pch = (unsigned char *)buf;
	while (*pch)
	  {
	     if ((col >= min_col) && (col < max_col))
	       {
		  cell = cells + (col-min_col);
		  cell->bytes = &cell->byte;
		  cell->byte = *pch;
		  cell->color = attr;
		  cell->len = 1;
		  if (cell >= max_cell)
		    max_cell = cell + 1;
	       }
	     col++;
	     pch++;
	  }
     }

   if (col < max_col_reached)
     col = max_col_reached;
   else
     max_col_reached = col;

   /* Now add "..." if selective display.  To do that, the next line needs to
    * be dealt with to determine whether or not it will be hidden.
    */
   if (Most_Selective_Display
       && (Most_W_Opt == 0)
       && (beg < Most_Eob)
       && ((col >= min_col) && (col < max_col)))
     {
	if (*beg == '\n') beg++;

	while ((beg < Most_Eob)
	       && ((*beg == ' ') || (*beg == '\t') || (*beg == '\r')))
	  beg++;

	if ((beg >= Most_Eob) || (*beg == '\n')
	    || (most_apparant_distance(beg) >= Most_Selective_Display))
	  {
	     max_col_reached = col + 3;
	     while (col < max_col_reached)
	       {
		  if (col < max_col)
		    {
		       cell = cells + (col-min_col);
		       cell->bytes = &cell->byte;
		       cell->byte = '.';
		       cell->color = 0;
		       cell->len = 1;
		       if (cell >= max_cell)
			 max_cell = cell + 1;
		    }
		  col++;
	       }
	  }
     }
   *start_colorp = start_attr;
   *start_atp = start_at;
   return max_cell - cells;
}

static void display_cells (Multibyte_Cell_Type *cell, unsigned int n, char dollar)
{
   Multibyte_Cell_Type *cell_max;
   int last_color = -1;

   cell_max = cell + n;
   while (cell < cell_max)
     {
	if (last_color != cell->color)
	  {
	     last_color = cell->color;
	     SLsmg_set_color (last_color);
	  }
	SLsmg_write_chars (cell->bytes, cell->bytes + cell->len);
	cell++;
     }

   if (last_color != 0)
     SLsmg_set_color (0);

   SLsmg_erase_eol ();
   if (dollar)
     {
	SLsmg_gotorc (SLsmg_get_row (), SLtt_Screen_Cols-1);
	SLsmg_write_nchars (&dollar, 1);
     }
}

void most_display_line (int reset)
{
   unsigned char *beg, *end;
   unsigned char dollar;
   static Multibyte_Cell_Type *cells;
   static unsigned int num_cells;
   unsigned int screen_cols;
   unsigned int num_cells_set;
   static int last_color = 0;	       /* used for a line that wrapped */
   static int last_at = 0;

   if (Most_B_Opt)
     {
	output_binary_formatted_line ();
	return;
     }

   screen_cols = SLtt_Screen_Cols;
   if (num_cells != screen_cols + 1)
     {
	num_cells = screen_cols + 1;

	SLfree ((char *) cells);
	if (NULL == (cells = (Multibyte_Cell_Type *)SLcalloc (num_cells, sizeof (Multibyte_Cell_Type))))
	  most_exit_error ("Out of memory");
     }

   (void) most_extract_line (&beg, &end);

   if (reset || (Most_W_Opt == 0))
     {
	last_color = 0;
	last_at = 0;
     }

   num_cells_set = most_analyse_line (beg, end, cells, num_cells, &last_color, &last_at);

   dollar = 0;
   if (Most_W_Opt)
     {
	if (Most_Show_Wrap_Marker
	    && (end < Most_Eob)
	    && (*end != '\n'))
	  dollar = '\\';
     }
   else if (num_cells_set > screen_cols)
     dollar = '$';

   display_cells (cells, num_cells_set, dollar);
}

/* given a position in a line, return apparent distance from bol
   expanding tabs, etc... up to pos */
MOST_INT most_apparant_distance (unsigned char *pos)
{
   MOST_INT i, prev_width;
   unsigned char *save_pos, ch;
   unsigned int save_offset;

   save_offset = Most_C_Offset;
   save_pos = pos;
   Most_C_Offset = (MOST_UINT) (pos - Most_Beg);
   pos = most_beg_of_line();
   Most_C_Offset = save_offset;

   i = 0;
   prev_width = 1;
   while (pos < save_pos)
     {
	ch = *pos++;
	if (IS_BYTE_PRINTABLE(ch))
	  {
	     i++;
	     prev_width = 1;
	     continue;
	  }

	if ((ch == '\b') && (Most_V_Opt == 0))
	  {
	     i -= prev_width;
	     if (i < 0) i = 0;
	     continue;
	  }
	if ((ch == '\r') && (Most_V_Opt == 0))
	  {
	     if (i != 1) i = 0;
	     prev_width = 1;
	     continue;
	  }
	if ((ch == '\t') && (Most_T_Opt == 0))
	  {
	     prev_width = Most_Tab_Width * (i/Most_Tab_Width + 1) - i;  /* Most_Tab_Width column tabs */
	     i += prev_width;
	     continue;
	  }

	if ((ch == 033) && (Most_V_Opt == 0)
	    && (0 == most_parse_color_escape (&pos, save_pos, NULL, NULL)))
	  continue;

	if (ch & 0x80)
	  {
	     SLwchar_Type wch;
	     if ((Most_UTF8_Mode)
		 && (NULL != SLutf8_decode (pos-1, save_pos, &wch, NULL)))
	       {
		  prev_width = SLwchar_wcwidth (wch);
		  pos = SLutf8_skip_chars (pos-1, save_pos, 1, NULL, 1);
		  i += prev_width;
		  continue;
	       }
	     prev_width = 4;
	     i += prev_width;	       /* <XX> */
	     continue;
	  }

	prev_width = 2;
	i += prev_width;	       /* ^X */
     }
   return i;
}

/*
 * Returns a pointer to the num_cols'th character after the one
 * pointed at b. Invisible character runs are not counted toward this
 * limit, i.e. strings that represent attributes, such as "_\b" for
 * underlines.
 *
 * If multi_column is non-zero, characters spanning more than one
 * column will add their extra width to the column count.
 *
 * If there the end of the buffer is reached, as delimited by argument
 * e, then e is returned.
 */
unsigned char *most_forward_columns (unsigned char *b, unsigned char *e, MOST_UINT num_cols)
{
   MOST_UINT col = 0;
   MOST_UINT prev_width = 1;

   while (b < e)
     {
	unsigned char ch = *b++;

	if (col >=num_cols)
	  {
	     if ((ch == 033) && (Most_V_Opt == 0))
	       {
		  while ((ch == 033)
			 && (0 == most_parse_color_escape (&b, e, NULL, NULL))
			 && (b < e))
		    ch = *b++;
	       }
	     b--;
	     break;
	  }

	if (IS_BYTE_PRINTABLE(ch))
	  {
	     col++;
	     prev_width = 1;
	     continue;
	  }

	if (ch & 0x80)
	  {
	     SLwchar_Type wch;

	     if ((Most_UTF8_Mode)
		 && (NULL != SLutf8_decode (b-1, e, &wch, NULL)))
	       {
		  b = SLutf8_skip_chars (b-1, e, 1, NULL, 1);
		  prev_width = SLwchar_wcwidth (wch);
		  col += prev_width;
		  continue;
	       }
	     prev_width = 4;
	     col += prev_width;	       /* <XX> */
	     continue;
	  }

	if (ch == '\b')
	  {
	     if (Most_V_Opt == 0)
	       {
		  if (col < prev_width)
		    col = 0;
		  else
		    col -= prev_width;
	       }
	     else col += 2;	       /* ^H */
	     continue;
	  }

	if (ch == '\r')
	  {
	     if (Most_V_Opt == 0)
	       {
		  prev_width = 1;
		  col = 0;
	       }
	     else col += 2;	       /* ^M */
	     continue;
	  }

	if (ch == '\t')
	  {
	     if (Most_T_Opt == 0)
	       {
		  prev_width = Most_Tab_Width * (col/Most_Tab_Width + 1) - col;
		  col += prev_width;
	       }
	     else
	       col += 2;	       /* ^I */
	     continue;
	  }

	if ((ch == 033) && (Most_V_Opt == 0)
	    && (0 == most_parse_color_escape (&b, e, NULL, NULL)))
	  continue;

	/* Ctrl-char ^X */
	prev_width = 2;
	col += prev_width;
     }
   return b;
}
