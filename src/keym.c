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
#include <ctype.h>

#include <slang.h>
#include "jdmacros.h"

#include "most.h"
#include "keym.h"
#include "display.h"
#include "window.h"
#include "search.h"
#include "edit.h"
#include "sysdep.h"
#include "file.h"
#include "keyparse.h"

int *Most_Digit_Arg;
static char Last_Char;
static int Edt_Direction = 1;

static void digit_arg_cmd(void);

static void page_down_cmd (void)
{
   int n = 1;

   if (Most_Digit_Arg != (int *) NULL) n = *Most_Digit_Arg;
   if (n < 0) n = 1;
   n = Most_C_Line + n * (Most_Win->bot - Most_Win->top + 1);
   most_read_to_line (n + 50);
   most_update_windows (n);
}

static void page_up_cmd (void)
{
   int n = 1;
   if (Most_Digit_Arg != (int *) NULL) n = *Most_Digit_Arg;
   n = Most_C_Line - n * (Most_Win->bot - Most_Win->top + 1);
   if (n < 1) n = 1;
			  
   most_update_windows (n);
}

static void page_right_cmd (void)
{
   int n = 1;
   if (Most_Digit_Arg != (int *) NULL) n = *Most_Digit_Arg;
   Most_Column = Most_Column + n * 59;
   most_update_windows(Most_C_Line);
}

static void page_left_cmd(void)
{
   int n = 1;
   if (Most_Digit_Arg != (int *) NULL) n = *Most_Digit_Arg;
   Most_Column = Most_Column - n * 59;
   most_update_windows(Most_C_Line);
}

static void column_right_cmd (void)
{
   int n = 1;
   if (Most_Digit_Arg != (int *) NULL) n = *Most_Digit_Arg;
   Most_Column = Most_Column + n;
   most_update_windows(Most_C_Line);
}

static void column_left_cmd(void)
{
   int n = 1;
   if (Most_Digit_Arg != (int *) NULL) n = *Most_Digit_Arg;
   Most_Column = Most_Column - n;
   most_update_windows(Most_C_Line);
}

static void next_line_cmd(void)
{
   int n = 1;
   if (Most_Digit_Arg != (int *) NULL) n = *Most_Digit_Arg;

   most_read_to_line (Most_C_Line + n + 50);
   most_update_windows(Most_C_Line + n);
}

static void previous_line_cmd(void)
{
   int n = 1;
   if (Most_Digit_Arg != (int *) NULL) n = *Most_Digit_Arg;

   most_update_windows(Most_C_Line - n);
}

static void tail_mode_cmd (void)
{
   Most_Tail_Mode = 1;

   do
     {
	if (0 != most_read_file_dsc (-1))
	  most_update_windows (-1);

	most_message ("Most Tail Mode--  MOST keys are still active.", 0);
	most_put_message ();

	most_point_cursor ();
	SLsmg_refresh ();
     }
   while (0 == SLang_input_pending (15));
   Most_Tail_Mode = 0;
}

static void top_of_buffer_cmd(void)
{
   most_update_windows(1);
}

static void end_of_buffer_cmd(void)
{
   while ((Most_Buf->fd != -1)
	  && (0 != most_read_file_dsc(-1)))
     ;
   most_update_windows (-1);
}
#ifdef unix
# include <signal.h>
#endif
static void sys_spawn_cmd(void)
{
#ifdef IBMPC_SYSTEM
   most_message ("Not implemented.", 1);
#else
   int update = 0;
# ifndef VMS
   static int can_suspend = -1;

   if (can_suspend == -1)
     {
	can_suspend = 0;
#  ifdef SIGTSTP
	if (SIG_DFL == SLsignal (SIGTSTP, SIG_DFL))
	  can_suspend = 1;
#  endif
     }
   if (can_suspend == 0)
     {
	most_message ("Shell forbids suspension.", 1);
	return;
     }
# endif

   if (Most_Secure_Mode || Most_Captive_Mode)
     {
	most_message ("Operation not permitted by this account.", 1);
	return;
     }
   most_reset_tty();
   most_reset_display();
# ifdef VMS
   if (Last_Char == '\032') exit(0);
   if (most_do_shell_command()) update = 1; /* scroll region reset by message facility */
# else
#  ifdef SIGTSTP
   kill(0, SIGTSTP);
#  endif
   update = 1;
# endif
   most_init_tty();
   most_init_display ();
   if (update) most_redraw_display();
#endif				       /* IBMPC_SYSTEM */
}

