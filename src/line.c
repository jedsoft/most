/*
 This file is part of MOST.

 Copyright (c) 1991, 1999, 2002, 2005 John E. Davis

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

int Most_Tab_Width = 8;

int Most_Selective_Display = 0;
/* take 16 binary characters and put them in displayable form */
static void ascii_format_line (unsigned char *beg, unsigned char *end,
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
	if (((ch >= ' ') && (ch < 0x7F))
	    || (ch >= SLsmg_Display_Eight_Bit))
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

   sprintf (buf, "0x%08X: ", Most_C_Offset);
   ascii_format_line (beg, end, buf + 12);
   SLsmg_write_string (buf);
   SLsmg_erase_eol ();
}

static int most_analyse_line(unsigned char *begg, unsigned char *endd, 
			     char *out, char *attributes)
{
   unsigned char *beg, *end;
   unsigned int min_col, max_col;
   unsigned int i, i_max;

   beg = begg;
   end = endd;
   i = i_max = 0;
   min_col = Most_Column - 1;
   max_col = min_col + SLtt_Screen_Cols;

   while (beg < end)
     {
	char attr = ' ';
	unsigned char ch;

	if ('\n' == (ch = *beg++))
	  break;
	
	if ((ch == '\r') && (Most_V_Opt == 0))
	  {
	     if (i > i_max) i_max = i;
	     i = 0;
	     continue;
	  }
	
	if ((ch == '\b') && (Most_V_Opt == 0))
	  {
	     if (i > i_max) i_max = i;
	     if (i > 0)
	       i--;
	     continue;
	  }
	
	if (i < i_max)		       /* overstrike */
	  {
	     attr = 'b';
	     if ((i >= min_col) && (i < max_col))
	       {
		  if (out[i-min_col] == '_')
		    attr = 'u';
		  else if (ch == '_')
		    {
		       attr = 'u';
		       ch = out[i - min_col];
		    }
	       }
	     if (ch == ' ')
	       {
		  i++;
		  continue;
	       }
	     /* drop */
	  }
	
	if ((ch >= ' ') && (ch < 0x7F))
	  {
	     if ((i >= min_col) && (i < max_col))
	       {
		  out[i-min_col] = ch;
		  attributes[i-min_col] = attr;
	       }
	     i++;
	     continue;
	  }
	
	if (ch >= SLsmg_Display_Eight_Bit)
	  {
	     if ((i >= min_col) && (i < max_col))
	       {
		  out[i-min_col] = ch;
		  attributes[i-min_col] = attr;
	       }
	     i++;
	     continue;
	  }

	if ((ch == '\t') && (Most_T_Opt == 0) && (Most_Tab_Width))
	  {

	     int nspaces = Most_Tab_Width * (i/Most_Tab_Width + 1) - i;
	     while (nspaces > 0)
	       {
		  if ((i >= min_col) && (i < max_col))
		    {
		       out[i-min_col] = ' ';
		       attributes[i-min_col] = attr;
		    }
		  i++;
		  nspaces--;
	       }
	     continue;
	  }

	if (ch & 0x80)
	  {
	     if ((i >= min_col) && (i < max_col))
	       {
		  out[i-min_col] = '~';
		  attributes[i-min_col] = attr;
	       }
	     i++;
	     ch &= 0x7F;
	     /* drop */
	  }
	
	if ((i >= min_col) && (i < max_col))
	  {
	     out[i-min_col] = '^';
	     attributes[i-min_col] = attr;
	  }
	i++;
	
	if (ch == 0x7F) ch = '?';
	else ch += '@';
	
	if ((i >= min_col) && (i < max_col))
	  {
	     out[i-min_col] = ch;
	     attributes[i-min_col] = attr;
	  }
	i++;
     }

   if (i < i_max) 
     i = i_max;

   /* Now add "..." if selective display.  To do that, the next line needs to 
    * be dealt with to determine whether or not it will be hidden.
    */
   if (Most_Selective_Display 
       && (Most_W_Opt == 0)
       && (beg < Most_Eob)
       && ((i >= min_col) && (i < max_col)))
     {
	if (*beg == '\n') beg++;

	while ((beg < Most_Eob) 
	       && ((*beg == ' ') || (*beg == '\t') || (*beg == '\r')))
	  beg++;
	
	if ((beg >= Most_Eob) || (*beg == '\n') 
	    || (most_apparant_distance(beg) >= Most_Selective_Display))
	  {
	     i_max = i + 3;
	     while (i < i_max)
	       {
		  if (i < max_col)
		    {
		       out[i] = '.';
		       attributes[i] = ' ';
		    }
		  i++;
	       }
	  }
     }
   
   i_max = i;

   if (i < min_col)
     i = min_col;
   else if (i >= max_col)
     i = max_col;

   i -= min_col;

   out[i] = 0;
   attributes[i] = 0;
   return i_max;
}

