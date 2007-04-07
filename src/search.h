#ifndef _DAVIS_SEARCH_H_
# define _DAVIS_SEARCH_H_
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
# include <stdio.h>
# include <string.h>

extern int Most_Case_Sensitive;
extern int Most_Search_Dir;
#define MOST_SEARCH_BUF_LEN	256
extern char Most_Search_Str[MOST_SEARCH_BUF_LEN];
extern int most_search(unsigned char *, int, MOST_INT *);
#endif

