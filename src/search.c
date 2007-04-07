/* -*- mode: C; mode: fold -*- */
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
#include <string.h>
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

#undef SLANG_REGEXP
#undef HAVE_V8_REGCOMP

#define UPCASE(ch) ((!Most_Case_Sensitive && (ch <= 'z') && (ch >= 'a')) ? (ch - 32) : ch)

#if	defined(HAVE_V8_REGCOMP) || defined(SLANG_REGEXP)

/*
 *  Regular expression stuff
 *  ************************
 */
# ifdef	HAVE_V8_REGCOMP
#  include "regexp.h"

/* pointer to area malloced by regcomp() */
static struct regexp	*regpattern = NULL;

# else

/* slang regular expression structure */
#if SLANG_VERSION < 20000
static SLRegexp_Type	regdata;
/* buffer for compiled regular expression */
static unsigned char regbuf[sizeof(Most_Search_Str) * 3];
#else
static SLRegexp_Type *Regexp;
#endif
# endif	/* HAVE_V8_REGCOMP */

/* set if regexp compiled OK, 0 if regular expression error */
static int		regcompOK = 1;

/* uncompiled search pattern */
static char		savepattern[sizeof(Most_Search_Str)];

/*
 * This function is called by the V8 regcomp to report
 * errors in regular expressions.
 */
static void regerror(char *s)
{
   char	string[256];

   regcompOK = 0;			/* clear flag, ie regexp error */
   sprintf(string, "Regular expression error: %s", s);
   most_message(string, 1);
}

/*
 *  Compiles the search pattern "key" into a regular expression for use by
 *  do_regexec()
 *
 *  Returns:  1	 success
 *	      0	 error
 *
 */
static int do_regcomp(unsigned char *key)
{
   static int old_Most_Case_Sensitive;
   unsigned char UpCaseKey[sizeof(savepattern)];
# ifndef HAVE_V8_REGCOMP
   int	posn;			/* reg exp error at this offset */
# endif

   /*
    *	Only recompile search string if it has changed
    */
   if ((0 == strcmp ((char *)key, (char *) savepattern))
       && (Most_Case_Sensitive == old_Most_Case_Sensitive))
     return 1;

# ifdef	HAVE_V8_REGCOMP
   if (regpattern != NULL)
     free(regpattern);
# endif

   if ( strlen((char *)key) >= sizeof(savepattern) )
     {
	regerror("Search string too long");
	savepattern[0] = '\0';
	return 0;
     }

   old_Most_Case_Sensitive = Most_Case_Sensitive;

   if ( Most_Case_Sensitive == 0 )
     {
	register unsigned char	*p;		/* ptr to UpCaseKey */
	register unsigned char	*keyp;		/* ptr to key */
	register unsigned char	c;		/* source character */

	    /*
	     *	Make a upper case copy of every character from "key"
	     *	into "UpCaseKey"
	     */
	p = UpCaseKey;
	keyp = key;
	while ( (c = *keyp++) != '\0' )
	  *p++ = UPCASE(c);

	*p = '\0';
     }

   strcpy((char *)savepattern, (char *)key);

# ifdef	HAVE_V8_REGCOMP
   regpattern = regcomp((char *)(Most_Case_Sensitive ? key : UpCaseKey));
   if (regpattern == NULL)
     {
	regcompOK = 1;
	return 1;
     }
# else

#  if SLANG_VERSION < 20000
   regdata.case_sensitive = 1;
   regdata.buf = regbuf;
   regdata.pat = Most_Case_Sensitive ? key : UpCaseKey;
   regdata.buf_len = sizeof (regbuf);
   posn = SLang_regexp_compile(&regdata);
#  else
   if (Regexp != NULL)
     SLregexp_free (Regexp);
   if (NULL == (Regexp = SLregexp_compile ((char *)key, Most_Case_Sensitive ? 0 : SLREGEXP_CASELESS)))
     posn = -1;
   else
     posn = 0;
#  endif

   if (posn == 0)
     {
	regcompOK = 1;
	return 1;
     }
   regerror ("Unable to compile pattern");
# endif	/* HAVE_V8_REGCOMP */

   /*
    * regcomp has already printed error message via regerror().
    */
   savepattern[0] = '\0';
   return 0;				/* failure */
}

/*
 * Call the appropriate regular expression execute function
 */

static unsigned char *do_regexec(unsigned char *string, unsigned int len)
{
# ifdef	HAVE_V8_REGCOMP
   if ( regexec(regpattern, (char *)string) )
     return( (unsigned char *)regpattern->startp[0] );
   else
     return( NULL );
# else
#  if SLANG_VERSION < 20000
   return ( SLang_regexp_match(string, len, &regdata) );
#  else
   return (unsigned char *)SLregexp_match (Regexp, (char *)string, len));
