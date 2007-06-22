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

#ifdef __unix__
# ifndef MOST_SYSTEM_INITFILE
#  define MOST_SYSTEM_INITFILE "/etc/most.conf"
# endif
#endif
  
/* parses user key definition file */

/* The file is assumed to have a simple format:
 *
 *   set "keyname"  function
 *   unset "keyname"
 *
 * For example:
 *
 *   unsetkey "^K"
 *   setkey exit "^Kx"
 *
 * Comments extend from the first '%' character to the end of the line.
 */

#include <slang.h>
#include "jdmacros.h"
#include "most.h"
#include "keyparse.h"
#include "display.h"

static unsigned int Line_Num;

static void parse_error (char *s)
{
   fprintf(stderr, "Most: Error reading config file on line %u.\n%s\n",
	   Line_Num, s);

   most_exit_error (NULL);
}

static int unsetkey_fun (int, SLcmd_Cmd_Table_Type *);
static int setkey_fun (int, SLcmd_Cmd_Table_Type *);
static int color_fun (int, SLcmd_Cmd_Table_Type *);
static int mono_fun (int, SLcmd_Cmd_Table_Type *);

static SLcmd_Cmd_Type Startup_File_Cmds[] =
{
     {unsetkey_fun, "unsetkey", "S"},
     {setkey_fun, "setkey", "SS"},
     {color_fun, "color", "SSS"},
     {mono_fun, "mono", "SSsss"},
     {NULL, "", ""}
};
static SLcmd_Cmd_Table_Type Cmd_Table;

static int setkey_fun (int argc, SLcmd_Cmd_Table_Type *table) /*{{{*/
{
   char *fun = table->string_args[1];
   char *key = table->string_args[2];

   (void) argc;
   if (NULL == SLang_find_key_function(fun, Most_Keymap))
     {
	parse_error ("Undefined function");
     }

   if (0 != SLang_define_key (key, fun, Most_Keymap))
     parse_error ("Error setting key");

   return 0;
}

static int unsetkey_fun (int argc, SLcmd_Cmd_Table_Type *table) /*{{{*/
{
   char *key = table->string_args[1];

   (void) argc;

   SLang_undefine_key (key, Most_Keymap);
   return 0;
}

static int parse_file (char *file)
{
   char buf[512];
   FILE *fp;

   if (NULL == (fp = fopen (file, "r"))) 
     return 1;

   Cmd_Table.table = Startup_File_Cmds;

   Line_Num = 0;
   while (NULL != fgets (buf, sizeof (buf), fp))
     {
	Line_Num++;
	(void) SLcmd_execute_string (buf, &Cmd_Table);
	if (SLang_get_error())
	  parse_error ("Undefined keyword");
     }

   fclose (fp);
   if (SLang_get_error ())
     {
	SLang_set_error(0);
	return -1;
     }
   return 0;
}

int most_load_user_keymaps (void)
{
#ifndef VMS
   char filebuf[MAX_PATHLEN];
   unsigned int len;
#endif
   char *file;

#ifdef MOST_SYSTEM_INITFILE
   if (MOST_SYSTEM_INITFILE != NULL)
     {
	int status;
	status = parse_file (MOST_SYSTEM_INITFILE);
	if (status == -1)
	  return -1;
     }
#endif

   if (NULL == (file = getenv ("MOST_INITFILE")))
     {
#ifdef VMS
	file = "SYS$LOGIN:MOST.RC";
#else
	*filebuf = 0;
	file = getenv ("HOME");
	if (file == NULL)
	  return -1;
	
	len = strlen (file);
	if (len + 8 >= sizeof (filebuf))   /* 8 for strlen("/.mostrc") */
	  return -1;

	strcpy (filebuf, file);
	file = filebuf;

	if (len && (file[len - 1] == '/')) 
	  len--;
	strcpy (file + len, "/.mostrc");
#endif
     }
   return parse_file (file);
}

/* The following code was borrowed from slrn */
/*{{{ Setting Color/Mono Attributes */

typedef struct /*{{{*/
{
   char *name;
   int value;
   char *fg, *bg;
   SLtt_Char_Type mono;
}

