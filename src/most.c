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

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef VMS
# include <rmsdef.h>
#endif

#include <errno.h>
#include <stdarg.h>

#include <slang.h>
#include "version.h"

#include "jdmacros.h"

#include "most.h"
#include "search.h"
#include "window.h"
#include "file.h"
#include "sysdep.h"
#include "keym.h"
#include "display.h"
#include "line.h"

int Most_S_Opt = 0;		       /* squeeze liness */
int Most_A_Opt = 1;		       /* automatically choose -b if necessary */
int Most_V_Opt = 0;		       /* display control chars */
int Most_B_Opt = 0;		       /* display Binary File */
int Most_T_Opt = 0;		       /* display tab as ^I-- valid only with V option */
int Most_D_Opt = 0;		       /* delete file mode  (see ':D')  */
int Most_K_Opt = 0;		       /* Display 8 bit unformatted (Kanji) */
int Most_Z_Opt = 0;		       /* Gunzip on the fly */
int Most_Want_Exit;
int Most_Secure_Mode;
int Most_Captive_Mode;
#if MOST_HAS_MMAP
int Most_Disable_MMap	= 0;
#endif

int Most_UTF8_Mode = -1;	       /* -1:auto, 0:off, 1:on */

static int  Most_Starting_Line;
char *Most_Program;	/* Program Name (argv[0]) */

static char *Most_Version = MOST_VERSION_STR;

#ifdef VMS
# ifndef isalpha
#  define isalpha(x) \
   (((x >= 'A') && (x <= 'Z'))||((x >= 'a') && (x <= 'z')) ? 1 : 0)
# endif
#endif

void most_usage (void)
{
   fprintf(stderr,"MOST version %s (S-Lang version %s)\n",
	   Most_Version, SLang_Version_String);
   if (SLang_Version != SLANG_VERSION)
     fprintf (stderr, " *Note: This executable was compiled against S-Lang %s\n", SLANG_VERSION_STRING);
     
   fprintf (stderr, "Usage:\n");
   fprintf(stderr, "most [-1Cbcdkstvw] [+/string] [+line number] [+s] [+d] file...\n");
   fputs(" where: -1:  assume VT100 terminal. (VMS only)\n", stderr);
   fputs("        -b:  Startup in binary mode.\n", stderr);
   fputs("        -C:  disable color support\n", stderr);
   fputs("        -c:  Make searches case sensitive.\n", stderr);
   fputs("        -d:  Do not display the \\ wrap marker when wrapping lines.\n", stderr);
   /* fputs("        -k:  Kanji mode.\n", stderr); */
#if MOST_HAS_MMAP
   fputs("        -M:  Do not attempt to mmap files.\n", stderr);
#endif
   fputs("        -s:  Squeeze out excess blank lines.\n", stderr);
   fputs("        -t:  Display tabs as ^I.  If this option is immediately followed\n", stderr);
   fputs("               by an integer, the integer sets the tab width.\n", stderr);
   fputs("        -u:  Disable UTF-8 mode\n", stderr);
   fputs("        -v:  Do not interpret backspace formatting characters.\n", stderr);
   fputs("        -w:  Wrap lines.\n", stderr);
   fputs("        -z:  No gunzip-on-the-fly.\n", stderr);
   fputs("        +/string:\n", stderr);
   fputs("             Search for string\n", stderr);
   fputs("        +line number\n", stderr);
   fputs("             Start up at specified line number.\n", stderr);
   fputs("        +d:  Allow file deletion.\n", stderr);
   fputs("        +s:  Secure Mode-- no edit, cd, shell, and reading files not\n", stderr);
   fputs("               already listed on the command line.\n", stderr);
   fputs("        +u:  Enable UTF-8 mode.\n", stderr);
   fprintf(stderr, "\nExample: most -ct4 +82 keymap.c\n");
   fputs(" makes searches case sensitive, sets tabwidth to 4, and displays the file\n", stderr);
   fputs(" keymap.c starting at line 82.\n", stderr);
}

static void do_switches(char *str);

static void do_extended_switches(char *str)
{
   int i;
   char ch;
   char numstr [256];

   i = 0;
   ch = *(++str);
   if ( ch == '/')
     {
	strcpy (Most_Search_Str,++str);
	return;
     }

   if (ch >= '0' && ch <= '9')
     {
	while ((ch >= '0' && ch <= '9') && (i < 10))
	  {
	     numstr[i++] = ch;
	     ch = *(++str);
	  }
	numstr[i] = '\0';
	if (1 == sscanf (numstr,"%d", &i))
	  Most_Starting_Line = i;
	return;
     }

   if (isalpha(ch))
     {
	while (1)
	  {
	     switch (ch)
	       {
		case 0:
		  return;
		case ' ':
		case '+':
		  break;
		case '-':
		  do_switches (str);
		  return;

		case 'D':
		case 'd':
		  Most_D_Opt = 1;   /* delete file mode */
		  break;
		case 'S':
		case 's':
		  Most_Secure_Mode = 1;
		  break;
		  
		case 'U':
		case 'u':
		  Most_UTF8_Mode = 1;  /* +u */
		  break;

		default:
		  fprintf(stderr,"%s invalid extended option %c ignored.\n",
			  Most_Program, ch);
	       }
	     ch = *(++str);
	  }
     }

   fprintf(stderr,"%s: switch '+%s' not valid.\n", Most_Program, str);
   exit (1);
}

