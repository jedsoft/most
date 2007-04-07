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

#ifdef VMS
# include <stat.h>
#else
# include <sys/types.h>
# include <sys/stat.h>
#endif

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include <slang.h>
#include "jdmacros.h"

#include "most.h"
#include "window.h"
#include "file.h"
#include "display.h"
#include "line.h"
#include "keym.h"
#include "sysdep.h"

/* This section provided by Mats Akerberg (mats@efd.lth.se) */

static char *help[] =
{
   "Quitting:",
   "  Q                      Quit MOST.",
   "  :N,:n                  Quit this file and view next. ",
   "                            (Use UP/DOWN arrow keys to select next file.)",
   "Movement:",
   "  SPACE, D              *Scroll down one Screen.",
   "  U, DELETE             *Scroll Up one screen.",
   "  RETURN, DOWN          *Move Down one line.",
   "  UP                    *Move Up one line.",
   "  T                      Goto Top of File.",
   "  B                      Goto Bottom of file.",
   "  > , TAB                Scroll Window right",
   "  <                      Scroll Window left",
   "  RIGHT                  Scroll Window left by 1 column",
   "  LEFT                   Scroll Window right by 1 column",
   "  J, G                   Goto line.",
   "  %                      Goto percent.",
   "Window Commands:",
   "  Ctrl-X 2, Ctrl-W 2     Split window.",
   "  Ctrl-X 1, Ctrl-W 1     Make only one window.",
   "  O, Ctrl-X O            Move to other window.",
   "  Ctrl-X 0               Delete Window.",
   "Searching:",
   "  S, f, /               *Search forward",
   "  ?                     *Search Backward",
   "  N                     *Find next in current search direction.",
   "Miscellaneous:",
   "  W                      Toggle width between 80 and 132 char mode.",
   "  Ctrl-X Ctrl-F          Read a file from disk",
   "  R, Ctrl-R              Redraw Screen.",
   "  F                      Simulate tail -f mode",
   "  :o                     Toggle options:  b-binary, w-wrap, t-tab",
   "  E                      Edit file.  Uses MOST_EDITOR and EDITOR",
   "                           environment variables.",
   "*Note:  This command may be repeated `n' times By entering a number then",
   "        the command key, e.g.,  '5 SPACE' moves 5 screens forward.",
   NULL
};

static void most_do_help_text (void)
{
   char **p = help, *sect = NULL;
   int r;

   while (*p != NULL)
     {
	SLsmg_cls ();

	r = 0;
	SLsmg_gotorc (0, 0);

	if ((sect != NULL) && (**p == ' '))
	  {
	     most_tt_bold_video ();
	     SLsmg_gotorc (r, 0);
	     SLsmg_write_string (sect);
	     most_tt_normal_video ();
	     SLsmg_write_string (" (continued)");
	     r += 2;
	  }
	else sect = NULL;

	while (r < SLtt_Screen_Rows - 1)
	  {
	     if (*p == NULL) break;

	     if (**p != ' ')
	       {
		  if (((r + 5) > SLtt_Screen_Rows)
		      && (**p != '*'))
		    {
		       sect = NULL;
		       break;
		    }

		  if (sect != NULL)
		    {
		       r++;
		    }

		  if (**p != '*')
		    {
		       sect = *p;
		       most_tt_bold_video ();
		    }
		  else sect = NULL;
	       }
	     SLsmg_gotorc (r, 0);
	     SLsmg_write_string (*p);

	     if ((**p != ' ') && (**p != '*'))
	       {
		  most_tt_normal_video ();
		  r++;
	       }
	     p++;
	     r++;
	  }

	SLsmg_gotorc (r, 0);

	most_tt_reverse_video();
	SLsmg_write_string("Press any key to continue.");
	most_tt_normal_video();

	SLsmg_refresh ();

	most_getkey ();
     }

   most_redraw_display();
}

static void most_do_help_file (char *helpfile)
{
   char *buf_name;
   FILE *fp;

   buf_name = "*help*";

#ifdef MOST_HELPFILE
   if (helpfile == NULL) helpfile = MOST_HELPFILE;
#endif

   if (helpfile != NULL)
     {
	if (most_file_visible(buf_name)) return;

	/* See if we can open it */
	if (NULL != (fp = fopen  (helpfile, "r")))
	  {
	     fclose (fp);
	     if (!most_split_window())
	       {
		  most_message("Two many windows.",1);
		  return;
	       }
	     most_update_status();  /* create status line of prev. window */
	     most_other_window(1);
	     (void) most_find_file(helpfile);
	     strcpy(Most_Buf->file, buf_name);
	     Most_B_Opt = 0;
	     most_window_buffer ();
	     most_redraw_window ();
	     most_update_status ();
	     return;
	  }
     }

   most_do_help_text ();
}

void most_do_help_command(void)
{
   most_do_help_file (getenv ("MOST_HELP"));
}

