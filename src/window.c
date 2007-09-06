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

#ifndef VMS
# include <sys/types.h>
#endif

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <slang.h>
#include "jdmacros.h"

#include "window.h"
#include "search.h"
#include "display.h"
#include "most.h"
#include "sysdep.h"
#include "file.h"
#include "line.h"

char *Most_Global_Msg = "Press `Q' to quit, `H' for help, and SPACE to scroll.";

Most_Window_Type *Most_Win;
Most_Window_Type *Most_Top_Win;
int Most_Top_Line;		       /* row number of top window */
int Most_Curs_Row;
int Most_Curs_Col;
int Most_Column = 1;
int Most_Restore_Width_To = 0;
char Most_Mini_Buf[256];
unsigned long Most_Curs_Offset;

static int Beep_Mini = 0;
static int Minibuffer_Selected;

void most_message(char *what, int how)
{
   strcpy (Most_Mini_Buf,what);
   if (how) Beep_Mini = 1; else Beep_Mini = 0;
}

static char *Last_Message;
void most_select_minibuffer()
{
   SLsmg_gotorc (SLtt_Screen_Rows - 1, 0);
   SLsmg_erase_eol ();
   /* SLsmg_refresh (); */
   Last_Message = NULL;
   Minibuffer_Selected = 1;
}

void most_exit_minibuffer()
{
#if 0
   SLsmg_gotorc (SLtt_Screen_Rows - 1, 0);
   SLsmg_erase_eol ();
#endif
   Minibuffer_Selected = 0;
}

static void put_message_1 (char *what)
{
   most_select_minibuffer ();
   SLsmg_write_string (what);
   most_exit_minibuffer();
   Last_Message = what;
}

void most_flush_message (char *msg)
{
   SLsmg_gotorc (SLtt_Screen_Rows - 1, 0);
   SLsmg_write_string (msg);
   SLsmg_erase_eol ();
   SLsmg_refresh ();
}

void most_put_message ()
{
   if (Beep_Mini) SLtt_beep();

   put_message_1 ((char *) Most_Mini_Buf);

   if (Beep_Mini)
     {
	SLsmg_refresh ();
	/* wait half a second */
	if (SLang_input_pending (5))
	  {
	     (void) most_getkey ();
	     SLKeyBoard_Quit = 0;
	     SLang_set_error (0);
	  }
     }
   Beep_Mini = 0;
   *Most_Mini_Buf = 0;
}

/* puts 'what in the minibuffer to be edited. */
/* returns number of chars read */

SLang_RLine_Info_Type *Most_RLI;

#if SLANG_VERSION < 20000
typedef SLang_RLine_Info_Type SLrline_Type;

static void rline_update (unsigned char *buf, int len, int col)
{
   SLsmg_gotorc (SLtt_Screen_Rows - 1, 0);
   SLsmg_write_nchars ((char *) buf, len);
   SLsmg_erase_eol ();
   SLsmg_gotorc (SLtt_Screen_Rows - 1, col);
   SLsmg_refresh ();
}
#else
static void rline_update (SLrline_Type *rli, char *prompt, 
			  char *buf, unsigned int len, unsigned int point,
			  VOID_STAR client_data)
{
   int col;

   (void) client_data;
   (void) rli;
   
   while (1)
     {
	SLsmg_gotorc (SLtt_Screen_Rows - 1, 0);
	SLsmg_write_string (prompt);
	SLsmg_write_nchars (buf, point);
	col = SLsmg_get_column ();
	SLsmg_write_nchars (buf + point, len - point);
	if (col < SLtt_Screen_Cols)
	  break;
	buf++; point--; len--;      /* FIXME for UTF-8 */
     }
   SLsmg_erase_eol ();
   SLsmg_gotorc (SLtt_Screen_Rows - 1, col);
   SLsmg_refresh ();
}
#endif

