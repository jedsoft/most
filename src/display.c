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
#include <stdio.h>
#include <slang.h>
#include "jdmacros.h"

#include "display.h"

void most_narrow_width(void)
{
   SLsmg_refresh ();
   SLtt_write_string ("\033[?3l");
}

void most_wide_width(void)
{
   SLsmg_refresh ();
   SLtt_write_string ("\033[?3h");
}

void most_tt_set_color (int color)
{
   SLsmg_set_color (color);
}

void most_tt_bold_video (void)
{
   SLsmg_set_color (MOST_BOLD_COLOR);
}

void most_tt_normal_video (void)
{
   SLsmg_set_color (0);
}

void most_tt_underline_video (void)
{
   SLsmg_set_color (MOST_ULINE_COLOR);
}

void most_tt_reverse_video (void)
{
   SLsmg_set_color (MOST_STATUS_COLOR);
}

void most_goto_rc (int r, int c)
{
   SLsmg_gotorc (r - 1, c - 1);
}

