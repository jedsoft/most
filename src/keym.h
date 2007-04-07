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
extern void most_init_keymaps (void);
extern void most_execute_key (void);
extern int *Most_Digit_Arg;
extern void most_do_help_command (void);

/* These are in  cmd.c */
extern void most_extended_key_cmd(void);
extern void most_next_file (void);
extern void most_toggle_case (void);
extern void most_delete_file_cmd (void);
extern void most_toggle_options (void);

#if 0
extern void ctrl_x_map_cmd(void);
extern void ctrl_k_map_cmd(void);
extern void ctrl_w_map_cmd(void);
extern void pf1_map_cmd(void);
extern void pf1_esc_map_cmd(void);
extern void dec_extended_map_cmd(void);
extern void esc_map_cmd(void);
extern void page_down_cmd(void);
extern void search_cmd(void);
extern void next_line_cmd(void);
extern void previous_line_cmd(void);
extern void extended_cmd_cmd(void);
extern void redraw_cmd(void);
extern void goto_line_cmd(void);
extern void time_cmd(void);
extern void page_up_cmd(void);
extern void page_up_cmd(void);
extern void column_left_cmd(void);
extern void column_right_cmd(void);
extern void page_right_cmd(void);
extern void sys_spawn_cmd(void);
extern void set_mark_cmd(void);
extern void top_of_buffer_cmd(void);
extern void goto_mark_cmd(void);
extern void search_back_cmd(void);
extern void find_next_cmd(void);
extern void end_of_buffer_cmd(void);
extern void exit_cmd(void);
extern void one_window_cmd(void);
extern void two_window_cmd(void);
extern void del_window_cmd(void);
extern void other_window_cmd(void);
extern void O_map_cmd(void);
extern void find_file_cmd(void);
extern void digit_arg_cmd(void);
extern void edit_cmd(void);
extern void toggle_width_cmd(void);
extern void goto_percent_cmd(void);
extern void edt_page_cmd(void);
extern void edt_forward_cmd(void);
extern void edt_back_cmd(void);
extern void edt_line_cmd(void);
extern void edt_find_cmd(void);
extern void edt_find_next_cmd(void);

extern int do_extended_key(void);
extern int do_extended_cmd(void);
extern void do_help_command(void);
extern void execute_key(void);

#endif
