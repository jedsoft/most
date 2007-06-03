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
#ifndef _DAVIS_LINE_H_
# define _DAVIS_LINE_H_
#if 0
extern int most_analyse_line(unsigned char *, unsigned char *, char *, char *);
extern void most_output(unsigned char *, unsigned int, unsigned char *, unsigned char);
#endif

extern void most_display_line(void);
extern int most_apparant_distance(unsigned char *);
extern unsigned char *most_forward_columns (unsigned char *b, unsigned char *e, unsigned int num_cols);

extern int Most_Show_Wrap_Marker;

#endif

