/*
 This file is part of MOST.

 Copyright (c) 1991, 1999, 2002, 2005, 2006, 2007 John E. Davis

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

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include <string.h>

#include <slang.h>
#include "jdmacros.h"

#include "most.h"
#include "buffer.h"
#include "display.h"
#include "window.h"
#include "line.h"
#include "file.h"

int Most_W_Opt = 0;

unsigned char *Most_Beg;             /* beginning of current buffer */
unsigned char *Most_Eob;             /* end of current buffer */

Most_Buffer_Type *Most_Buf;

MOST_INT Most_Num_Lines;

MOST_UINT Most_C_Offset;
MOST_INT Most_C_Line;

/* p>p0 assumed */
#define BSKIP_CHAR(p,p0) \
   (((Most_UTF8_Mode == 0) || (0 == ((*(p-1)) & 0x80))) \
     ? ((p)--) \
     : ((p) = SLutf8_bskip_char ((p0), (p))))


static unsigned char *beg_of_line1(void)
{
   unsigned char *pos;
   unsigned char *cpos;

   cpos = pos = Most_Beg + Most_C_Offset;
   if (pos == Most_Beg) return pos;

   if (pos != Most_Eob)
     {
	if (*pos == '\n')
	  {
	     pos--;/* Skip back over the new-line. */
	     while ((pos > Most_Beg)
		    && (*pos != '\n'))
	       pos--;

	     if (*pos != '\n') return pos;
	     if (pos + 1 != cpos)
	       return pos + 1;
	  }
     }
   else BSKIP_CHAR (pos, Most_Beg);

   if (*pos != '\n')
     {
	while ((pos > Most_Beg)
	       && (*pos != '\n'))
	  pos--;
	if (*pos != '\n') return Most_Beg;
	return pos + 1;
     }

   /* from here on *pos == '\n' */

   if (Most_S_Opt == 0) return pos + 1;

   while ((pos > Most_Beg)
	  && (*pos == '\n')) pos--;
   pos += 2;
   if (pos > cpos) pos = cpos;
   return pos;
}

/* does not move point */
static unsigned char *end_of_line1(void)
{
   register unsigned char *pos, *pmax;
   int n, n2;

   pos = Most_Beg + Most_C_Offset;
   pmax = Most_Eob;
   if (pos >= pmax)  return pmax;

   /* find the first '\n'.  If we are wrapping lines, then do not go more
    * than 3 times the display width.
    */
   if (Most_W_Opt && SLtt_Screen_Cols)
     {
	pmax = pos + 3 * SLtt_Screen_Cols;
	if (pmax > Most_Eob)
	  pmax = Most_Eob;
     }

   if (*pos != '\n')
     {
	/* This block is UTF-8 safe, because it only scans the buffer
	 * for a new-line, and doesn't count characters. 
	 */
	n = pmax - pos;
	n2 = n % 8;
	pmax = pos + (n - 8);

	while (pos <= pmax)
	  {
	     if (*pos == '\n') return pos;
	     if (*(pos + 1) == '\n') return pos + 1;
	     if (*(pos + 2) == '\n') return pos + 2;
	     if (*(pos + 3) == '\n') return pos + 3;
	     if (*(pos + 4) == '\n') return pos + 4;
	     if (*(pos + 5) == '\n') return pos + 5;
	     if (*(pos + 6) == '\n') return pos + 6;
	     if (*(pos + 7) == '\n') return pos + 7;
	     pos += 8;
	  }
	pmax = pos + n2;
	while ((pos < pmax) && (*pos != '\n')) pos++;
	return(pos);
     }

   if (!Most_S_Opt) return (pos);
  /* file */
    /* if Most_Beg = "....abc\n\n\n\n\ndef..." then we are at some first \n.  We
       want to return the last '\n' unless we wre at the first '\n'. */

   /* Here we are on a \n and NOT at the end of the buffer */

   if ((pos > Most_Beg) && (*(pos - 1) != '\n')) return (pos);

   while ((pos < Most_Eob) && (*pos == '\n')) pos++;
   if (pos == Most_Eob) return pos;
   if (pos != Most_Beg) pos--;
   return pos;
   /* if (pos == Most_Eob) return (pos - 1);
    return pos; */
}

unsigned char *most_beg_of_line(void)
{
   unsigned char *b;
   unsigned int ncols;
   unsigned char *e;

   if (Most_W_Opt == 0) return beg_of_line1();

   b = beg_of_line1 ();
   e = end_of_line1 ();
   ncols = SLtt_Screen_Cols;
   if (Most_Show_Wrap_Marker)
     ncols--;
   while (1)
     {
	unsigned char *next_b = most_forward_columns (b, e, ncols);
	if ((next_b == e) || (next_b == b))
	  break;
	
	if (next_b >= Most_Beg + Most_C_Offset)
	  break;

	 b = next_b;
     }
   
   return b;
}