#  endif
# endif	/* HAVE_V8_REGCOMP */
}

/*
 *  Make a upper case copy of a string.	 Also changes any "c\b" character
 *  strings into just "" so that highlighted and underlined characters
 *  can be searched.
 *
 *  Reuses malloced memory, so a copy cannot be retained between calls.
 */

static unsigned char *StrUpCaseCopy(unsigned char *input)
{
   static unsigned char *uppercase;	/* ptr to malloced area */
   static size_t	  bufsize;	/* size of malloced area */
   unsigned char	 *src;		/* ptr to source */
   register unsigned char *dest;	/* ptr to destination */
   register int	  idx;	/* index into uppercase[] */
   register unsigned char c;		/* source character */
   size_t		  length;	/* size of string to copy */

   src = input;
   length = strlen((char *)src) + 1;	/* len of line plus terminator */

   if ( length > bufsize )
     {
	if ( uppercase != (unsigned char *)NULL )
	  free(uppercase);

	bufsize = (length > 256 ) ? length : 256;	/* 256 byte default */

	uppercase = (unsigned char *)malloc(bufsize);
	if ( uppercase == (unsigned char *)NULL )
	  return(NULL);
     }

    /*
     *	Make the copy converting to upper case as we go
     */

   dest = uppercase;

   for ( idx = 0 ; (c = *src) != '\0' ; src++ )
     {
	if ( c == '\b' )		/* backspace */
	  {
	     if ( idx-- > 0 )
	       dest--;			/* back up dest pointer */
	  }
	else
	  {
	     if ( idx++ >= 0 )
	       *dest++ = UPCASE(c);
	  }
     }

   *dest = '\0';		/* add termination */

   return(uppercase);
}

/*
 *  Given an offset into a copy made by StrUpCaseCopy() and a pointer to the
 *  original string, returns a pointer into the original string corresponding
 *  to this offset.
 */

static unsigned char *GetOrigPtr(unsigned char *original, int offset)
{
   register unsigned char *p = original;
   register int	    j = offset;

    /*
     *	Step through, adjusting offset according to backspaces found
     */
   while ( *p != '\0' )
     {
	if ( *p == '\b' )
	  j++;
	else
	  j--;

	if ( j < 0 )
	  break;
	else
	  p++;
     }

   return(p);
}
#endif	/* HAVE_V8_REGCOMP || SLANG_REGEXP */

/* This routine returns the 1 + position of first match of key in str.
   key is modified to match the case of str. */
/* We should try to optimize this routine */
/* searches from beg up to but not including end */

#if defined(SLANG_REGEXP)
static unsigned char *
  forw_search_region_regexp (unsigned char *beg, unsigned char *end,
			     unsigned char *key)
{
   if (Regexp != NULL)
     SLregexp_free (Regexp);

   if (NULL == (Regexp = SLregexp_compile ((char *)key, Most_Case_Sensitive ? 0 : SLREGEXP_CASELESS)))
     return NULL;

   if ( do_regcomp(key) == 0 )
     return(Most_Eob);

    /*
     *	For regular expression searches we need to do a line by line
     *	search, so it is necessary to temporarily replace '\n' with '\0'
     *	characters.
     * 
     * ***** THIS IS NOT ALLOWED FOR MMAPPED FILES!!!!!!!!! *****
     */
   p = beg;
   linebeg = beg;

   while (linebeg < end)
     {
	while ((p < end) && (*p != '\n')) p++;
	if (p == end) break;
	/* *p = 0; -- not allow for mmapped files */

	if ( Most_Case_Sensitive == 0 )	/* i.e. case insensitive */
	  {
	     copy = StrUpCaseCopy(linebeg);
	     if ( copy == (unsigned char *)NULL )
	       return(Most_Eob);
	  }

	/*
	 * Quick sanity check for beginning of line archored tests.
	 * If 1st char of key is "^", then the character before linebeg (which
	 * must be beyond the start of the window), must be a "\n",
	 * otherwise do_regexec() isn't called.
	 */
	if ( 
# if 0
	     ((*key != '^') 
	      || (linebeg > Most_Win->beg_pos && linebeg[-1] == '\n'))
	     &&
#endif
	     (match = do_regexec(Most_Case_Sensitive ? linebeg : copy)))
	  {
	     /* *p = '\n'; --- NOT ALLOWED */
	     if ( Most_Case_Sensitive == 0 )
	       {
		/*
		 *  Use offset into "copy" as idx to find point in
		 *  real line.
		 */
		  return( GetOrigPtr(linebeg, match - copy) );
	       }
	     else
	       {
		  return( match );
	       }
	  }

	/* *p++ = '\n'; */
	linebeg = p;
     }

   return(Most_Eob);
}