#if SLANG_VERSION < 20000
static SLang_RLine_Info_Type  *init_readline (void)
{
   unsigned char *buf = NULL;
   SLang_RLine_Info_Type *rli;

   if ((NULL == (rli = (SLang_RLine_Info_Type *) SLMALLOC (sizeof(SLang_RLine_Info_Type))))
       || (NULL == (buf = (unsigned char *) SLMALLOC (256))))
     {
	fprintf(stderr, "malloc error.\n");
	exit(11);
     }

   SLMEMSET ((char *) rli, 0, sizeof (SLang_RLine_Info_Type));
   rli->buf = buf;
   rli->buf_len = 255;
   rli->tab = 8;
   rli->dhscroll = 20;
   rli->getkey = most_getkey;
   rli->tt_goto_column = NULL;
   rli->update_hook = rline_update;

   if (SLang_init_readline (rli) < 0)
     {
	most_exit_error ("Unable to initialize readline library.");
     }

   return rli;
}

static char *SLrline_read_line (SLrline_Type *rli, char *prompt, unsigned int *lenp)
{
   int i;
   
   rli->prompt = prompt;
   i = SLang_read_line (rli);
   if (i < 0)
     {
	*lenp = 0;
	return NULL;
     }
   *lenp = i;
   return SLmake_string ((char *)rli->buf);
}

static int SLrline_set_line (SLrline_Type *rli, char *what)
{
   strcpy ((char *)rli->buf, what);
   return 0;
}
#define SLrline_save_line SLang_rline_save_line

static int SLrline_set_point (SLrline_Type *rli, int point)
{
   rli->point = point;
   return 0;
}

#else
static SLang_RLine_Info_Type  *init_readline (void)
{
   SLang_RLine_Info_Type *rli;
   unsigned int flags = SL_RLINE_BLINK_MATCH;

   if (Most_UTF8_Mode)
     flags |= SL_RLINE_UTF8_MODE;

   if (NULL == (rli = SLrline_open (SLtt_Screen_Cols, flags)))
     return NULL;

   (void) SLrline_set_update_hook (rli, rline_update, NULL);
   return rli;
}
#endif

int most_read_from_minibuffer(char *prompt, char *stuff, char *what, unsigned int buflen)
{
   int i = 0;
   char *buf;
   unsigned int len;

   if (Most_RLI == NULL)
     {
	Most_RLI = init_readline ();
     }
#if SLANG_VERSION < 20000
   Most_RLI->edit_width = SLtt_Screen_Cols - 1;
   Most_RLI->prompt = prompt;
   *Most_RLI->buf = 0;
#else
   SLrline_set_display_width (Most_RLI, SLtt_Screen_Cols);
#endif

   most_select_minibuffer();

   /* do not use default.  The up arrow can always get it back. */
   if (stuff != NULL)
     {
	if (-1 == SLrline_set_line (Most_RLI, stuff))
	  return -1;

	SLrline_set_point (Most_RLI, strlen (stuff));
     }

   buf = SLrline_read_line (Most_RLI, prompt, &len);

   if ((buf != NULL) && (0 == SLang_get_error()) && !SLKeyBoard_Quit)
     {
	SLrline_save_line (Most_RLI);
	strncpy(what, (char *) buf, buflen);
	what[buflen-1] = 0;
     }
   SLfree (buf);

   if (SLKeyBoard_Quit) i = -1;
   SLang_set_error (0);
   SLKeyBoard_Quit = 0;

   most_exit_minibuffer();

   if (i == -1) most_message ("Aborted!", 1);

   return i;
}

void most_clear_minibuffer()
{
   Most_Mini_Buf[0] = '\0';
   Beep_Mini = 0;
   most_put_message ();
}

void most_check_minibuffer ()
{
   if (*Most_Mini_Buf) most_put_message ();
   else
     if (Last_Message != Most_Global_Msg)
       put_message_1 (Most_Global_Msg);
}