/* if non-zero, assume terminal is only a generic vt100 */
static int assume_vt100 = 0;
static int No_Colors = 0;

static void do_switches(char *str)
{
   char ch;
   if (*str == '-') str++;
   while ((ch = *str++) != '\0')
     {
	switch (ch)
	  {
	   default:
	     fprintf(stderr,"%s: invalid switch %c ignored.\n",
		     Most_Program, ch);
	     break;

	   case 'C':
	     No_Colors = 1;
	     break;
	   case 'c':
	     Most_Case_Sensitive = 1;
	     break;
	   case 'd':
	   case 'D':
	     Most_Show_Wrap_Marker = 0;
	   case 's':
	   case 'S':
	     Most_S_Opt = 1; break;
	   case 'V':
	   case 'v':
	     Most_V_Opt = 1;  /* verbose-- convert control chars to '^' 'ch' */
	     break;
	   case 'W':
	   case 'w':  Most_W_Opt = 1; break;

	   case 'K':		       /* Kanji option */
	   case 'k':
	     /* Most_K_Opt = 1; break; */
	     break;

	   case 'B':
	   case 'b':
	     Most_B_Opt = 1;  /* Binary display 8 bit */
	     break;

	   case 'M':
#if MOST_HAS_MMAP
	     Most_Disable_MMap = 1;
#endif
	     break;

	   case 'z':
	   case 'Z':
	     Most_Z_Opt = 1;  /* NO Gunzip-on-the-fly */
	     break;

	   case 't':
	   case 'T': /* expand tabs to '^I'; meaningful only with 'v' */
	     ch = *str;
	     if ((ch <= '9') && (ch >= '0'))
	       {
		  str++;
		  Most_Tab_Width = (int) ch - '0';
		  if (Most_Tab_Width == 0) Most_T_Opt = 1;
	       }
	     else Most_T_Opt = 1;
	     break;

	   case 'n': case 'N':
	      /* could be the Gopher Naive user switch --- ignored. */
	     break;
	   case '1': assume_vt100 = 1;
	     break;
	     
	   case 'u':
	   case 'U':
	     Most_UTF8_Mode = 0;       /* -u */
	     break;

	     /* Allow MOST_SWITCHES environment variable to contain + forms,
	      * e.g., "-sn+d" or "-s -n +d"
	      */
	   case ' ':
	   case '-':
	     break;
	   case '+':
	     do_extended_switches (str - 1);   /* include '+' */
	     return;
	  }
     }
}

