/* -*- mode: C; mode: fold -*- */
/*
 This file is part of MOST.

 Copyright (c) 1991, 1999, 2002, 2005-2017 John E. Davis

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

#include "most.h"
#include "window.h"
#include "line.h"
#include "file.h"
#include "display.h"
#include "search.h"

/* Note!!!  The regular expression searches may not work.  I have not
 * tested them.
 * FIXME!!!  This whole thing needs to be rewritten.  It is a mess.
 */

int Most_Case_Sensitive = 0;
char Most_Search_Str[256];
int Most_Search_Dir = 1;

#include "jdmacros.h"

typedef struct Search_Type_ Search_Type;
struct Search_Type_
{
   void *cd;
   unsigned char *(*fsearch_method) (Search_Type *, unsigned char *, unsigned char *, unsigned char *);
   unsigned char *(*bsearch_method) (Search_Type *, unsigned char *, unsigned char *, unsigned char *);
   void (*close_method)(Search_Type *);
};


static unsigned char Ascii_Upper[256];

static void bs_search_init (void)
{
   static int inited = 0;
   unsigned int i;

   if (inited) return;
   for (i = 0; i < 256; i++) Ascii_Upper[i] = i;
   for (i = 'a'; i <= 'z'; i++) Ascii_Upper[i] = (i - 32);
   inited = 1;
}

#define UPCASE(ch) (Ascii_Upper[(ch)])

#define CHAR_EQS(a,b) \
   (((a) == (b)) || (!Most_Case_Sensitive && (UPCASE(a) == UPCASE(b))))

static int is_ansi_escape (unsigned char **begp, unsigned char *end)
{
   unsigned char *p, ch;

   p = *begp;
   if ((p == end) || (*p++ != '[')) return 0;

   /* Look for "ESC X m", where X is one of:
    *   X = ""
    *   X = digit
    *   X = digit ; digit ...
    */
   ch = *p++;
   while (isdigit (ch))
     {
	while ((p < end) && isdigit (*p))
	  p++;

	if (p == end)
	  return 0;

	ch = *p++;
	if (ch == 'm')
	  break;

	if ((ch != ';') || (p == end))
	  return 0;

	ch = *p++;
     }
   if (ch != 'm')
     return 0;

   *begp = p;
   return 1;
}

static int is_rev_ansi_escape (unsigned char *beg, unsigned char **endp)
{
   unsigned char *p, ch;

   p = *endp;
   if (p == beg)
     return 0;

   ch = *p--;
   while (isdigit (ch))
     {
	while ((p > beg) && isdigit (*p))
	  p--;

	if (p == beg)
	  return 0;

	ch = *p--;

	if (ch == '[')
	  break;

	if ((ch != ';') || (p == beg))
	  return 0;

	ch = *p--;
     }

   if ((ch != '[') || (p < beg) || (*p != 033))
     return 0;

   *endp = p-1;
   return 1;
}

/* These routines have special processing for ANSI escape sequence and backspace handling.
 * For example, "hello world" may occur as:
 *   plain: hello world
 *   underlined: h_e_l_l_o_ world
 *   underlined: _h_e_l_l_o world
 *   bold: hheelllloo world
 *   ansi: [5mhello[m world
 *   ansi: [5mh[32;43mello[m world
 */

/* This routine returns the 1 + position of first match of key in str.
 * searches from beg up to but not including end.  Handles backspace, etc
 */