static int get_scroll(int *line)
{
    /* line is the line we want at the top of the window if possible */
   int dtop, dbot,n,top,bot,wsize;

   top = Most_Win->beg_line;
   wsize = Most_Win->bot - Most_Win->top; /* actually 1 less than window size */
   bot = top + wsize;

   if ((*line == 1) && (top == 1))
     {
	Most_Curs_Col = Most_Curs_Row = 1;
	if (Most_C_Offset != 0)
	  {
	     Most_C_Offset = 0;
	     Most_C_Line = 1;
	     return 0;
	  }
	most_message("Top of Buffer.",0);
	return(0);
     }

    /* handles endof file in a window */
   if ((bot > Most_Num_Lines) && *line > Most_C_Line)
     {
	*line = top;
	most_message("End of Buffer.",1);
	return(0);
     }

   if (Most_Num_Lines <= wsize)     /* short file */
     {
	*line = 1;
	return(0);
     }

   dtop = top - 1;
   dbot = Most_Num_Lines - bot;
   n = *line - top;

   if ((n>0) && (dbot < n))
     {
	n = dbot;
	*line = top + n;
	if (!n) most_message("End of buffer.",1);
     }
   else if ((n < 0) && (dtop + n < 0))
     {
	n = -dtop;
	*line = n + top;
	if (!n)
	  {
	     Most_Curs_Col = Most_Curs_Row = 1;
	     if (Most_C_Offset != 0)
	       {
		  Most_C_Offset = 0;
		  Most_C_Line = 1;
		  return 0;
	       }
	     most_message("Top of buffer.",0);
	  }
     }
   return(n);
}

static void check_dirty_flag (void)
{
   if (Most_Win->dirty_flag == 0) return;

   Most_Win->dirty_flag = 0;
   /* 
   Most_Num_Lines = most_count_lines (Most_Beg, Most_Eob);
    */
   Most_Win->n_lines = Most_Num_Lines;
}

void most_update_window (int line)
{
   int save_line, save_col;
   unsigned long save_offset;

   most_read_to_line (line);
   check_dirty_flag ();

   if (Most_Column != Most_Win->col)
     {
	if (Most_Column < 1) Most_Column = 1;
	if (Most_Column != Most_Win->col)
	  {
	     save_offset = Most_Curs_Offset;
	     save_line = Most_Curs_Row;
	     save_col = Most_Curs_Col;
	     most_redraw_window ();
	     most_update_status ();
	     Most_Win->curs_offset = Most_Curs_Offset = save_offset;
	     Most_Win->curs_line = Most_Curs_Row = save_line;
	     Most_Win->curs_col = Most_Curs_Col = save_col;
	  }
	return;
     }

   (void) get_scroll (&line);

   most_goto_line (line);
   most_redraw_window ();
   most_update_status ();
}

/* updates current window as well as scroll lock ones */
/* Although current window is update in an absolute fashion, scroll locked
   ones are updated in a relative fashion */
void most_update_windows (int line)
{
   int dline,flg;
   Most_Window_Type *w;

   if (line == -1)
     {
	check_dirty_flag ();
	line = Most_Num_Lines;
     }

   dline = line - Most_C_Line;
   most_update_window(line);
   if (!Most_Win->lock) return;
   flg = 0;
   w = Most_Win;
   while(Most_Win = Most_Win->next, Most_Win != w)
     {
	if (Most_Win->lock)
	  {
	     flg = 1;
	     most_set_window(Most_Win);
	     line = Most_C_Line + dline;
	     most_update_window(line);
	  }
     }
   Most_Win = w;
   if (flg) most_set_window(Most_Win);
}

void most_redraw_window (void)
{
   int n, r;

   check_dirty_flag ();

   n = Most_Win->bot - Most_Win->top;

   if ((Most_C_Line + n) > Most_Num_Lines)
     most_goto_line (Most_Num_Lines - n);

   Most_Curs_Offset = Most_C_Offset;
   Most_Win->curs_offset = Most_Win->top_offset = Most_Curs_Offset;
   Most_Win->beg_line = Most_C_Line;
   Most_Win->col = Most_Column;
   Most_Win->curs_col = Most_Curs_Col = 1;
   Most_Win->curs_line = Most_Curs_Row = 1;

   r = Most_Win->top;
   most_goto_rc (r, 1);

   most_display_line ();

   while (n--)
     {
	r++;
	most_goto_rc (r, 1);

	if (most_forward_line(1))
	  most_display_line();
	else
	  SLsmg_erase_eol ();
     }

   Most_C_Offset = Most_Win->top_offset;
   Most_C_Line = Most_Win->beg_line;
}