static void redraw_cmd(void)
{
   most_redraw_display();
}

static int read_integer (char *prompt, MOST_INT *n)
{
   int status;
   Most_Mini_Buf[0] = 0;
   
   status = most_read_from_minibuffer(prompt, NULL, (char *) Most_Mini_Buf, MOST_MINI_BUF_LEN);
   if (status < 0)
     return -1;

   if (1 != sscanf((char *) Most_Mini_Buf, MOST_INT_D_FMT, n))
     {
	most_message ("Expecting an integer", 1);
	return -1;
     }

   Most_Mini_Buf[0] = 0;
   return 0;
}

static void goto_line_cmd(void)
{
   MOST_INT n;

   if (Most_Digit_Arg != (int *) NULL) n = *Most_Digit_Arg;
   else
     {
	if (-1 == read_integer ("Goto Line: ", &n))
	  return;
     }
   most_update_windows ((int)n);
}

static void goto_percent_cmd(void)
{
   unsigned char *pos;
   MOST_INT n;

   if (Most_Digit_Arg != NULL) n = *Most_Digit_Arg;
   else
     {
	if (-1 == read_integer ("Goto Percent: ", &n))
	  return;
     }

   if (n < 0) n = 0; else if (n > 100) n = 100;
   if (Most_Buf->fd != -1) most_read_file_dsc(-1);
   pos = Most_Beg + (n * (Most_Eob - Most_Beg))/100;
   n = most_what_line(pos);
   most_update_windows (n);
}

static void set_mark_cmd(void)
{
   Most_Buf->mark = Most_C_Line;
   most_message("Mark Set.",0);
}

static void goto_mark_cmd(void)
{
   int mark = Most_Buf->mark;
   Most_Buf->mark = Most_C_Line;
   most_update_window(mark);
   most_message("Mark Set.",0);
}

static void one_window_cmd(void)
{
   most_one_window();
}

static void two_window_cmd(void)
{
   most_two_windows();
}

static void del_window_cmd(void)
{
   most_delete_window();
}

static void other_window_cmd(void)
{
   int n = 1;
   if (Most_Digit_Arg != (int *) NULL) n = *Most_Digit_Arg;
   most_other_window(n);
}

static void find_next_cmd(void)
{
   MOST_INT col, line, n = 1;
   unsigned long ofs;

   if (Most_Digit_Arg != NULL) n = *Most_Digit_Arg;
   line = most_search ((Most_Beg + Most_Curs_Offset) + Most_Search_Dir, n, &col);
   if (line < 1) return;

   ofs = Most_Curs_Offset;

   if ((line < Most_Win->beg_line) || (line > Most_Win->beg_line + Most_Win->bot - Most_Win->top))
     most_update_window(line);

   Most_Curs_Offset = ofs;

   Most_Curs_Row = line - Most_C_Line + 1;
   Most_Curs_Col = col;
}

static void find_next_opposite_dir_cmd (void)
{
   Most_Search_Dir = -Most_Search_Dir;
   find_next_cmd ();
   Most_Search_Dir = -Most_Search_Dir;
}

static void search_cmd_dir (char *prompt, int dir)
{
   char buf[MOST_SEARCH_BUF_LEN];

   if (-1 == most_read_from_minibuffer(prompt, NULL, buf, MOST_SEARCH_BUF_LEN))
     return;

   Most_Search_Dir = dir;
   if (*buf)
     strcpy (Most_Search_Str, buf);    /* no buffer overflow here */
   Most_Curs_Offset = Most_C_Offset;
   find_next_cmd ();
}

static void search_cmd(void)
{
   search_cmd_dir (
#ifdef SLANG_REGEXP
		   "Regexp Search: ",
#else
		   "Search: ",
#endif
		   1);
}

static void search_back_cmd(void)
{
   search_cmd_dir (
#ifdef SLANG_REGEXP
		   "Regexp Search Backwards: ",
#else
		   "Search Backwards: ",
#endif
		   -1);
}

static void help_cmd(void)
{
   most_do_help_command();
}

static void find_file_cmd(void)
{
   most_user_get_file();
}

static void time_cmd(void)
{
   most_message(most_get_time(),0);
}