static unsigned char *
bs_fsearch (Search_Type *st,
	    unsigned char *beg, unsigned char *end,
	    unsigned char *key)
{
   unsigned char ch, ch1, ch1up;
   unsigned char *pos;
   int cis, key_len, j, str_len;

   (void) st;
   key_len = strlen ((char *)key);
   if (key_len == 0)
     return Most_Eob;

   str_len = (int) (end - beg);
   if (str_len < key_len) return (Most_Eob);

   cis = (Most_Case_Sensitive == 0);
   ch1 = key[0];
   ch1up = UPCASE(ch1);

   while (1)
     {
	/* Find first character that matches */
	while (1)
	  {
	     if (beg == end) return Most_Eob;

	     ch = *beg++;
	     if ((ch == ch1)
		 || (cis && (ch1up == UPCASE(ch))))
	       break;
	  }

	 /* so we have a position of possible match */
	j = 1;

	pos = beg;  /* save this position so we start from here again */

	while (1)
	  {
	     if (j == key_len)
	       return pos - 1;

	     if (beg == end)
	       break;

	     ch = *beg++;

	     /* FIXME:  This only works for x^Hx but not x^Hx^Hx...
	      * It is probably better to skip all the ^H characters
	      * until the end.  That is, regard "a^Hb^Hc" as 'c'.
	      */
	     if ((ch == 8)
		 && (beg + 1 < end)
		 && (CHAR_EQS(key[j - 1], *beg)
		     || (*beg == '_')))
	       {
		  ch = *(beg + 1);
		  beg += 2;
	       }
	     else if ((ch == '_') && (beg + 1 < end))
	       {
		  ch = *beg++;
		  if (ch == 8) ch = *beg++;
		  else
		    {
		       ch = '_';
		       beg--;
		    }
	       }
	     else if ((ch == 033) && is_ansi_escape (&beg, end))
	       continue;

	     if (!CHAR_EQS(ch, key[j]))
	       break;

	     j++;
	  }

	beg = pos;
     }
}

/*
 *  Search backwards in the buffer "beg" up to, but not including "end" for
 *  pattern "key".  It handles backspaces, etc
 */
static unsigned char *
bs_bsearch (Search_Type *st,
	    unsigned char *beg, unsigned char *end,
	    unsigned char *key)
{
   unsigned char ch, ch1, ch1up;
   unsigned char *pos;
   int key_len,j, str_len;
   int cis;

   (void) st;
   key_len = strlen ((char *)key);
   if (key_len == 0) return Most_Eob;

   str_len = (int) (end - beg);
   if (str_len < key_len) return Most_Eob;

   ch1 = key[key_len-1];
   ch1up = UPCASE(ch1);
   cis = (Most_Case_Sensitive == 0);

   while (1)
     {
	while (1)
	  {
	     if (end < beg)
	       return Most_Eob;

	     ch = *end--;
	     if ((ch == ch1)
		 || (cis && (ch1up == UPCASE(ch))))
	       break;
	  }

	pos = end;  /* save this position so we start from here again */

	j = key_len - 2;

	while (1)
	  {
	     if (j < 0)
	       return end + 1;
	     if (end < beg)
	       break;

	     ch = *end--;

	     if ((ch == 8)
		 && (end >= beg + 1)
		 && (CHAR_EQS(key[j + 1], *end)
		     || (*end == '_')))
	       {
		  ch = *(end - 1);
		  end -= 2;
	       }
	     else if ((ch == '_')
		      && (end >= beg + 1))
	       {
		  ch = *end--;
		  if (ch == 8) ch = *end--;
		  else
		    {
		       ch = '_';
		       end++;
		    }
	       }
	     else if ((ch == 'm') && is_rev_ansi_escape (beg, &end))
	       continue;

	     if (!CHAR_EQS(ch, key[j]))
	       break;

	     j--;
	  }
	end = pos;
     }
}

static int bs_open_search (Search_Type *st, char *key)
{
   (void) key;

   bs_search_init ();
   st->fsearch_method = bs_fsearch;
   st->bsearch_method = bs_bsearch;
   st->close_method = NULL;
   st->cd = NULL;
   return 0;
}

static unsigned char *sl_fsearch (Search_Type *st,
				  unsigned char *beg, unsigned char *end,
				  unsigned char *key)
{
   unsigned char *p;

   (void) key;
   if (NULL == (p = SLsearch_forward ((SLsearch_Type *)st->cd, beg, end)))
     p = Most_Eob;
   return p;
}

static unsigned char *sl_bsearch (Search_Type *st,
				  unsigned char *beg, unsigned char *end,
				  unsigned char *key)
{
   unsigned char *p;

   (void) key;
   if (NULL == (p = SLsearch_backward ((SLsearch_Type *)st->cd, beg, end, end)))
     p = Most_Eob;
   return p;
}