#define MOST_V_OPT  1
#define MOST_T_OPT  2
#define MOST_B_OPT  4
#define MOST_S_OPT 8
#define MOST_W_OPT 16

/* associates current window with current buffer */
void most_save_win_flags(Most_Window_Type *w)
{
   w->flags = 0;
   if (Most_V_Opt) w->flags |= MOST_V_OPT;
   if (Most_B_Opt) w->flags |= MOST_B_OPT;
   if (Most_T_Opt) w->flags |= MOST_T_OPT;
   if (Most_W_Opt) w->flags |= MOST_W_OPT;
   if (Most_S_Opt) w->flags |= MOST_S_OPT;
   w->n_lines = Most_Num_Lines;
   w->display = Most_Selective_Display;
}

void most_window_buffer()
{
   Most_Win->beg_line = Most_C_Line;
   Most_Win->top_offset = Most_C_Offset;
   Most_Win->col = Most_Column;
   Most_Win->buf = Most_Buf;
   Most_Selective_Display = 0;
   most_save_win_flags(Most_Win);

   Most_Curs_Row = Most_Win->curs_line = 1;
   Most_Curs_Col = Most_Win->curs_col = 1;
   Most_Curs_Offset = Most_Win->curs_offset = Most_Win->top_offset;
}

static void restore_win_flags (void)
{
   Most_V_Opt = Most_Win->flags & MOST_V_OPT;
   Most_B_Opt = Most_Win->flags & MOST_B_OPT;
   Most_T_Opt = Most_Win->flags & MOST_T_OPT;
   Most_W_Opt = Most_Win->flags & MOST_W_OPT;
   Most_S_Opt = Most_Win->flags & MOST_S_OPT;
   Most_Num_Lines = Most_Win->n_lines;
   Most_Selective_Display = Most_Win->display;
}

static Most_Window_Type *make_window(int r1,int r2)
{
   Most_Window_Type *neew;

   neew = (Most_Window_Type *) MOSTMALLOC(sizeof(Most_Window_Type));
   SLMEMSET ((char *) neew, 0, sizeof (Most_Window_Type));
#if 0
   neew->status = (char *) MOSTMALLOC(256);
   SLMEMSET (neew->status, 0, SLtt_Screen_Cols + 1);
#endif
   neew->col = Most_Column;
   neew->top = r1;
   neew->bot = r2;
   neew->lock = 0;
   neew->buf = NULL;
   most_save_win_flags(neew);
   neew->dirty_flag = 1;
   return(neew);
}

int Most_Display_Inited = 0;

static void error_hook (char *msg)
{
   most_message (msg, 1);
}

void most_init_display ()
{
   int h = SLtt_Screen_Rows;

   if (Most_Display_Inited) return;
   Most_Display_Inited = 1;

   most_get_term_dimensions(&SLtt_Screen_Cols, &SLtt_Screen_Rows);

   if (Most_Win == NULL)
     {
	Most_Top_Win = Most_Win = make_window(1,SLtt_Screen_Rows - 2);
	Most_Win->prev = Most_Win->next = Most_Win;
     }
   else if (h > SLtt_Screen_Rows) most_resize_display ();
   else Most_Top_Win->prev->bot = SLtt_Screen_Rows - 2;

   SLsmg_init_smg ();
   SLang_Error_Hook = error_hook;

   most_goto_rc (Most_Win->top,1);
   Most_Display_Inited = 1;
   /* SLsmg_refresh (); */
}

void most_reset_display()
{
   if (Most_Display_Inited == 0) return;
   SLsmg_reset_smg ();

   if (Most_Restore_Width_To)
     {
	most_set_width(Most_Restore_Width_To, 0);
	Most_Restore_Width_To = 0;
     }

   Most_Display_Inited = 0;
}

