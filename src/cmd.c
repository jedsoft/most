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

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include <slang.h>
#include "jdmacros.h"

#include "most.h"
#include "search.h"
#include "window.h"
#include "file.h"
#include "keym.h"
#include "display.h"
#include "sysdep.h"

static char getkey_nocase (void)
{
   char ch = most_getkey ();
   if ((ch >= 'a') && (ch <= 'z')) ch = ch - ' ';

   SLang_flush_input ();

   return ch;
}

void most_next_file (void)
{
   static int next = 1;
   char *msg = NULL;

   if (!Most_Num_Files) msg = "File ring is empty.";
   else if (Most_Num_Files == 1) msg = "There is only one file!";
   else
     {
	most_do_next_file(&next);
     }
   if (msg != NULL) most_message (msg, 1);
}

void most_toggle_case (void)
{
   char *msg;
   Most_Case_Sensitive = !Most_Case_Sensitive;
   if (Most_Case_Sensitive)
     msg = "Searches now respect case.";
   else
     msg = "Searches nolonger respect case.";

   most_message (msg, 0);
}

void most_delete_file_cmd (void)
{
   int bell = 0;
   char ch;
   char *msg;

   if ('*' == *Most_Buf->file) return;

   if (Most_Secure_Mode) Most_D_Opt = 0;
   if (Most_D_Opt == 0) return;

   most_select_minibuffer ();
   SLsmg_printf ("Delete %s? [n]", Most_Buf->file);
   SLsmg_refresh ();

   ch = getkey_nocase ();
   if (ch == 'Y')
     {
	if (!most_delete_file(Most_Buf->file))
	  {
	     msg = "File could not be deleted.";
	     bell = 1;
	  }
	else
	  msg = "File deleted.";
     }
   else
     msg = "File not deleted.";
   most_exit_minibuffer ();
   most_message(msg, bell);
}

void most_toggle_options (void)
{
   char ch;
   char *msg = NULL;
   int n;
   unsigned int save;

   most_select_minibuffer();
   SLsmg_write_string("Toggle option: b(binary) d(selective display) t(tab) v(verbose) w(wrap)");
   SLsmg_gotorc (SLtt_Screen_Rows - 1, 0);
   SLsmg_refresh ();

   ch = getkey_nocase();
   switch (ch)
     {
      default:
	msg = "\007Invalid option!";
	break;

      case 'B':
	Most_B_Opt = !Most_B_Opt;
	Most_Num_Lines = most_count_lines(Most_Beg,Most_Eob);
	break;

      case 'D':
	if (Most_W_Opt)
	  {
	     msg = "\007Selective Display illegal in wrap mode.";
	     break;
	  }

	if (Most_Digit_Arg == NULL)
	  {
	     msg = "Selective Display off.  Prefix with integer to set.";
	     n = 0;
	  }
	else
	  {
	     msg = "Selective Display is set.";
	     n = abs( *Most_Digit_Arg );
	  }

	if (Most_Selective_Display != n)
	  {
	     Most_Selective_Display = n;
	     Most_Num_Lines = most_count_lines(Most_Beg,Most_Eob);
	     most_forward_line (-1);

	     /* This is duplicated below */
	     /* Most_C_Line = most_count_lines (Most_Beg, Most_C_Pos); */
	  }

	break;

      case 'S':

	if (Most_B_Opt) msg = "\007Squeezing not available in Binary mode.";
	else
	  {
	     Most_S_Opt = !Most_S_Opt;
	     Most_Num_Lines = most_count_lines(Most_Beg,Most_Eob);
	     if (Most_S_Opt) msg = "Lines are now squeezed.";
	     else msg = "Line squeezing is turned off";
	  }

	break;

      case 'W':
	if (Most_Selective_Display)
	  {
	     msg = "\007Wrap mode cannot be enabled while selective display is on.";
	     break;
	  }
	Most_W_Opt = !Most_W_Opt;
	Most_Num_Lines = most_count_lines(Most_Beg, Most_Eob);
	if (Most_W_Opt) msg = "Wrap turned on.";
	else msg = "Wrap turned off.";
	break;

      case 'V':
	Most_V_Opt = !Most_V_Opt;
	if (Most_V_Opt) msg = "Control char display is turned on.";
	else msg = "Control char display is turned off.";
	break;

      case 'T':
	if (Most_Digit_Arg == NULL) Most_T_Opt = !Most_T_Opt;
	else
	  {
	     Most_Tab_Width = abs( *Most_Digit_Arg );
	     if (Most_Tab_Width == 0) Most_T_Opt  = 1;
	     else Most_T_Opt = 0;
	  }
	if (Most_T_Opt) msg = "Tabs are nolonger expanded.";
	else msg = "Tabs are now expanded.";
	break;
     }

   SLsmg_erase_eol ();
   most_exit_minibuffer();

   if ((msg != NULL) && (*msg == '\007'))
     {
	most_message (msg + 1, 1);
	return;
     }

   if (!Most_Num_Lines) Most_Num_Lines = 1;
   most_save_win_flags(Most_Win);

   save = Most_C_Offset;
   Most_C_Offset = 0;
   Most_C_Line = 1;
   Most_C_Line = most_what_line (Most_Beg + save);
   Most_C_Offset = save;
   Most_Win->beg_line = Most_C_Line;
   most_redraw_window();
   most_update_status();

   if (msg != NULL) most_message (msg, 0);
}

/* ----------------------------------------------------------------------*/

void most_extended_key_cmd (void)
{
   char ch;

   if (Most_Secure_Mode) Most_D_Opt = 0;

   most_select_minibuffer();
   SLsmg_write_string ("Choose: N (next file), C (toggle case), O (toggle options)");
   if (Most_D_Opt) SLsmg_write_string (" ,D (delete file)");
   SLsmg_gotorc (SLtt_Screen_Rows - 1, 0);
   SLsmg_refresh ();
   ch = getkey_nocase();
   SLsmg_erase_eol ();

   most_exit_minibuffer ();

   switch (ch)
     {
      case 'N':
	most_next_file ();
	break;
      case 'C':
	most_toggle_case ();
	break;
      case 'O':
	most_toggle_options ();
	break;
      case 'D':
	if (Most_D_Opt)
	  {
	     most_delete_file_cmd ();
	     break;
	  }
	/* drop */
      default:
	most_message ("Invalid option.", 1);
     }
}