static void sl_search_close (Search_Type *st)
{
   if (st->cd != NULL)
     SLsearch_delete ((SLsearch_Type *) st->cd);
}

static int sl_open_search (Search_Type *st, char *key)
{
   unsigned int flags = 0;

   if (Most_Case_Sensitive == 0) flags |= SLSEARCH_CASELESS;
   if (Most_UTF8_Mode) flags |= SLSEARCH_UTF8;

   if (NULL == (st->cd = SLsearch_new ((SLuchar_Type *) key, flags)))
     return -1;

   st->fsearch_method = sl_fsearch;
   st->bsearch_method = sl_bsearch;
   st->close_method = sl_search_close;
   return 0;
}

static int
do_search_internal (Search_Type *st,
		    unsigned char *from, int repeat, MOST_INT *col)
{
   /* return the line match was found as well as line number,
    * search from i on; assume that line_array match the i so we need
    * no initial lookup */

   int test;
   MOST_INT save_line, the_col, row, s_len;
   char string[300];
   unsigned char *pos, *eob;
   unsigned int save_ofs;
   unsigned int found_ofs;

   if (*Most_Search_Str == 0)
     {
	most_message("Search string not specified.",1);
	return -1;
     }

   if ((from < Most_Beg) || (from > Most_Eob)) return -1;

   save_ofs = Most_C_Offset;
   save_line = Most_C_Line;
   found_ofs = Most_Eob - Most_Beg;
   *col = 0;
   s_len = strlen (Most_Search_Str);
   pos = from;

   eob = Most_Eob;

   test = repeat && (pos < Most_Eob) && (pos >= Most_Beg);
   while(test)
     {
	if (Most_Search_Dir == 1)
	  {
	     while (1)
	       {
		  unsigned int pos_ofs;

		  pos = (*st->fsearch_method)(st, pos, Most_Eob, (unsigned char*) Most_Search_Str);
		  pos_ofs = (unsigned int) (Most_Eob - Most_Beg);

		  if (pos < Most_Eob)
		    break;

		  if (0 == most_read_file_dsc (10, 0))
		    {
		       /* Pointer may be invalid after this call */
		       pos = Most_Beg + pos_ofs;
		       break;
		    }

		  /* This might need an adjustment */
		  pos = Most_Beg + (pos_ofs - s_len);
		  if (pos < Most_Beg) pos = Most_Beg;
	       }
	  }
	else
	  pos = (*st->bsearch_method)(st, Most_Beg, pos, (unsigned char *) Most_Search_Str);

	if (pos < Most_Eob)
	  {
	     repeat--;
	     found_ofs = pos - Most_Beg;
	     if (Most_Search_Dir == 1)
	       pos += s_len;
	     else pos--;
	  }
	test = repeat && (pos < Most_Eob) && (pos >= Most_Beg);
	if (SLKeyBoard_Quit)
	  {
	     most_message ("Search Interrupted.", 1);
	     break;
	  }
     }

   if (eob != Most_Eob)
     Most_Num_Lines = most_count_lines (Most_Beg, Most_Eob);

   if (repeat) /* not found */
     {
	*col = 0;

	(void) sprintf(string,"Search failed: %s",Most_Search_Str);
	most_message(string,1);
	row = -1;
     }
   else /* if ( !Most_T_Opt && !Most_B_Opt) */   /* expand tabs to get col correct */
     {
	most_find_row_column(Most_Beg + found_ofs, &row, &the_col);
	if (Most_B_Opt) *col = the_col + 52;
	else
	  *col = 1 + most_apparant_distance(Most_Beg + found_ofs);
     }

   Most_C_Offset = save_ofs;
   Most_C_Line = save_line;
   if (row > 0) Most_Curs_Offset = found_ofs;

   return row;
}

static int search_internal (Search_Type *st, unsigned char *from, int repeat, MOST_INT *colp)
{
   int status;

   status = do_search_internal (st, from, repeat, colp);

   if (st->close_method != NULL)
     (st->close_method)(st);

   return status;
}