static void update_status1 (void)
{
   char info[256];
   char buf[32];
   char *file;
   unsigned int len;
   unsigned int num_chars;
   unsigned int field_width, info_len;
   unsigned char *eob;
   int r;
   MOST_INT x;

   eob = Most_Eob;
#if MOST_HAS_MMAP
   if (Most_Buf->is_mmaped)
     eob = Most_Beg + Most_Buf->mmap_size;
#endif
   
   if (eob == Most_Beg) x = 100;
   else
     {
	x = Most_C_Offset * 100;
	x = x / (eob - Most_Beg);
     }

   if (Most_C_Line + (Most_Win->bot - Most_Win->top + 1) >= Most_Num_Lines)
     x = 100;

   /* for files with end of file above the bottom row (due to window manipulations) */
   if (x > 100) x = 100;

   sprintf (info, "(" MOST_INT_D_FMT ",%d) " MOST_INT_D_FMT "%%",
	    Most_C_Line, Most_Column, x);
   
   r = Most_Win->bot + 1;
   most_goto_rc (r,1);

   most_tt_reverse_video ();

   buf[0] = '-';
   if (Most_Win->lock) buf[1] = '*'; else buf[1] = '-';
   strcpy (buf + 2, " MOST: ");

   num_chars = strlen (buf);
   SLsmg_write_nchars (buf, num_chars);
   
   /* So far num_chars have been written out.  We would like to put the
    * line number information about info_len charcters from the RHS of the
    * screen.  This leaves a field width of 
    * SLtt_Screen_Cols - (16 + num_chars) for the filename.
    */
   info_len = strlen (info);
   field_width = 12;
   if (info_len > field_width)
     field_width = info_len;
   field_width = field_width + num_chars;
   if (field_width > (unsigned int)SLtt_Screen_Cols)
     field_width = 0;
   else
     field_width = (unsigned int)SLtt_Screen_Cols - field_width;

   file = Most_Win->buf->file;
   len = strlen (file);

   if (len >= field_width)
     {
	/* Filename does not fit.  So, only display the trailing characters
	 * of the Filename.  Indicate this with ...
	 */
	if (field_width > 3)
	  {
	     buf[0] = '.';
	     buf[1] = '.';
	     buf[2] = '.';
	     SLsmg_write_nchars (buf, 3);

	     len += 3;
	  }

	file += (len - field_width);
     }
   SLsmg_write_string (file);
   SLsmg_erase_eol ();

   most_goto_rc (r, num_chars + field_width + 1);

   SLsmg_write_nchars (info, info_len);
   most_tt_normal_video();
}

void most_update_status (void)
{
   check_dirty_flag ();

   Most_C_Line = Most_Win->beg_line;
   Most_C_Offset = Most_Win->top_offset;
   /* most_set_scroll_region(1,SLtt_Screen_Rows); */
   update_status1();
   /* most_set_scroll_region(Most_Win->top,Most_Win->bot); */
   /* SLsmg_refresh (); */
}

/* splits window-- no screen update, does not affect scrolling region */
int most_split_window (void)
{
   Most_Window_Type *new, *old;
   int b2,t2,b1, line;

   b2 = Most_Win->bot;
   b1 = (Most_Win->bot + Most_Win->top) / 2 - 1;
   t2 = b1 + 2;
   if ((b1 == Most_Win->top) || (t2 == b2)) return(0);

    /* line is top line of new window. */
   line = Most_Win->beg_line + t2 - Most_Win->top;
   old = Most_Win;
   Most_Win->bot = b1;
   new = make_window(t2,b2);
    /* add to ring */
   Most_Win->next->prev = new;
   new->next = Most_Win->next;
   new->prev = Most_Win;
   Most_Win->next = new;

   new->beg_line = line;
   new->buf = Most_Buf;
#if 0
    /* new window status line is at same position as old */
   strcpy(new->status,Most_Win->status);
#endif
   return(1);
}