#endif
static unsigned char *forw_search_region(unsigned char *beg,
					 unsigned char *end,
					 unsigned char *key)
{
#if defined(HAVE_V8_REGCOMP) || defined(SLANG_REGEXP)
   return forw_search_region_regexp (beg, end, key);
#else
   char ch, char1, work[256];
   unsigned char *pos;
   int key_len,j, str_len;

   if (Most_Case_Sensitive)
     {
	strcpy(work, (char *) key);
	key_len = strlen((char *) key);
     }
   else
     {
          /* upcase key */
	key_len = 0;
	while (0 != (ch = key[key_len]))
	  {
	     ch = UPCASE(ch);
	     work[key_len++] = ch;        /* null char is ok */
	  }
     }

   if (key_len == 0)
     return Most_Eob;

   str_len = (int) (end - beg);
   if (str_len < key_len) return (Most_Eob);

# if 0
   str_len -= key_len; /* effective length */
   end -= (key_len - 1);
# endif

   char1 = work[0];

   while (1)
     {
	 /* Find first character that matches */
	while (1)
	  {
	     if (beg == end) return Most_Eob;

	     ch = *beg++;
	     ch = UPCASE(ch);
	     if (ch == char1)
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
		 && (Most_V_Opt == 0)
		 && ((work[j - 1] == UPCASE(*beg))
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

	     if (UPCASE(ch) != work[j])
	       break;

	     j++;
	  }

	beg = pos;
     }
#endif	/* HAVE_V8_REGCOMP || SLANG_REGEXP */
}

/*
 *  Search backwards in the buffer "beg" up to, but not including "end" for
 *  pattern "key".
 */

static unsigned char *back_search_region(unsigned char *beg,
					 unsigned char *end,
					 unsigned char *key)
{
#if	defined(HAVE_V8_REGCOMP) || defined(SLANG_REGEXP)
   register unsigned char	*p;
   unsigned char		*endp,		/* end of line */
   *lastmatch,	/* last match in line */
   *endprevline,	/* end of line before this one */
   *match;		/* ptr to matching string */
   unsigned char		savec;		/* last char on line */

    /*
     *	Compile "key" into an executable regular expression
     */
   if ( do_regcomp(key) == 0 )
     return(Most_Eob);

    /*
     *	Starting from the end of the buffer, break the buffer into lines
     *	then for each line do forward search to find a match.  If one is
     *	found, move pointer forward one character and try again until
     *	unsuccessful.  In this way we find the last match on the line
     *	and isn't that what we want to do in a reverse search.
     */
   endp = end;
   lastmatch = Most_Eob;
   while ( 1 )			/* forever loop */
     {
	if ( (endp < beg) )
	  return(Most_Eob);		/* Reach start of buffer, no match */

	/* Find the real end of current line */
	if ( (p = (unsigned char *)strchr((char *)endp, '\n')) != NULL )
	  endp = p;

	savec = *endp;
	*endp = '\0';			/* terminate line with NULL */

	/* Find the beginning of line */
	for ( p = endp - 1 ; (p >= beg) && (*p != '\n') ; p-- )
	  {
	  }

	endprevline = p;

	p++;			/* point to 1st char after newline */

	/*
	 *  Keep searching forward in this line till no more matches
	 */
	if ( Most_Case_Sensitive == 0 )		/* i.e. case insensitive */
	  {
	     unsigned char	*copy;		/* ptr to upper case copy */
	     unsigned char	*savecopy;	/* copy of "copy" */

	     copy = StrUpCaseCopy(p);
	     if ( copy == (unsigned char *)NULL )
	       return(Most_Eob);

	     savecopy = copy;

	    /*
	     * Quick sanity check for beginning of line archored tests.
	     * Must be at start of line.
	     */
	     while ( ((*key != '^') || (copy == savecopy))
		    && (match = do_regexec(copy)) )
	       {
		  if ( GetOrigPtr(p, match - savecopy) > end )
		    break;
		  lastmatch = match;
		  if ( *lastmatch == '\0' )	/* key must be "$" or "^" */
		    break;
		  copy = lastmatch + 1;		/* character after match */
	       }

	     if ( lastmatch != Most_Eob )	/* found a match */
	       lastmatch = GetOrigPtr(p, lastmatch - savecopy);
	  }
	else
	  {
	    /*
	     * Quick sanity check for beginning of line archored tests.
	     * Must be at start of buffer or start of line
	     */
	     while ( ( (*key != '^') || (p == endprevline + 1) )
		    && (match = do_regexec(p)) )
	       {
		  if ( match > end )
		    break;
		  lastmatch = match;
		  if ( *lastmatch == '\0' )	/* key must be "$" or "^" */
		    break;
		  p = lastmatch + 1;		/* character after match */
	       }
	  }

	*endp = savec;
	if ( lastmatch != Most_Eob )	/* found a match */
	  return(lastmatch);

	endp = endprevline;
     }
#else
   char ch, char1, work[256];
   unsigned char *pos;
   int key_len,j, str_len;

   if (Most_Case_Sensitive)
     {
	strcpy(work, (char *) key);
	key_len = strlen((char *) key);
     }
   else
     {
	 /* upcase key */
	key_len = 0;
	while (0 != (ch = key[key_len]))
	  {
	     ch = UPCASE(ch);
	     work[key_len++] = ch;        /* null char is ok */
	  }
     }

   if (key_len == 0) return Most_Eob;

   str_len = (int) (end - beg);
   if (str_len < key_len) return Most_Eob;

# if 0
   str_len = str_len - key_len; /* effective length */
   beg += key_len;
# endif

   char1 = work [key_len - 1];

   while (1)
     {
	while (1)
	  {
	     if (end < beg)
	       return Most_Eob;

	     ch = *end--;
	     ch = UPCASE (ch);
	     if (ch == char1)
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
		 && (Most_V_Opt == 0)
		 && ((work[j + 1] == UPCASE(*end))
		     || (*end == '_')))
	       {
		  ch = *(end - 1);
		  end -= 2;
	       }
	     else if ((ch == '_')
		      && (end >= beg + 1)
		      && (Most_V_Opt == 0))
	       {
		  ch = *end--;
		  if (ch == 8) ch = *end--;
		  else
		    {
		       ch = '_';
		       end++;
		    }
	       }

	     if (UPCASE (ch) != work[j])
	       break;

	     j--;
	  }
	end = pos;
     }
#endif	/* HAVE_V8_REGCOMP || SLANG_REGEXP */
}