static unsigned char *end_of_line (unsigned char *b)
{
   unsigned char *e, *b1;
   int ncols;

   e = end_of_line1();
   if (Most_W_Opt == 0)
     return e;

   if (b == NULL) b = most_beg_of_line ();

   ncols = SLtt_Screen_Cols;
   if (Most_Show_Wrap_Marker)
     ncols--;

   b = most_forward_columns (b, e, ncols);
   
   /* Do not wrap the line if the last character falls on the last column 
    * of the display.
    */
   if (Most_Show_Wrap_Marker)
     {
	if (Most_UTF8_Mode == 0)
	  b1 = b + 1;
	else
	  b1 = SLutf8_skip_char (b, Most_Eob);
	
	if ((b1 <= e)
	    && (b1 < Most_Eob)
	    && (*b1 == '\n'))
	  b = b1;
     }

   return b;
}

MOST_INT most_forward_line (MOST_INT save)
{
   MOST_INT m, n;
   unsigned char *p;
   unsigned char *pmax;

   n = save;
   pmax = Most_Eob;

   if (n > 0)
     {
	if (Most_B_Opt)
	  {
	     m = (Most_Eob - (Most_Beg + Most_C_Offset)) / 16;
	     if (n > m) n = m;
	     Most_C_Offset += n * 16;
	     Most_C_Line += n;
	     return n;
	  }

	p = NULL;
	while (n--)
	  {
	     p = end_of_line (p);
	     Most_C_Offset = p - Most_Beg;
	     if (p == Most_Eob) return (save - (n + 1));
	     Most_C_Line++; Most_C_Offset++;

	     if (Most_Selective_Display)
	       {
		  /* Skip past lines with too much indentation to the start
		   * of a valid one.
		   */
		  p = Most_Beg + Most_C_Offset;
		  while (p < pmax)
		    {
		       while ((p < pmax) && (*p <= ' ')) p++;
		       if (most_apparant_distance(p) < Most_Selective_Display)
		       	 break;
		       Most_C_Offset = (p - Most_Beg);
		       p = end_of_line (p);
		       if (p < pmax) p++;
		    }
		  Most_C_Offset = (p - Most_Beg);
	       }
	     p = Most_Beg + Most_C_Offset;
	  }
     }
   else
     {
	if (Most_B_Opt)
	  {
	     m = Most_C_Offset / 16;
	     if (n < m) n = m;
	     Most_C_Offset += n * 16;
	     Most_C_Line += n;
	     return n;
	  }
	else while (n++)
	  {
	     p = most_beg_of_line();
	     Most_C_Offset = (p - Most_Beg);
	     if (Most_C_Offset == 0) return (n - (save + 1));
	     Most_C_Line--;
	     Most_C_Offset--;

	     if (Most_Selective_Display)
	       {
		  /* Skip past lines with too much indentation to the start
		   * of a valid one.
		   */
		  p = Most_Beg + Most_C_Offset;
		  while (p > Most_Beg)
		    {
		       /* skip all blank lines */
		       while ((p > Most_Beg) && (*p <= ' ')) p--;
		       pmax = p;
		       Most_C_Offset = pmax - Most_Beg;
		       p = most_beg_of_line ();
		       Most_C_Offset = p - Most_Beg;
		       while ((p < pmax) && (*p <= ' ')) p++;
		       if (most_apparant_distance(p) < Most_Selective_Display)
		       	 break;
		       Most_C_Offset = p - Most_Beg;
		       p = most_beg_of_line ();
		       if (p > Most_Beg) p--;
		       Most_C_Offset = p - Most_Beg;
		    }
		  Most_C_Offset = p - Most_Beg;
	       }
	  }
     }
   return(save);
}

/* Count lines in the region.  A half line counts as 1 */
MOST_INT most_count_lines(unsigned char *beg, unsigned char *end)
{
   MOST_INT save_line, n;
   unsigned char *save_beg, *save_eob;
   MOST_UINT save_pos;
   int dn = 1000;

   if (Most_B_Opt) return(1 + (MOST_INT)(end - beg) / 16);

   save_line = Most_C_Line; save_beg = Most_Beg; save_eob = Most_Eob;
   save_pos = Most_C_Offset;

   Most_Beg = beg; Most_Eob = end;
   Most_C_Offset = 0;

   n = 1;
   while((dn = most_forward_line(dn)) != 0) n += dn;

   Most_C_Offset = save_pos;
   Most_Eob = save_eob;
   Most_Beg = save_beg;
   Most_C_Line = save_line;
   return(n);
}

void most_goto_line (MOST_INT line)
{
   MOST_INT dif_c, dif_b, dif_t;

   if (line < 1) line = 1;
   most_read_to_line(line);
   if (line > Most_Num_Lines) line = Most_Num_Lines;

   if (Most_B_Opt)
     {
	Most_C_Offset = (16 * (line - 1));
	Most_C_Line = line;
	return;
     }

   dif_c = line - Most_C_Line;
   dif_b = line - Most_Num_Lines;
   dif_t = line - 1;

    /* 4 possibilites */
   if (dif_c <= 0)
     {
	if (dif_t < -dif_c) /* go from top */
	  {
	     Most_C_Line = 1;
	     Most_C_Offset = 0;
	     (void) most_forward_line(dif_t);
	  }
	else  /* from curr back */
	  {
	     (void) most_forward_line(dif_c);
	  }
     }
   else if (dif_c > 0)
     {
	if ((dif_c + dif_b) < 0) /* go from curr */
	  {
	     (void) most_forward_line(dif_c);
	  }
	else
	  {
	     Most_C_Line = Most_Num_Lines;
	     Most_C_Offset = (Most_Eob - Most_Beg);
	     (void) most_forward_line(dif_b);
	  }
     }
}