void most_two_windows()
{
   int line;
   Most_Window_Type *nnew, *old;
   if (!most_split_window()) return;

   old = Most_Win;
   nnew = Most_Win->next;
   line = nnew->beg_line;
   if (line + nnew->bot - nnew->top > Most_Num_Lines)
     {
	most_other_window(1);
          /* since split window left nnew window undefined... */
	Most_C_Offset = old->top_offset;
	Most_C_Line = old->beg_line;
	if (Most_Num_Lines <= nnew->bot - nnew->top + 1)
	  {
	     Most_C_Line = nnew->beg_line = 1;
	     Most_C_Offset = 0;
	     nnew->top_offset = 0;
	     most_redraw_window();
	     most_update_status();
	  }
	else if (line > Most_Num_Lines)
	  {
	     most_goto_line(Most_Num_Lines - nnew->bot + nnew->top);
	     Most_Win->top_offset = Most_C_Offset;
	     Most_Win->beg_line = Most_C_Line;
	     most_redraw_window();
	     most_update_status();
	  }
	else
	  {
	     most_goto_line(line);
	     Most_Win->top_offset = Most_C_Offset;
	     Most_Win->beg_line = Most_C_Line;
	     most_update_window(Most_Num_Lines - nnew->bot + nnew->top);
	  }
	Most_Win->curs_line = 1;
	Most_Win->curs_col = Most_Column;
	Most_Win->curs_offset = Most_C_Offset;
	most_other_window(-1);
     }
   else
     {
	Most_Win = nnew;
	(void) most_forward_line(line - old->beg_line);
	nnew->beg_line = Most_C_Line;
	nnew->curs_line = 1;
	nnew->curs_col = Most_Column;
	nnew->top_offset = Most_C_Offset;
	nnew->curs_offset = Most_C_Offset;
	most_update_status();
	Most_Win = old;
     }
   most_update_status();
}

static void delete_other_windows (void)
{
   Most_Window_Type *w, *tmp;

   if (Most_Win->next == Most_Win) return;
   w = Most_Win;
   Most_Win = Most_Win->next;
   /* delete other windows preserving the ring! */
   while (Most_Win != w)
     {
	most_free_window_buffer(); /* needs a ring */
	tmp = Most_Win->next;
#if 0
	/* if this is the bottom window, save its status line */
	if (tmp == Most_Top_Win) strcpy(w->status,Most_Win->status);
#endif
	tmp->prev = Most_Win->prev;
	Most_Win->prev->next = tmp;
#if 0
	SLFREE(Most_Win->status);
#endif
	SLFREE(Most_Win);
	Most_Win = tmp;
     }
   Most_Win = w;
}

void most_one_window ()
{
   /* Note: This is called from a SIGWINCH signal handler.  As a result, make
    * sure that top, bot is set.
    */
   if (Most_Win->next != Most_Win)
     delete_other_windows ();

   Most_Win->top = 1;
   Most_Win->bot = SLtt_Screen_Rows - 2;
   if (Most_Win->bot < 1)
     Most_Win->bot = 1;

   Most_Top_Win = Most_Win;
   most_redraw_window();
   most_update_status();
}

void most_set_window (Most_Window_Type *w)
{
   Most_Win = w;

   if (Most_Win->buf == NULL)
     {
	Most_Win->buf = Most_Buf;
	if (Most_Buf == NULL)
	  return;
     }

   most_switch_to_buffer(Most_Win->buf);

   Most_C_Offset = Most_Win->top_offset;
   Most_Curs_Offset = Most_Win->curs_offset;
   Most_Curs_Row = Most_Win->curs_line;
   Most_Curs_Col = Most_Win->curs_col;
   Most_C_Line = Most_Win->beg_line;
   Most_Column = Most_Win->col;
   restore_win_flags();
   check_dirty_flag ();
   most_point_cursor ();
}

void most_other_window(int n)
{
   if (!n) return;
   Most_Win->top_offset = Most_C_Offset;
   Most_Win->curs_offset = Most_Curs_Offset;
   Most_Win->curs_line = Most_Curs_Row;
   Most_Win->curs_col = Most_Curs_Col;
   Most_Win->beg_line = Most_C_Line;
   Most_Win->col = Most_Column;

   most_save_win_flags(Most_Win);
   if (n < 0)
     while (n++) Most_Win = Most_Win->prev;
   else
     while (n--) Most_Win = Most_Win->next;
   most_set_window(Most_Win);
}

/* kills window by moving lower window up */
static void delete_as_top_window (void)
{
   int t1,t2,b1,b2;
   t1 = Most_Win->top;
   t2 = Most_Win->next->top;
   b1 = Most_Win->bot;
   b2 = Most_Win->next->bot;
   Most_Win->prev->next = Most_Win->next;
   Most_Win->next->prev = Most_Win->prev;

   most_other_window(1);
   Most_Win->top = t1;
   most_redraw_window();
   most_update_status();
}