static void output_with_attr (unsigned char *out, unsigned char *attr)
{
   unsigned char at, ch, lat;
   unsigned char *p = out;

   if (Most_V_Opt) 
     {
	SLsmg_write_string ((char *) out);
	return;
     }

   lat = ' ';
   while ((ch = *p) != 0)
     {
	if (lat != *attr)
	  {
	     if (p != out)
	       {
		  SLsmg_write_nchars ((char *) out, (unsigned int) (p - out));
		  out = p;
	       }

	     at = *attr;
	     if (at == 'u')
	       {
		  most_tt_underline_video ();
	       }
	     else if (at == 'b')
	       {
		  most_tt_bold_video ();
	       }
	     else most_tt_normal_video ();
	     lat = at;
	  }
	p++;
	attr++;
     }

   SLsmg_write_nchars ((char *) out, (unsigned int) (p - out));
   if (lat != ' ') most_tt_normal_video ();
}

#if 0
static void most_output(unsigned char *line, unsigned int len, unsigned char *attr, char unsigned d_char)
{
   if (len > (unsigned int) SLtt_Screen_Cols + (Most_Column - 1))
     line[SLtt_Screen_Cols-1] = d_char;
   output_with_attr (line, attr);
}
#endif
void most_display_line (void)
{
   unsigned char *beg, *end;
   unsigned int len;
   unsigned char dollar;
   static unsigned char *line;
   static unsigned char *attr;
   static unsigned int line_len;

   if (Most_B_Opt)
     {
	output_binary_formatted_line ();
	return;
     }
#if SLANG_VERSION < 20000
# define SLUTF8_MAX_MBLEN	1
#endif
   if (line_len < (unsigned int)(SLtt_Screen_Cols + 1) * SLUTF8_MAX_MBLEN)
     {
	SLfree ((char *) line);
	SLfree ((char *) attr);
	
	line_len = (SLtt_Screen_Cols + 1) * SLUTF8_MAX_MBLEN;
	
	if ((NULL == (line = (unsigned char *) SLmalloc (line_len)))
	    || (NULL == (attr = (unsigned char *) SLmalloc (line_len))))
	  most_exit_error ("Out of memory");
     }

   (void) most_extract_line (&beg, &end);

   len = most_analyse_line(beg, end, (char *) line, (char *) attr);

   dollar = 0;
   if (Most_W_Opt)
     {
	if ((end < Most_Eob)
	    && (*end != '\n'))
	  dollar = '\\';
     }
   else if (len > (unsigned int) SLtt_Screen_Cols + (Most_Column - 1))
     dollar = '$';
   
   if (dollar)
     {
	line[SLtt_Screen_Cols-1] = dollar;
	attr[SLtt_Screen_Cols-1] = ' ';
	line[SLtt_Screen_Cols] = 0;
	attr[SLtt_Screen_Cols] = 0;
     }
   
   output_with_attr (line, attr);
   SLsmg_erase_eol ();
}


/* given a position in a line, return apparant distance from bol
   expanding tabs, etc... up to pos */
int most_apparant_distance (unsigned char *pos)
{
   int i;
   unsigned char *save_pos, ch;
   unsigned int save_offset;

   save_offset = Most_C_Offset;
   save_pos = pos;
   Most_C_Offset = (unsigned int) (pos - Most_Beg);
   pos = most_beg_of_line();
   Most_C_Offset = save_offset;

   i = 0;
   while (pos < save_pos)
     {
	ch = *pos++;
	if (((ch >= ' ') && (ch < 0x7F))
	    || (ch >= SLsmg_Display_Eight_Bit))
	  {
	     i++;
	     continue;
	  }

	if (!Most_V_Opt && (ch == '\b'))
	  {
	     if (i > 0) i--;
	  }
	else if (!Most_V_Opt && (ch == '\015')) /* ^M */
	  {
	     if (i != 1) i = 0;
	  }
	else if (!Most_T_Opt && (ch == '\t'))
	  {
	     i = Most_Tab_Width * (i/Most_Tab_Width + 1);  /* Most_Tab_Width column tabs */
	  }
	 /* else Control char */
	else
	  {
	     if (ch & 0x80) i += 3;
	     else i += 2;
	  }
     }
   return i;
}