static void toggle_width_cmd(void)
{
   if (SLtt_Screen_Cols == 80)
     {
	if (!Most_Restore_Width_To) Most_Restore_Width_To = 80;
	most_set_width(132, 1);
     }
   else if (SLtt_Screen_Cols == 132)
     {
	if (!Most_Restore_Width_To) Most_Restore_Width_To = 132;
	most_set_width(80, 1);
     }
}

static void edt_forward_cmd(void)
{
   Edt_Direction = 1;
}

static void edt_back_cmd(void)
{
   Edt_Direction = 0;
}

static void edt_page_cmd(void)
{
   if (Edt_Direction == 0)
     {
	page_up_cmd();
     }
   else
     {
	page_down_cmd();
     }
}

static void edt_line_cmd(void)
{
   if (Edt_Direction == 0)
     {
	previous_line_cmd();
     }
   else
     {
	next_line_cmd();
     }
}

static void edt_find_cmd(void)
{
   if (Edt_Direction == 0)
     {
	search_back_cmd();
     }
   else
     {
	search_cmd();
     }
}

static void edt_find_next_cmd(void)
{
   if (Edt_Direction == 0)
     {
	Most_Search_Dir = -1;
     }
   else
     {
	Most_Search_Dir = 1;
     }
   find_next_cmd();
}

#define A_KEY(s, f)  {s, (int (*)(void)) f}
SLKeymap_Function_Type Most_Functions [] =
{
   A_KEY("edit", most_edit_cmd),
   A_KEY("suspend", sys_spawn_cmd),
   A_KEY("next_file", most_next_file),
   A_KEY("toggle_options", most_toggle_options),
   A_KEY("toggle_lock", most_toggle_lock),
   A_KEY("extended_key", most_extended_key_cmd),
   A_KEY("toggle_case", most_toggle_case),
   A_KEY("delete_file", most_delete_file_cmd),
   A_KEY("bob", top_of_buffer_cmd),
   A_KEY("delete_window", del_window_cmd),
   A_KEY("digit_arg", digit_arg_cmd),
   A_KEY("down", next_line_cmd),
   A_KEY("edt_backward", edt_back_cmd),
   A_KEY("edt_find", edt_find_cmd),
   A_KEY("edt_find_next", edt_find_next_cmd),
   A_KEY("edt_forward", edt_forward_cmd),
   A_KEY("edt_line", edt_line_cmd),
   A_KEY("edt_page", edt_page_cmd),
   A_KEY("eob", end_of_buffer_cmd),
   A_KEY("exit", most_exit_most),
   A_KEY("find_file", find_file_cmd),
   A_KEY("find_next", find_next_cmd),
   A_KEY("find_next_other_dir", find_next_opposite_dir_cmd),
   A_KEY("goto_line", goto_line_cmd),
   A_KEY("goto_mark", goto_mark_cmd),
   A_KEY("goto_percent", goto_percent_cmd),
   A_KEY("help", help_cmd),
   A_KEY("one_window", one_window_cmd),
   A_KEY("other_window", other_window_cmd),
   A_KEY("page_down", page_down_cmd ),
   A_KEY("page_left", page_left_cmd),
   A_KEY("page_right", page_right_cmd ),
   A_KEY("page_up", page_up_cmd ),
   A_KEY("column_left", column_left_cmd ),
   A_KEY("column_right", column_right_cmd ),
   A_KEY("redraw", redraw_cmd),
   A_KEY("search_backward", search_back_cmd),
   A_KEY("search_forward", search_cmd),
   A_KEY("set_mark", set_mark_cmd),
   A_KEY("show_time", time_cmd),
   A_KEY("tail_mode", tail_mode_cmd),
   A_KEY("toggle_width", toggle_width_cmd),
   A_KEY("two_windows", two_window_cmd),
   A_KEY("up", previous_line_cmd),
     {(char *) NULL, NULL}
};

SLKeyMap_List_Type *Most_Keymap;

#ifdef IBMPC_SYSTEM
static void gobble_mouse_cmd(void)
{
   SLang_flush_input ();
}
#endif