/* free buffer for this window if no other windows are viewing it. */
void most_free_window_buffer (void)
{
   Most_Window_Type *w;
   Most_Buffer_Type *b;

   w = Most_Win->next;

   while(Most_Win != w)
     {
	if (Most_Win->buf == w->buf) return;
	w = w->next;
     }
   b = w->buf;
   if (b == NULL) return;
   
   most_close_buffer_file (b);
   Most_Win->buf = NULL;
   SLFREE(b);
}

void most_free_windows (void)
{
   Most_Window_Type *w;

   while (Most_Win != NULL)
     {
	most_free_window_buffer ();
	w = Most_Win->next;
	w->prev = Most_Win->prev;
	Most_Win->prev->next = w;
#if 0
	SLFREE (Most_Win->status);
#endif
	SLFREE (Most_Win);
	if (w == Most_Win) w = NULL;
	Most_Win = w;
     }

   Most_Buf = NULL;
   Most_Beg = Most_Eob = NULL;
   Most_C_Line = 0;
   Most_Num_Lines = Most_C_Line = 0;
}

void most_delete_window (void)
{
   int new_b, old_b;
   Most_Window_Type *w;

   w = Most_Win;
   if (Most_Win->next == Most_Win) return;
   most_free_window_buffer();

   if (Most_Win->next != Most_Top_Win)
     {
	if (Most_Win == Most_Top_Win)
	  {
	     delete_as_top_window ();
	     Most_Top_Win = Most_Win;  /* not anymore, this one is */
	  }
	else
	  delete_as_top_window();
#if 0
	SLFREE(w->status);
#endif
	SLFREE(w);
	return;
     }

   old_b = Most_Win->top - 2;
   new_b = Most_Win->bot;
   most_other_window(-1);
   Most_Win->bot = new_b;
   most_redraw_window();

#if 0
   strcpy(Most_Win->status,w->status); /* share the same line */
#endif

   Most_Win->next = w->next;
   Most_Win->next->prev = Most_Win;

#if 0
   SLFREE (w->status);
#endif
   SLFREE(w);

   most_update_status();
}

void most_redraw_display (void)
{
   Most_Window_Type *w;
   int n,t;

   SLsmg_cls ();

   Last_Message = NULL;
   most_save_win_flags(Most_Win);
   w = Most_Win;
   do
     {
	Most_Win = Most_Win->next;
	most_switch_to_buffer(Most_Win->buf);
	Most_C_Offset = Most_Win->top_offset;
	Most_C_Line = Most_Win->beg_line;
	Most_Column = Most_Win->col;

	restore_win_flags();
	n = Most_Win->bot - Most_Win->top;

	check_dirty_flag ();

	t = Most_Win->top;
	most_goto_rc(t, 1);
	most_display_line();

	while (n--)
	  {
	     t++;
	     most_goto_rc (t, 1);
	     if (most_forward_line(1)) most_display_line();
	  }
	Most_C_Line = Most_Win->beg_line;
	Most_C_Offset = Most_Win->top_offset;
	update_status1();
     }
   while(Most_Win != w);

   most_set_window(w);

   if (Minibuffer_Selected)
     {
	most_select_minibuffer ();
	SLtt_write_string ((char *) Most_Mini_Buf);
     }
   SLsmg_refresh ();
}

void most_toggle_lock()
{
   Most_Win->lock = !(Most_Win->lock);
   most_update_status();
}

void most_point_cursor (void)
{
   int r, c;

   c = Most_Curs_Col - Most_Column + 1;
   r = Most_Curs_Row;

   if (r < 1) r = 1;
   else if (r > (Most_Win->bot - Most_Win->top + 1))
     r = Most_Win->bot - Most_Win->top + 1;

   if (c > SLtt_Screen_Cols) c = SLtt_Screen_Cols;
   else if (c < 1) c = 1;

   most_goto_rc (r + Most_Win->top - 1, c);
}