/* return line the point is on without the final '\n's  */
int most_extract_line(unsigned char **beg, unsigned char **end)
{
   *beg = most_beg_of_line();
   *end = end_of_line (*beg);

   return 0;
}

MOST_INT most_what_line(unsigned char *pos)
{
   unsigned int save_pos;
   MOST_INT save_line, dir;
   MOST_INT dif_c, dif_b,dif_t;
   int ret;

   if (Most_B_Opt)
     {
	return (1 + (pos - Most_Beg)/16);
     }

   if (Most_Selective_Display)
     {
        return most_count_lines (Most_Beg, pos);
     }

   save_pos = Most_C_Offset;
   save_line = Most_C_Line;

   dif_c = pos - (Most_Beg + Most_C_Offset);
   dif_b = pos - Most_Eob;
   dif_t = pos - Most_Beg;

    /* 4 possibilites */
   if (dif_c <= 0)
     {
	if (dif_t < -dif_c) /* go from top */
	  {
	     Most_C_Line = 1;
	     Most_C_Offset = 0;
	     dir = 1;
	  }
	else  /* from curr back */
	  {
	     dir = -1;
	  }
     }
   else if (dif_c > 0)
     {
	if ((dif_c + dif_b) < 0) /* go from curr */
	  {
	     dir = 1;
	  }
	else
	  {
	     Most_C_Line = Most_Num_Lines;
	     Most_C_Offset = Most_Eob - Most_Beg;
	     dir = -1;
	  }
     }
   if (dir == 1)
     {
	while (1)
	  {
	     unsigned char *cpos;

	     cpos = end_of_line (NULL);
	     Most_C_Offset = cpos - Most_Beg;

	     if (cpos >= pos)
	       break;

	     Most_C_Offset++;
	     Most_C_Line++;
	  }
     }
   else
     {
	while (1)
	  {
	     unsigned char *cpos;

	     cpos = most_beg_of_line ();
	     Most_C_Offset = cpos - Most_Beg;
	     if (pos >= cpos)
	       break;
	     Most_C_Line--;
	     Most_C_Offset--;
	  }
     }

   ret = Most_C_Line;
   Most_C_Offset = save_pos;
   Most_C_Line = save_line;
   return(ret);
}

/* given a buffer position, find the line and column */
void most_find_row_column(unsigned char *pos, MOST_INT *r, MOST_INT *c)
{
   unsigned char *beg;
   unsigned int save_offset;
   MOST_INT save_line;

   if (pos <= Most_Beg)
     {
	*r = 1;
	*c = 1;
	return;
     }

   save_line = Most_C_Line;
   save_offset = Most_C_Offset;
   *r = most_what_line(pos);

   if (Most_B_Opt)
     {
	*c = (int) (pos - Most_Beg) - (*r - 1) * 16 + 1;
	return;
     }
   Most_C_Line = *r;
   Most_C_Offset = pos - Most_Beg;

   /* Now we have found the line it is on so.... */
   beg = most_beg_of_line();
   *c = 1;
   if (Most_UTF8_Mode)
     {
	/* FIXME: This should take into account the printable representation
	 * of the characters.
	 */
	while ((beg = SLutf8_skip_chars (beg, pos, 1, NULL, 1)) < pos)
	  *c += 1;
     }
   else 
     while (beg++ < pos) *c = *c + 1;

   Most_C_Line = save_line;
   Most_C_Offset = save_offset;
}

Most_Buffer_Type *most_switch_to_buffer(Most_Buffer_Type *nnew)
{
   Most_Buffer_Type *old;
   old = Most_Buf;
   Most_Buf = nnew;
   Most_Beg = Most_Buf->beg;
   Most_Eob = Most_Buf->end;
   return old;
}

Most_Buffer_Type *most_create_buffer(char *file)
{
   Most_Buffer_Type *buf;

   buf = (Most_Buffer_Type *) MOSTMALLOC(sizeof(Most_Buffer_Type));
   memset ((char *) buf, 0, sizeof(Most_Buffer_Type));
   strcpy(buf->file,file);
   return(buf);
}

unsigned char *most_malloc(unsigned int n)
{
   unsigned char *b = (unsigned char *) SLMALLOC(n);
   if (b == NULL)
     {
	most_exit_error("malloc: Memory Allocation Error.");
     }
   return b;
}

unsigned char *most_realloc(unsigned char *p, unsigned int n)
{
   unsigned char *b = (unsigned char *) SLREALLOC(p, n);
   if (b == NULL)
     {
	most_exit_error("malloc: Memory Allocation Error.");
     }
   return b;
}