void most_init_keymaps (void)
{
   char  *err = "Unable to create keymaps!";
   char esc[3], gold[5], dig[2];
   int i;

   if (NULL == (Most_Keymap = SLang_create_keymap ("Most", NULL)))
     most_exit_error (err);

   Most_Keymap->functions = Most_Functions;

   esc[0] = 27; esc[2] = 0;
   gold[0] = 27; gold[1] = 'O'; gold[2] = 'P'; gold[4] = 0;
   dig[1] = 0;

   for (i = '0'; i <= '9'; i++)
     {
	dig[0] = (char) i;
	esc[1] = (char) i;
	gold[3] = (char) i;
	SLkm_define_key (dig, (FVOID_STAR) digit_arg_cmd, Most_Keymap);
	SLkm_define_key (esc, (FVOID_STAR) digit_arg_cmd, Most_Keymap);
	SLkm_define_key (gold, (FVOID_STAR) digit_arg_cmd, Most_Keymap);
     }

   dig[0] = '-'; esc[1] = '-'; gold[3] = '-';
   SLkm_define_key (dig, (FVOID_STAR) digit_arg_cmd, Most_Keymap);
   SLkm_define_key (esc, (FVOID_STAR) digit_arg_cmd, Most_Keymap);
   SLkm_define_key (gold, (FVOID_STAR) digit_arg_cmd, Most_Keymap);

   SLkm_define_key (" ", (FVOID_STAR) page_down_cmd, Most_Keymap);
   SLkm_define_key ("$", (FVOID_STAR) sys_spawn_cmd, Most_Keymap);
   SLkm_define_key ("%", (FVOID_STAR) goto_percent_cmd, Most_Keymap);
   SLkm_define_key (",", (FVOID_STAR) goto_mark_cmd, Most_Keymap);
   SLkm_define_key ("/", (FVOID_STAR) search_cmd, Most_Keymap);
   SLkm_define_key (":", (FVOID_STAR) most_extended_key_cmd, Most_Keymap);
   SLkm_define_key ("<", (FVOID_STAR) page_left_cmd, Most_Keymap);
   SLkm_define_key (">", (FVOID_STAR) page_right_cmd, Most_Keymap);
   SLkm_define_key ("?", (FVOID_STAR) search_back_cmd, Most_Keymap);
   SLkm_define_key ("B", (FVOID_STAR) end_of_buffer_cmd, Most_Keymap);
   SLkm_define_key ("D", (FVOID_STAR) page_down_cmd, Most_Keymap);
   SLkm_define_key ("E", (FVOID_STAR) most_edit_cmd, Most_Keymap);
   SLkm_define_key ("F", (FVOID_STAR) tail_mode_cmd, Most_Keymap);
   SLkm_define_key ("G", (FVOID_STAR) goto_line_cmd, Most_Keymap);
   SLkm_define_key ("H", (FVOID_STAR) help_cmd, Most_Keymap);
   SLkm_define_key ("J", (FVOID_STAR) goto_line_cmd, Most_Keymap);
   SLkm_define_key ("L", (FVOID_STAR) most_toggle_lock, Most_Keymap);
   SLkm_define_key ("M", (FVOID_STAR) set_mark_cmd, Most_Keymap);
   SLkm_define_key ("n", (FVOID_STAR) find_next_cmd, Most_Keymap);
   SLkm_define_key ("N", (FVOID_STAR) find_next_opposite_dir_cmd, Most_Keymap);
   SLkm_define_key ("O", (FVOID_STAR) other_window_cmd, Most_Keymap);
   SLkm_define_key ("Q", (FVOID_STAR) most_exit_most, Most_Keymap);
   SLkm_define_key ("R", (FVOID_STAR) redraw_cmd, Most_Keymap);
   SLkm_define_key ("S", (FVOID_STAR) search_cmd, Most_Keymap);
   SLkm_define_key ("T", (FVOID_STAR) top_of_buffer_cmd, Most_Keymap);
   SLkm_define_key ("U", (FVOID_STAR) page_up_cmd , Most_Keymap);
   SLkm_define_key ("V", (FVOID_STAR) next_line_cmd, Most_Keymap);
   SLkm_define_key ("W", (FVOID_STAR) toggle_width_cmd, Most_Keymap);
   SLkm_define_key ("X", (FVOID_STAR) most_exit_most, Most_Keymap);
   SLkm_define_key ("\033$", (FVOID_STAR) sys_spawn_cmd, Most_Keymap);
   SLkm_define_key ("\033<", (FVOID_STAR) top_of_buffer_cmd, Most_Keymap);
   SLkm_define_key ("\033>", (FVOID_STAR) end_of_buffer_cmd, Most_Keymap);
#if !defined(IBMPC_SYSTEM)
   SLkm_define_key ("\033OA", (FVOID_STAR) previous_line_cmd, Most_Keymap);
   SLkm_define_key ("\033OB", (FVOID_STAR) next_line_cmd, Most_Keymap);
   SLkm_define_key ("\033OC", (FVOID_STAR) column_right_cmd, Most_Keymap);
   SLkm_define_key ("\033OD", (FVOID_STAR) column_left_cmd, Most_Keymap);
   SLkm_define_key ("\033OPE", (FVOID_STAR) most_exit_most, Most_Keymap);
   SLkm_define_key ("\033OPO", (FVOID_STAR) one_window_cmd, Most_Keymap);
   SLkm_define_key ("\033OPQ", (FVOID_STAR) most_exit_most, Most_Keymap);
   SLkm_define_key ("\033OPS", (FVOID_STAR) sys_spawn_cmd, Most_Keymap);
   SLkm_define_key ("\033OPV", (FVOID_STAR) del_window_cmd, Most_Keymap);
   SLkm_define_key ("\033OPX", (FVOID_STAR) two_window_cmd, Most_Keymap);
   SLkm_define_key ("\033OP\033OR", (FVOID_STAR) edt_find_cmd, Most_Keymap);
   SLkm_define_key ("\033OP\033On", (FVOID_STAR) goto_mark_cmd, Most_Keymap);
   SLkm_define_key ("\033OP\033Ot", (FVOID_STAR) end_of_buffer_cmd, Most_Keymap);
   SLkm_define_key ("\033OP\033Ou", (FVOID_STAR) top_of_buffer_cmd, Most_Keymap);
   SLkm_define_key ("\033OP\033[A", (FVOID_STAR) other_window_cmd, Most_Keymap);
   SLkm_define_key ("\033OP\033[B", (FVOID_STAR) other_window_cmd, Most_Keymap);
   SLkm_define_key ("\033OP\033[C", (FVOID_STAR) page_right_cmd, Most_Keymap);
   SLkm_define_key ("\033OP\033[D", (FVOID_STAR) page_left_cmd, Most_Keymap);
   SLkm_define_key ("\033OQ", (FVOID_STAR) help_cmd, Most_Keymap);
   SLkm_define_key ("\033OR", (FVOID_STAR) edt_find_next_cmd, Most_Keymap);
   SLkm_define_key ("\033On", (FVOID_STAR) set_mark_cmd, Most_Keymap);
   SLkm_define_key ("\033Op", (FVOID_STAR) edt_line_cmd, Most_Keymap);
   SLkm_define_key ("\033Ot", (FVOID_STAR) edt_forward_cmd, Most_Keymap);
   SLkm_define_key ("\033Ou", (FVOID_STAR) edt_back_cmd, Most_Keymap);
   SLkm_define_key ("\033Ox", (FVOID_STAR) edt_page_cmd, Most_Keymap);
   SLkm_define_key ("\033V", (FVOID_STAR) page_up_cmd , Most_Keymap);
   SLkm_define_key ("\033[1~", (FVOID_STAR) search_cmd, Most_Keymap);
   SLkm_define_key ("\033[2~", (FVOID_STAR) goto_mark_cmd, Most_Keymap);
   SLkm_define_key ("\033[28~", (FVOID_STAR) help_cmd, Most_Keymap);
   SLkm_define_key ("\033[4~", (FVOID_STAR) set_mark_cmd, Most_Keymap);
   SLkm_define_key ("\033[5~", (FVOID_STAR) page_up_cmd , Most_Keymap);
   SLkm_define_key ("\033[6~", (FVOID_STAR) page_down_cmd, Most_Keymap);
   SLkm_define_key ("\033[A", (FVOID_STAR) previous_line_cmd, Most_Keymap);
   SLkm_define_key ("\033[B", (FVOID_STAR) next_line_cmd, Most_Keymap);
   SLkm_define_key ("\033[C", (FVOID_STAR) column_right_cmd, Most_Keymap);
   SLkm_define_key ("\033[D", (FVOID_STAR) column_left_cmd, Most_Keymap);
   SLkm_define_key ("^@", (FVOID_STAR) set_mark_cmd, Most_Keymap);
#else
   SLkm_define_key ("^@H", (FVOID_STAR) previous_line_cmd, Most_Keymap);
   SLkm_define_key ("^@P", (FVOID_STAR) next_line_cmd, Most_Keymap);
   SLkm_define_key ("^@M", (FVOID_STAR) column_right_cmd, Most_Keymap);
   SLkm_define_key ("^@K", (FVOID_STAR) column_left_cmd, Most_Keymap);
   SLkm_define_key ("^@Q", (FVOID_STAR) page_down_cmd, Most_Keymap);
   SLkm_define_key ("^@I", (FVOID_STAR) page_up_cmd, Most_Keymap);
   SLkm_define_key ("^@G", (FVOID_STAR) top_of_buffer_cmd, Most_Keymap);
   SLkm_define_key ("^@O", (FVOID_STAR) end_of_buffer_cmd, Most_Keymap);

   SLkm_define_key ("\xE0H", (FVOID_STAR) previous_line_cmd, Most_Keymap);
   SLkm_define_key ("\xE0P", (FVOID_STAR) next_line_cmd, Most_Keymap);
   SLkm_define_key ("\xE0M", (FVOID_STAR) column_right_cmd, Most_Keymap);
   SLkm_define_key ("\xE0K", (FVOID_STAR) column_left_cmd, Most_Keymap);
   SLkm_define_key ("\xE0Q", (FVOID_STAR) page_down_cmd, Most_Keymap);
   SLkm_define_key ("\xE0I", (FVOID_STAR) page_up_cmd, Most_Keymap);
   SLkm_define_key ("\xE0G", (FVOID_STAR) top_of_buffer_cmd, Most_Keymap);
   SLkm_define_key ("\xE0O", (FVOID_STAR) end_of_buffer_cmd, Most_Keymap);
   SLkm_define_key ("\033[M", (FVOID_STAR) gobble_mouse_cmd, Most_Keymap);
#endif
   SLkm_define_key ("\t", (FVOID_STAR) page_right_cmd, Most_Keymap);
   SLkm_define_key ("^", (FVOID_STAR) previous_line_cmd, Most_Keymap);
   SLkm_define_key ("^?", (FVOID_STAR) page_up_cmd , Most_Keymap);
   SLkm_define_key ("^D", (FVOID_STAR) page_down_cmd, Most_Keymap);
   SLkm_define_key ("^F", (FVOID_STAR) search_cmd, Most_Keymap);
   SLkm_define_key ("^H", (FVOID_STAR) help_cmd, Most_Keymap);
   SLkm_define_key ("^KE", (FVOID_STAR) most_exit_most, Most_Keymap);
   SLkm_define_key ("^KG", (FVOID_STAR) find_file_cmd, Most_Keymap);
   SLkm_define_key ("^K^B", (FVOID_STAR) set_mark_cmd, Most_Keymap);
   SLkm_define_key ("^K^J", (FVOID_STAR) goto_mark_cmd, Most_Keymap);
   SLkm_define_key ("^K^M", (FVOID_STAR) goto_mark_cmd, Most_Keymap);
   SLkm_define_key ("^L", (FVOID_STAR) redraw_cmd, Most_Keymap);
   SLkm_define_key ("^M", (FVOID_STAR) next_line_cmd, Most_Keymap);
   SLkm_define_key ("^N", (FVOID_STAR) next_line_cmd, Most_Keymap);
   SLkm_define_key ("^P", (FVOID_STAR) previous_line_cmd, Most_Keymap);
   SLkm_define_key ("^R", (FVOID_STAR) redraw_cmd, Most_Keymap);
   SLkm_define_key ("^T", (FVOID_STAR) time_cmd, Most_Keymap);
   SLkm_define_key ("^U", (FVOID_STAR) page_up_cmd , Most_Keymap);
   SLkm_define_key ("^V", (FVOID_STAR) page_down_cmd, Most_Keymap);
   SLkm_define_key ("^W0", (FVOID_STAR) del_window_cmd, Most_Keymap);
   SLkm_define_key ("^W1", (FVOID_STAR) one_window_cmd, Most_Keymap);
   SLkm_define_key ("^W2", (FVOID_STAR) two_window_cmd, Most_Keymap);
   SLkm_define_key ("^WO", (FVOID_STAR) other_window_cmd, Most_Keymap);
   SLkm_define_key ("^X0", (FVOID_STAR) del_window_cmd, Most_Keymap);
   SLkm_define_key ("^X1", (FVOID_STAR) one_window_cmd, Most_Keymap);
   SLkm_define_key ("^X2", (FVOID_STAR) two_window_cmd, Most_Keymap);
   SLkm_define_key ("^XO", (FVOID_STAR) other_window_cmd, Most_Keymap);
   SLkm_define_key ("^X^C", (FVOID_STAR) most_exit_most, Most_Keymap);
   SLkm_define_key ("^X^F", (FVOID_STAR) find_file_cmd, Most_Keymap);
   SLkm_define_key ("^Z", (FVOID_STAR) sys_spawn_cmd, Most_Keymap);
   SLkm_define_key ("b", (FVOID_STAR) end_of_buffer_cmd, Most_Keymap);
   SLkm_define_key ("d", (FVOID_STAR) page_down_cmd, Most_Keymap);
   SLkm_define_key ("e", (FVOID_STAR) most_edit_cmd, Most_Keymap);
   SLkm_define_key ("f", (FVOID_STAR) search_cmd, Most_Keymap);
   SLkm_define_key ("g", (FVOID_STAR) goto_line_cmd, Most_Keymap);
   SLkm_define_key ("j", (FVOID_STAR) goto_line_cmd, Most_Keymap);
   SLkm_define_key ("l", (FVOID_STAR) most_toggle_lock, Most_Keymap);
   SLkm_define_key ("m", (FVOID_STAR) set_mark_cmd, Most_Keymap);
   SLkm_define_key ("n", (FVOID_STAR) find_next_cmd, Most_Keymap);
   SLkm_define_key ("o", (FVOID_STAR) other_window_cmd, Most_Keymap);
   SLkm_define_key ("q", (FVOID_STAR) most_exit_most, Most_Keymap);
   SLkm_define_key ("r", (FVOID_STAR) redraw_cmd, Most_Keymap);
   SLkm_define_key ("s", (FVOID_STAR) search_cmd, Most_Keymap);
   SLkm_define_key ("t", (FVOID_STAR) top_of_buffer_cmd, Most_Keymap);
   SLkm_define_key ("v", (FVOID_STAR) next_line_cmd, Most_Keymap);
   SLkm_define_key ("w", (FVOID_STAR) toggle_width_cmd, Most_Keymap);
   SLkm_define_key ("x", (FVOID_STAR) most_exit_most, Most_Keymap);
   if (SLang_get_error()) most_exit_error (err);

   (void) most_load_user_keymaps ();
}