/*}}}*/
Color_Handle_Type;

/* default colors -- suitable for a color xterm */
   
static Color_Handle_Type Color_Handles[] = /*{{{*/
{
     {"normal",		0,			"default",	"default", 0},
     {"status",		MOST_STATUS_COLOR,	"yellow",	"blue", SLTT_REV_MASK},
     {"underline",	MOST_ULINE_COLOR,	"brightgreen",	"default", SLTT_ULINE_MASK},
     {"overstrike",	MOST_BOLD_COLOR,	"brightred",		"default", SLTT_BOLD_MASK},

     {NULL, -1, NULL, NULL, 0}
};


static int set_object_color (char *name, char *fg, char *bg)
{
   Color_Handle_Type *ct = Color_Handles;

   while (ct->name != NULL)
     {
	if (!strcmp (ct->name, name))
	  {
	     SLtt_set_color (ct->value, name, fg, bg);
	     return 0;
	  }
	ct++;
     }
   
   parse_error ("Undefined color object");
   return -1;
}

static int color_fun (int argc, SLcmd_Cmd_Table_Type *table)
{
   char *what = table->string_args[1];
   char *fg = table->string_args[2];
   char *bg = table->string_args[3];
   
   (void) argc;   
   return set_object_color (what, fg, bg);
}


static int mono_fun (int argc, SLcmd_Cmd_Table_Type *table)
{
   char *what = table->string_args[1];
   char *attr;
   int i;
   
   Color_Handle_Type *ct = Color_Handles;
   
   while (ct->name != NULL)
     {
	if (!strcmp (ct->name, what))
	  {
	     SLtt_Char_Type mono_attr = 0;
	     for (i = 2; i < argc; i++)
	       {
		  attr = table->string_args[i];
		  if (!strcmp (attr, "bold")) mono_attr |= SLTT_BOLD_MASK;
		  else if (!strcmp (attr, "blink")) mono_attr |= SLTT_BLINK_MASK;
		  else if (!strcmp (attr, "underline")) mono_attr |= SLTT_ULINE_MASK;
		  else if (!strcmp (attr, "reverse")) mono_attr |= SLTT_REV_MASK;
		  else if (!strcmp (attr, "none")) mono_attr = 0;
		  else
		    {
		       parse_error ("Undefined mono attribute");
		       return -1;
		    }
	       }
	     SLtt_set_mono (ct->value, NULL, mono_attr);
	     return 0;
	  }
	ct++;
     }
   parse_error ("Undefined color object");
   return -1;
}

static char *Ansi_Color_Map[9] =
{
   "black",
   "red",
   "green",
   "yellow",
   "blue",
   "magenta",
   "cyan",
   "white", 
   "default"
};

void most_setup_colors (void)
{
   Color_Handle_Type *h;
   int i;
   int fg, bg, at;

   for (i = 1; i < 128; i++)
     {
	SLtt_set_color (i, NULL, "default", "default");
	SLtt_set_mono (i, NULL, 0);
     }

   for (at = 0; at < 9; at++)
     {
	for (fg = 0; fg < 9; fg++)
	  {
	     for (bg = 0; bg < 9; bg++)
	       {
		  i = fg + 9*(bg + 9*at);
		  if (i == 0)
		    continue;
		  i += MOST_EMBEDDED_COLOR_OFFSET;
		  SLtt_set_color (i, NULL, Ansi_Color_Map[fg], Ansi_Color_Map[bg]);
	       }
	  }
     }

   h = Color_Handles;
   while (h->name != NULL)
     {
	/* if (h->value != 0)*/	       /* Let COLORFGBG apply to initial color */
	SLtt_set_color (h->value, NULL, h->fg, h->bg);
	SLtt_set_color (h->value+MOST_EMBEDDED_COLOR_OFFSET, NULL, h->fg, h->bg);
	SLtt_set_mono (h->value, NULL, h->mono);
	SLtt_set_mono (h->value+MOST_EMBEDDED_COLOR_OFFSET, NULL, h->mono);
	
	h++;
     }
}