static int simple_search (unsigned char *from, int repeat, MOST_INT *colp)
{
   Search_Type st;

   if (Most_V_Opt || Most_B_Opt)
     {
	/* Nothing special about the ^H and _ chars.  User faster SLsearch */
	if (-1 == sl_open_search (&st, Most_Search_Str))
	  return -1;
     }
   else if (-1 == bs_open_search (&st, Most_Search_Str))
     return -1;

   return search_internal (&st, from, repeat, colp);
}


static void re_search_close (Search_Type *st)
{
   if (st->cd != NULL)
     SLregexp_free ((SLRegexp_Type *) st->cd);
}

static unsigned char *
re_fsearch (Search_Type *st,
	    unsigned char *beg, unsigned char *end,
	    unsigned char *key)
{
   SLRegexp_Type *re;
   unsigned char *p;
   unsigned int flags;

   (void) key;

   re = (SLRegexp_Type *)st->cd;
   (void) SLregexp_get_hints (re, &flags);

   while (beg < end)
     {
	unsigned char *line_end = beg;

	while (line_end < end)
	  {
	     unsigned char ch = *line_end++;
	     if (ch == '\n') break;
	  }

	p = (unsigned char *)SLregexp_match (re, (char *)beg, (line_end - beg));
	if (p != NULL)
	  {
	     if ((0 == (flags & SLREGEXP_HINT_BOL))
		 || (p != beg)
		 || (beg == Most_Beg)
		 || (*(beg - 1) == '\n'))
	       return p;
	  }
	beg = line_end;
     }

   return Most_Eob;
}

static unsigned char *
re_bsearch (Search_Type *st,
	    unsigned char *beg, unsigned char *end,
	    unsigned char *key)
{
   SLRegexp_Type *re;
   unsigned char *line_end, *eob;
   unsigned int flags;

   (void) key;
   re = (SLRegexp_Type *)st->cd;
   (void) SLregexp_get_hints (re, &flags);

   line_end = end;
   eob = Most_Eob;
   while (line_end < eob)
     {
	if (*line_end == '\n')
	  break;
	line_end++;
     }

   while (end > beg)
     {
	unsigned char *p, *match;
	unsigned char *line = end;
	while (line > beg)
	  {
	     line--;
	     if (*line == '\n')
	       {
		  line++;
		  break;
	       }
	  }

	/* line is now at the start of a line */
	if (NULL != (match = (unsigned char *)SLregexp_match (re, (char *)line, line_end-line)))
	  {
	     if (match >= end)
	       {
		  /* Match occurs to right of boundary.  Try previous line */
		  end = line_end = line-1;
		  continue;
	       }

	     if (flags & SLREGEXP_HINT_BOL)
	       return match;

	     /*    t    tt  z    t  t     t z */
	     /* Find match closest to end */
	     while ((line < end)
		    && (NULL != (p = (unsigned char *)SLregexp_match (re, (char *)line, (line_end - line))))
		    && (p < end))
	       {
		  match = p;
		  line++;
	       }
	     return match;
	  }

	end = line-1;
	line_end = end;
     }

   return Most_Eob;
}


static int regexp_search (unsigned char *from, int repeat, MOST_INT *colp)
{
   Search_Type st;
   SLRegexp_Type *re;
   char *pattern;
   unsigned int flags;

   pattern = Most_Search_Str;

   flags = 0;
   if (Most_Case_Sensitive == 0) flags |= SLREGEXP_CASELESS;

   re = SLregexp_compile (pattern, flags);
   if (re == NULL)
     return -1;

   (void) SLregexp_get_hints (re, &flags);
   if (flags & SLREGEXP_HINT_OSEARCH)
     {
	SLregexp_free (re);
	return simple_search (from, repeat, colp);
     }

   st.cd = (void *)re;
   st.fsearch_method = re_fsearch;
   st.bsearch_method = re_bsearch;
   st.close_method = re_search_close;

   return search_internal (&st, from, repeat, colp);
}

int most_search (unsigned char *from, int repeat, MOST_INT *colp)
{
   if (Most_Do_Regexp_Search)
     return regexp_search (from, repeat, colp);

   return simple_search (from, repeat, colp);
}