int most_search(unsigned char *from, int repeat, MOST_INT *col)
{
    /* return the line match was found as well as line number,
     * search from i on; assume that line_array match the i so we need
     * no initial lookup */

   int test;
   MOST_INT save_line, the_col, row, s_len;
   char string[300];
   unsigned char *pos;
   unsigned int save_ofs;
   unsigned int found_ofs;

   if ((from < Most_Beg) || (from > Most_Eob)) return(-1);
   save_ofs = Most_C_Offset;
   save_line = Most_C_Line;
   found_ofs = Most_Eob - Most_Beg;
   *col = 0;
   s_len = strlen (Most_Search_Str);
   pos = from;

   if (*Most_Search_Str)
     {
	test = repeat && (pos < Most_Eob) && (pos >= Most_Beg);
	while(test)
	  {
	     if (Most_Search_Dir == 1)
	       {
		  while (1)
		    {
		       unsigned int pos_ofs;

		       pos = forw_search_region(pos, Most_Eob, (unsigned char*) Most_Search_Str);
		       pos_ofs = (unsigned int) (Most_Eob - Most_Beg);

		       if (pos < Most_Eob)
			 break;

		       if (0 == most_read_file_dsc (10))
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
	       pos = back_search_region(Most_Beg, pos,
					(unsigned char *) Most_Search_Str);

	     if (pos < Most_Eob)
	       {
		  repeat--;
		  found_ofs = pos - Most_Beg;
		  if (Most_Search_Dir == 1)
		    pos += s_len;
		  else pos--;
	       }
	     test = repeat && (pos < Most_Eob) && (pos >= Most_Beg);
	  }
     }

   if (repeat) /* not found */
     {
	*col = 0;
#if	defined(HAVE_V8_REGCOMP) || defined(SLANG_REGEXP)
	if ( regcompOK )	/* don't print error msg if regerr msg */
	  {
#endif
	     if (Most_Search_Str[0] == '\0')
	       most_message("Search string not specified.",1);
	     else
	       {
		  (void) sprintf(string,"Search failed: %s",Most_Search_Str);
		  most_message(string,1);
	       }
#if	defined(HAVE_V8_REGCOMP) || defined(SLANG_REGEXP)
	  }
#endif

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