void most_exit_error(char *fmt,...)
{
   va_list ap;

   most_reset_tty ();
   most_reset_display();
   if (fmt != NULL)
     {
	va_start (ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end (ap);
	putc ('\n', stderr);
     }
#ifdef MALLOC_DEBUG
   SLmalloc_dump_statistics ();
#endif
   exit(1);
}

static void play_cat(char *file)
{
   char buf[4096 * 4];
   int n;
   FILE *fp;

   if (file == NULL) fp = stdin;
   else
     {
	fp = fopen(file, "r");
	if (fp == NULL) return;
     }

   while ((n = fread(buf, 1, 4096 * 4, fp)) > 0)
     {
	int m;
	m = fwrite (buf, 1, n, stdout);
	if (m != n)
	  {
	     fprintf (stderr, "fwrite returned %d, errno = %d\n",
		      m, errno);
	     exit (1);
	  }
     }
}

void most_initialize_most (void)
{
   Most_S_Opt = 0;
   Most_A_Opt = 1;
   Most_V_Opt = 0;
   Most_B_Opt = 0;
   Most_T_Opt = 0;
   Most_D_Opt = 0;
   Most_K_Opt = 0;
   Most_W_Opt = 0;

   Most_Selective_Display = 0;
   *Most_Search_Str = 0;   Most_Search_Dir = 1;
   Most_Top_Win = Most_Win = NULL;
   Most_Buf = NULL;
   Most_Eob = NULL;
   Most_Beg = NULL;
   Most_Captive_Mode = Most_Secure_Mode = 0;
   Most_Want_Exit = 0;
}

static void do_most (char *file, int start)
{
   int piped;
   MOST_INT row, col;

   most_get_cdir(Most_C_Dir);

   row = col = 0;
   if (file[0] == '\0') piped = 1; else piped = 0;

   if ((-1 == most_find_file (file))
       && (Most_Num_Files == 1))
     most_exit_error ("%s: failed to open for reading.", file);

   most_init_display ();

   most_goto_line(start);

   Most_Curs_Offset = Most_C_Offset;

   if (*Most_Search_Str
       && ((row = most_search (Most_Beg + Most_C_Offset, 1, &col)) > 0))
     most_goto_line(row);
   else
     {
	row = Most_C_Line;
	col = 1;
     }

   most_window_buffer();
   Most_Curs_Row = Most_Win->curs_line = row - Most_C_Line + 1;
   Most_Win->curs_offset = Most_Curs_Offset;
   Most_Curs_Col = Most_Win->curs_col = col;
   most_redraw_window();
   most_update_status();

   while (Most_Want_Exit == 0)
     {
	most_execute_key();
     }
}

void most_exit_most (void)
{
   if (Most_Want_Exit) return;
   Most_Want_Exit = 1;
   most_clear_minibuffer ();
   most_reset_tty ();
   most_reset_display ();
   most_free_windows ();
#ifdef MALLOC_DEBUG
   SLmalloc_dump_statistics ();
#endif
}

static void utf8_config (void)
{
   int utf8_mode = Most_UTF8_Mode;

   utf8_mode = SLutf8_enable (-1);     /* returns 0 or 1 */
   if (Most_UTF8_Mode == -1)
     Most_UTF8_Mode = utf8_mode;
   else if (utf8_mode != Most_UTF8_Mode)
     {
	if (utf8_mode == 1)
	  (void) SLsmg_utf8_enable (0);   /* locale is UTF-8, but -u passed */
	else
	  (void) SLsmg_utf8_enable (1);   /* locale not UTF-8, but +u passed */
     }
}

int most (int argc, char **argv)
{
   char file[MAX_PATHLEN], *switches;
   int file_i = 0, quit,i,piped,a_opt;
   unsigned long context;
   int status = 0;

#ifdef VMS
   char filename[256];
#else
   int j;
#endif

   Most_Program = argv[0];
   piped = 0;

   switches = getenv ("MOST_PROMPT");
   if ((switches != NULL) && (*switches != 0)) Most_Global_Msg = switches;

   switches = getenv("MOST_SWITCHES");
   if (switches !=  NULL)  do_switches(switches);

   i = 1;
   if (argc > 1)
     {
	quit = 0;
	while ((!quit) && (i < argc))
	  {
	     if (argv[i][0] == '-')
	       do_switches(argv[i++]);
	     else if (argv[i][0] == '+')
	       do_extended_switches(argv[i++]);
	     else quit = 1;
	  }
     }

#if MOST_HAS_MMAP
   /* if (Most_D_Opt) */
   /*   Most_Disable_MMap = 1; */
#endif

   if (i == argc)
     {
	if (isatty(0))   /* 1 if stdin is a terminal, 0 otherwise */
	  {
	     most_usage ();
	     return 0;
	  }
	/* assume input is from stdin */
	file[0] = '\0';  /* tells most this is stdin */
	piped = 1;
	if (!isatty(fileno(stdout)))
	  {
	     play_cat(NULL);
	     return 0;
	  }
     }
   else
     {
	strncpy (file, argv[i], sizeof(file));
	file[sizeof(file)-1] = 0;
     }

   if (!isatty(fileno(stdout)))
     {
	while (i < argc) play_cat(argv[i++]);
	exit(0);
     }

   Most_Num_Files = 0;
   context = 0;

   SLtt_get_terminfo();
   utf8_config ();
   SLtt_Ignore_Beep = 1;
   if (No_Colors) 
     SLtt_Use_Ansi_Colors = 0;

   most_setup_colors ();
   most_init_tty ();
   most_init_keymaps ();

   if (Most_B_Opt) Most_A_Opt = 0;   /* explicit b overrides a */
   a_opt = Most_A_Opt;

   if (!piped)
     {
	file_i = i;
#ifdef VMS
	while(i < argc)
	  {
	     if (Most_Num_Files >= MOST_MAX_FILES) break;
	     if (argv[i][0] == '.') strcpy(file,"*"); else *file = 0;
	     strcat(file, most_unix2vms(argv[i++]));
	     while (RMS$_NORMAL == (status = most_expand_file_name(file,filename)))
	       {
		  Most_File_Ring[Most_Num_Files] = (char*) MOSTMALLOC(strlen(filename) + 1);
		  strcpy(Most_File_Ring[Most_Num_Files++], filename);
	       }
	     if (status == RMS$_NMF) status = RMS$_NORMAL; /* avoid spurious warning message */
	  }
	
	if (Most_Num_Files) strcpy(file,Most_File_Ring[0]);
	else fputs("%%MOST-W-NOFILES, no files found\n", stderr);
#else
	Most_Num_Files = argc - i;
	if (Most_Num_Files > MOST_MAX_FILES)
	  {
	     Most_Num_Files = MOST_MAX_FILES;
	     argc = Most_Num_Files + i;
	  }

	j = 0;
	while (i < argc)
	  {
	     Most_File_Ring[j++] = argv[i++];
	  }
#endif
     }

   if (Most_Num_Files || piped) do_most(file, Most_Starting_Line);
   else if (Most_Num_Files == 0)
     fprintf(stderr,"File %s not found\n", argv[file_i]);

   most_exit_most ();
   return status;
}

#if SLANG_VERSION <= 10409

int SLang_set_error (int x)
{
   SLang_Error = x;
   return 0;
}

int SLang_get_error (void)
{
   return SLang_Error;
}

#endif