static void sldo_key (void)
{
   SLang_Key_Type *key;

   key = SLang_do_key (Most_Keymap, most_getkey);
   SLKeyBoard_Quit = 0;
   SLang_set_error (0);
   Last_Char = SLang_Last_Key_Char;

   if ((key == NULL) || (key->f.f == NULL) || (key->type != SLKEY_F_INTRINSIC))
     {
	SLtt_beep ();
     }
   else (((void (*)(void))(key->f.f)) ());
}

static void digit_arg_cmd(void)
{
   char num[15], ch;
   int j = 0;
   static int digits;

   num[j++] = Last_Char;
   ch = most_getkey();
   while ((ch >= '0') && (ch <= '9'))
     {
	if (j == 15) return;

	num[j++] = ch;
	ch = most_getkey();
     }

   if (((j == 1) && (Last_Char != '-')) || (j > 1))
     {
	num[j] = '\0';
	sscanf(num,"%d",&digits);
	Most_Digit_Arg = &digits;
     }
   Last_Char = ch;
   SLang_ungetkey (ch);
   sldo_key ();
}

void most_execute_key (void)
{
   static int refresh_pending = 0;

   if (Most_Want_Exit) return;

   most_check_minibuffer ();
   if (Most_Mini_Buf[0] != '\0') most_put_message ();
   Most_Digit_Arg = (int *) NULL;

   most_point_cursor ();

   if (SLang_input_pending (0))
     refresh_pending++;
   else
     refresh_pending = 0;

   if ((refresh_pending == 0) || (refresh_pending > 4))
     {
	refresh_pending = 0;
	SLsmg_refresh ();
     }

   sldo_key ();
}
