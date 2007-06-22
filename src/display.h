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

/* These numbers match ANSI escape sequences of form ESC [ x m */
#define MOST_BOLD_COLOR		1
#define MOST_ULINE_COLOR	4
#define MOST_STATUS_COLOR	7

#define MOST_EMBEDDED_COLOR_OFFSET 32

extern void most_tt_set_color (int color);
extern void most_tt_reverse_video(void);
extern void most_tt_bold_video(void);
extern void most_tt_underline_video(void);
extern void most_tt_normal_video(void);
extern void most_wide_width(void);
extern void most_narrow_width(void);
extern void most_enable_cursor_keys(void);

extern void most_goto_rc (int, int);
extern void most_setup_colors (void);
