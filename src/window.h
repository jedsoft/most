#ifndef _DAVIS_WINDOW_H_
# define _DAVIS_WINDOW_H_
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
# include "buffer.h"

typedef struct _Most_Window_Type
{
   unsigned long top_offset; /* offset of top line of window from bob */
   int beg_line;             /* line number of top */
   int curs_line;            /* line number of curs pos */
   int curs_col;             /* column number of curs pos */
   unsigned long curs_offset;  /* pos of cursor from bob*/
   int col;                  /* column offset */
   int n_lines;              /* number of lines of buffer (mode dependent) */
   int top;                  /* screen location of top */
   int bot;                  /* screen location of bot */
   int display;              /* value of MOST_S_OPT for selective display */
#if 0
   char *status;             /* status line */
#endif
   Most_Buffer_Type *buf;              /* buffer structure in window */
   struct _Most_Window_Type *next; /* next window */
   struct _Most_Window_Type *prev; /* prev window */
   int flags;                /* Squeeze lines, etc.. */
   int lock;                 /* true if locked */
   int dirty_flag;
}
Most_Window_Type;

extern Most_Window_Type *Most_Win;
extern Most_Window_Type *Most_Top_Win;
extern unsigned long Most_Curs_Offset;
extern int Most_Column;
extern int Most_Curs_Row;
extern int Most_Curs_Col;
#define MOST_MINI_BUF_LEN	256
extern char Most_Mini_Buf[MOST_MINI_BUF_LEN];
extern int Most_Selective_Display;
extern int Most_Restore_Width_To;
extern int Most_Display_Inited;

extern int most_split_window (void);
extern void most_toggle_lock(void);
extern void most_update_window(int); /* moves window to have arg lines on the top */
extern void most_update_status(void);
extern void most_redraw_window(void);    /* redraws window updating the structure */
extern void most_window_buffer(void);
extern void most_init_display(void);
extern void most_reset_display(void);
extern void most_other_window(int);
extern void most_set_window (Most_Window_Type *);
extern void most_update_windows(int);
extern void most_message(char *, int);
extern void most_put_message(void);
extern void most_clear_minibuffer(void);
extern void most_check_minibuffer(void);
extern void most_free_window_buffer(void);
extern void most_select_minibuffer(void);
extern void most_exit_minibuffer(void);
extern int most_read_from_minibuffer(char *, char *, char *, unsigned int);
extern void most_redraw_display(void);
extern void most_one_window(void);
extern void most_two_windows(void);
extern void most_delete_window(void);
extern void most_save_win_flags(Most_Window_Type *);
extern void most_free_windows (void);
extern void most_flush_message (char *);
extern void most_point_cursor (void);
#endif

