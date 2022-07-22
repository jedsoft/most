/*
 This file is part of MOST.

 Copyright (c) 2021, 2022 John E. Davis <jed@jedsoft.org>

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
#ifndef _DAVIS_COLOR_H_
# define _DAVIS_COLOR_H_

/* These numbers match ANSI escape sequences of form ESC [ x m */
#define MOST_BOLD_COLOR		1
#define MOST_ULINE_COLOR	4
#define MOST_STATUS_COLOR	7

#define MOST_EMBEDDED_COLOR_OFFSET 256

extern int most_parse_color_escape (unsigned char **begp, unsigned char *end, int *colorp);
extern int most_setup_embedded_colors (void);
#endif
