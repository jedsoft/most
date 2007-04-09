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
/*
 * HISTORY
 * {1}	19-Mar-91  Henk D. Davids <hdavids@mswe.dnet.ms.philips.nl>
 * 	History started. Added default input file name so you do not
 *	have to specify name or type if you want it to be *.
 * 	Changes indicated by "-hdd" in comment.
 *
 *  2.  4/4/91  John E. Davis
 *      I added code to read the teminal size for unix systems-- at least it
 *      works on a sun4 (BSD ?).  In addition I have also recently added file
 *      deletion code for both unix and vms.
 */
#ifdef VMS
# include <ssdef.h>
# include <rmsdef.h>
# include <dvidef.h>
# include <jpidef.h>
# include <libdef.h>
# include <descrip.h>
# include <iodef.h>
# include <ttdef.h>
# include <starlet.h>

/* #include <unixlib.h> */

#endif  /* VMS */

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include <slang.h>
#include "jdmacros.h"

#ifdef unix
# include <signal.h>
# ifdef SYSV
#  include <sys/types.h>
#  include <fcntl.h>
# endif
# include <sys/file.h>
#endif

#ifdef __os2__
# include <signal.h>
#endif

#include <string.h>

#include "sysdep.h"
#include "display.h"
#include "window.h"
#include "most.h"

#ifdef VMS
typedef struct
{
   /* I/O status block     */
   short i_cond;           /* Condition value      */
   short i_xfer;           /* Transfer count     */
   long  i_info;           /* Device information     */
}
iosb;

typedef struct
{
   /* Terminal characteristics   */
   char  t_class;          /* Terminal class     */
   char  t_type;           /* Terminal type      */
   short t_width;          /* Terminal width in characters   */
   long  t_mandl;          /* Terminal's mode and length   */
   long  t_extend;         /* Extended terminal characteristics  */
}
termchar;

static short TTY_CHANNEL_GLOBAL;
static int zero = 0;

#else
int TTY_DESCR;
#endif /* VMS */

/*
 *
 *
 *                          SHELL COMMANDS
 *
 */

#ifdef VMS

/* these two from emacs source */
# if 0
static define_logical_name (char *varname, char *string)
{
   static char sstring[200], svarname[200];

   struct dsc$descriptor_s strdsc =
     {strlen (string), DSC$K_DTYPE_T, DSC$K_CLASS_S, sstring};
   struct dsc$descriptor_s envdsc =
     {strlen (varname), DSC$K_DTYPE_T, DSC$K_CLASS_S, svarname};
   struct dsc$descriptor_s lnmdsc =
     {7, DSC$K_DTYPE_T, DSC$K_CLASS_S, "LNM$JOB"};

   strcpy(sstring, string); strcpy(svarname, varname);

   LIB$SET_LOGICAL (&envdsc, &strdsc, &lnmdsc, 0, 0);
}

static delete_logical_name (char *varname)
{
   struct dsc$descriptor_s envdsc =
     {strlen (varname), DSC$K_DTYPE_T, DSC$K_CLASS_S, varname};
   struct dsc$descriptor_s lnmdsc =
     {7, DSC$K_DTYPE_T, DSC$K_CLASS_S, "LNM$JOB"};

   LIB$DELETE_LOGICAL (&envdsc, &lnmdsc);
}

int most_do_emacs_command()
{
   unsigned long pid;
   char *pidstr;

   if((pidstr = getenv("EMACS_PID")) != NULL)
     {
	(void) sscanf(pidstr,"%X",&pid);
	if (lib$attach(&pid) == SS$_NORMAL) /* we attach to emacs */
	  return(1);
	else
	  return(0);
          /* 	    printf("Can't attach to pid %X\n",pid); */
     }
   else return(0);
}
# endif  /* if 0 */

static unsigned long SHELL_PID = 0;

/* returns 0 on success */
int most_do_shell_command()
{
    /* here we try to attach to the parent otherwise just spawn a new one */
   unsigned long parent_pid;
   unsigned long status = 0;
   char str[80];

   $DESCRIPTOR(MOST_$_descr, "MOST > ");

   if (Most_Secure_Mode)
     {
	most_message ("Spawning not permitted in secure mode.", 1);
	return 0;
     }

   parent_pid = getppid();

   if (parent_pid && (parent_pid != 0xffffffffU))
      /* we attach to parent */
     status = lib$attach(&parent_pid);

   else if (SHELL_PID && (SHELL_PID != 0xffffffffU))
      /* try to attach to previous shell */
     status = lib$attach (&SHELL_PID);

   if (status != SS$_NORMAL)		/* others fail so spawn a new shell */
     {
	status = 0;
	SLtt_write_string("Spawning MOST DCL SUBPROCESS (Logout when finished)...");
	lib$spawn(0,0,0,0,0,&SHELL_PID,&status,0,0,0,&MOST_$_descr);
          /* if we attach back, status may come back unchanged */
	if ((status != 0) && (status != SS$_NORMAL))
	  {
	     sprintf(str,"Unable to spawn subprocess. Error = %X", status);
	     most_message(str,1);
	     return(0);
	  }
     }
   most_message(" ",0);  /* make sure message window is restored */
   return(1);
}

#endif /* VMS */

/*
 *                            FILE IO
 *
 */

#ifdef VMS
int most_expand_file_name(char *file,char *expanded_file)
{
   unsigned long status;
   static int context = 0, len = 0;
   static char inputname[MAX_PATHLEN] = "";
   $DESCRIPTOR(file_desc,inputname);
   $DESCRIPTOR(default_dsc,"SYS$DISK:[]*.*;");
   static struct dsc$descriptor_s  result =
     {0, DSC$K_DTYPE_T, DSC$K_CLASS_D, NULL};

   if (strcmp(inputname, file))
     {
	if (context)
	  {
	     lib$find_file_end(&context);
	  }
	context = 0;
	strcpy(inputname, file);
	len = strlen(inputname);
     }
   file_desc.dsc$w_length = len;

   status = lib$find_file(&file_desc,&result,&context,
			  &default_dsc,0,0,&zero);

   if (status == RMS$_NORMAL)
     {
	memcpy(expanded_file, result.dsc$a_pointer, result.dsc$w_length);
	expanded_file[result.dsc$w_length] = '\0';
     }
   else
     expanded_file[0] = '\0';       /* so file comes back as zero width */
   
   return status;
}
#endif /* VMS */

/*
 *
 *
 *         Terminal IO
 *
 */

#ifdef VMS
/*
 *      Exit Handler Control Block
 */
static struct argument_block
{
   int forward_link;
   int (*exit_routine)();
   int arg_count;
   int *status_address;
   int exit_status;
}
Exit_Block =
{
   0,
   NULL,
   1,
   &Exit_Block.exit_status,
   0
};

#endif  /* VMS */

#ifdef unix

# include <sys/time.h>

# if !defined(sun)
#  include <sys/ioctl.h>
# endif

# ifndef NO_UNISTD_H
#  include <unistd.h>
# endif

# if HAS_TERMIOS
#  include <termios.h>
# endif
# ifdef SYSV
#  include <sys/termio.h>
#  include <sys/stream.h>
#  include <sys/ptem.h>
#  include <sys/tty.h>
# endif
# include <sys/types.h>
# include <sys/stat.h>
# include <errno.h>

#if defined(SIGHUP) && !defined(IBMPC_SYSTEM)
static void unix_hangup(int sig)
{
   most_exit_error ("most: Exiting on signal %d.", sig);
}

#endif

#endif /* unix */

static int Want_Window_Size_Change;
static int Most_TTY_Inited = 0;

#ifdef REAL_UNIX_SYSTEM
static int handle_interrupts (void)
{
   if (Want_Window_Size_Change)
     most_resize_display ();

   return 0;
}
#endif

#ifdef SIGWINCH
static void sigwinch_handler (int sig)
{
   (void) sig;
   Want_Window_Size_Change = 1;
   (void) SLsignal_intr (SIGWINCH, sigwinch_handler);
}
#endif

#if defined(SIGBUS) && MOST_HAS_MMAP
static void sigbus_handler (int sig)
{
   (void) sig;
   most_reset_tty ();
   most_reset_display ();

   fprintf (stderr, "SIGBUS caught--- fatal error\n");
   if ((Most_Buf != NULL)
       && Most_Buf->is_mmaped)
     fprintf (stderr, "This may be due to a modified memory mapped file\n");
   
   exit (1);
}
#endif

void most_resize_display (void)
{
   Want_Window_Size_Change = 0;

   if (Most_Display_Inited == 0) 
     return;

   most_get_term_dimensions(&SLtt_Screen_Cols, &SLtt_Screen_Rows);

   if (Most_Win != NULL)
     most_one_window ();
#if SLANG_VERSION > 10305
   SLsmg_reinit_smg ();
#else
   SLsmg_reset_smg ();
   SLsmg_init_smg ();
#endif
   most_redraw_display ();
}


static int init_tty (void)
{
#if defined(REAL_UNIX_SYSTEM)
   SLsig_block_signals ();
#endif

   if (-1 == SLang_init_tty (7, 0, 1))
     {
	fprintf (stderr, "most: failed to init terminal.\n");
	exit (1);
     }
#if !defined(IBMPC_SYSTEM)
   SLtt_enable_cursor_keys ();
#endif
#if defined(REAL_UNIX_SYSTEM)
   SLsig_unblock_signals ();
   SLang_getkey_intr_hook = handle_interrupts;
#endif
   return 0;
}

void most_init_tty (void)
{
   int i;
   if (Most_TTY_Inited) return;

   (void) init_tty ();
#if !defined(IBMPC_SYSTEM)
   SLsig_block_signals ();
#endif

   for (i = 1; i < 32; i++)
     {
	switch (i)
	  {
#ifdef SIGHUP
	   case SIGHUP:
#endif
#ifdef SIGQUIT
	   case SIGQUIT:
#endif
#ifdef SIGTERM
	   case SIGTERM:
# ifdef REAL_UNIX_SYSTEM
	     SLsignal_intr (i, unix_hangup);
# else
	     (void) i;
# endif
	     break;
#endif

#ifdef SIGWINCH
	   case SIGWINCH:
	     (void) SLsignal_intr (SIGWINCH, sigwinch_handler);
	     break;
#endif

#if defined(SIGBUS) && MOST_HAS_MMAP
	   case SIGBUS:
	     (void) SLsignal_intr (SIGBUS, sigbus_handler);
	     break;
#endif

#ifdef SIGCONT
	   case SIGCONT:
#endif
#ifdef SIGTSTP
	   case SIGTSTP:
#endif
#ifdef SIGINT
	   case SIGINT:
#endif
#ifdef SIGCHLD
	   case SIGCHLD:
#endif
	   default:
	     break;
#ifdef SIGPIPE
	   case SIGPIPE:
	     (void) SLsignal_intr (SIGPIPE, SIG_IGN);
	     break;
#endif
	  }
     }

   SLang_set_abort_signal (NULL);

   Most_TTY_Inited = 1;
#if !defined(IBMPC_SYSTEM)
   SLsig_unblock_signals ();
#endif
}

void most_reset_tty (void)
{
   if (Most_TTY_Inited == 0) return;
   SLang_reset_tty ();
   Most_TTY_Inited = 0;
}

int most_reinit_terminal (void)
{
   if (Most_TTY_Inited == 0)
     return 0;
   most_reset_tty ();
   (void) init_tty ();
   Most_TTY_Inited = 1;
   return 0;
}


/* slang uses select to wait for terminal input.  If SIGINT is
 * generated, then slang returns ^G and sets SLKeyBoard_Quit
 * to 1 bypassing the read altogether.  If most was used in
 * conjunction with xargs (find . -print | xargs most) and ^G
 * is pressed, a SIGINT will be generated killing xargs and
 * leaving most an orphan unable to access the terminal.
 * Unfortunately, the select call will continue to time-out
 * causing most continue running in the background.  To
 * workaround this problem, exit if the terminal is not in the
 * same process group as most.
 */
static void check_if_foreground (void)
{
#if defined(HAVE_GETPGRP) && defined(HAVE_TCGETPGRP)
   pid_t pgid = getpgrp ();

   if ((SLang_TT_Read_FD != -1)
       && (pgid != tcgetpgrp (SLang_TT_Read_FD)))
     {
	most_exit_error ("\007Fatal Error: Most is not in the terminal's process group.");
     }
#endif
}

int most_getkey()
{
   unsigned int ch;
   static int last_was_sigint;

   /* This non-sense involving last_was_sigint is to handle a race condition.
    * The shell may not have had time to change the pgids, so sleep a bit
    * here.  Yea, its ugly and does not really solve the problem...
    */
   if (last_was_sigint)
     {
	sleep (1);
	check_if_foreground ();
	last_was_sigint = 0;
     }

   ch = SLang_getkey ();
   if (ch == SLANG_GETKEY_ERROR)
     most_exit_error ("most: getkey error.");

   if (SLKeyBoard_Quit)
     {
	check_if_foreground ();
	last_was_sigint = 1;
     }
   SLKeyBoard_Quit = 0;
   SLang_set_error (0);
   return (int) ch;
}

/*
 *
 *      Misc Termial stuff
 *
 *
 */

/*  This is to get the size of the terminal  */
void most_get_term_dimensions(int *cols, int *rows)
{
   SLtt_get_screen_size ();
   *rows = SLtt_Screen_Rows;
   *cols = SLtt_Screen_Cols;
}

/* returns 0 on failure, 1 on sucess */
int most_delete_file(char *filename)
{
#ifdef VMS
   return (1 + delete(filename));   /* 0: sucess; -1 failure */
#else
   return(1 + remove(filename));
#endif
}

/* This routine converts unix type names to vms names */
#ifdef VMS
static int locate(char ch, char *string)
{
   int i;
   char c;

   i = 0;
   while (c = string[i++], (c != ch) && (c != '\0'));
   if (c == ch) return(i); else return (0);
}

char *most_unix2vms(char *file)
{
   int i,device,j,first,last;
   static char vms_name[MAX_PATHLEN];
   char ch;

   if (locate('[',file)) return(file); /* vms_name syntax */
   if (!locate('/',file)) return(file); /* vms_name syntax */

    /* search for the ':' which means a device is present */
   device = locate(':',file);

   i = 0;
   if (device)
     {
	while (ch = file[i], i < device) vms_name[i++] = ch;
     }
   j = i;

    /* go from the  end looking for a '/' and mark it */
   i = strlen(file) - 1;
   while(ch = file[i], ch != '/' && i-- >= 0);
   if (ch == '/')
     {
	file[i] = ']';
	last = 0;
     }
   else last = 1;

   i = j;
   vms_name[j++] = '[';
   vms_name[j++] = '.';
   first = 0;
   while(ch = file[i++], ch != '\0')
     {
	switch (ch)
	  {
	   case '.':
	     if (last) vms_name[j++] = '.';
	     if (last) break;
	     ch = file[i++];
	     if (ch == '.')
	       {
		  if (!first) j--;  /* overwrite the dot */
		  vms_name[j++] = '-';
	       }
	     else if (ch == '/'); /*  './' combinations-- do nothing */
	     else if (ch == ']')
	       {
		  last = 1;
		  if (vms_name[j-1] == '.') j--;
		  vms_name[j++] = ']';
	       }

	     else vms_name[j++] = '.';
	     break;
	   case '/':
	     if (first)
	       {
		  vms_name[j++] = '.';
	       }
	     else
	       {
		  first = 1;
                      /* if '/' is first char or follows a colon do nothing */
		  if ((i!=1) && (file[i-2] != ':'))
		    {
		       vms_name[j++] = '.';
		    }
		  else j--; /* overwrite the '.' following '[' */
	       }
	     break;
	   case ']':
	     last = 1;
	     if (vms_name[j-1] == '.') j--;
	     vms_name[j++] = ']';
	     break;
	   default:
	     vms_name[j++] = ch;
	  }
     }
   return (vms_name);
}

/*
main(int argc, char **argv)
{
    puts(unix2vms(argv[1]));
}
*/

#endif /* VMS */

#include <time.h>

char *most_get_time()
{
   time_t clk;
   char *the_time;

   clk = time((time_t *) 0);
   the_time = (char *) ctime(&clk);
   /* returns the form Sun Sep 16 01:03:52 1985\n\0 */
   the_time[24] = '\0';
   return(the_time);
}

void most_set_width (int width, int redraw)
{
#ifdef IBMPC_SYSTEM
   (void) width; (void) redraw;
#else
#ifdef VMS
   short fd;
   int status;
   iosb iostatus;
   static termchar tc; /* Terminal characteristics   */
   $DESCRIPTOR( devnam, "SYS$ERROR");
#else
# ifdef TIOCGWINSZ
   struct winsize wind_struct;
# endif
#endif

    /* Switching physical terminal to narrow/wide mode.*/

   if(width <= 80)
     {
	width = 80;
	most_narrow_width();
     }
   else
     {
	width = 132;
	most_wide_width();
     }
   SLtt_Screen_Cols = width;

#ifdef VMS
    /* Assign input to a channel */
   status = sys$assign(&devnam, &fd, 0, 0);
   if ((status & 1) == 0)
     exit(status);
    /* Get current terminal characteristics */
   status = sys$qiow(          /* Queue and wait   */
		     0,        /* Wait on event flag zero  */
		     fd,       /* Channel to input terminal  */
		     IO$_SENSEMODE, /* Get current characteristic */
		     &iostatus, /* Status after operation */
		     0, 0,     /* No AST service   */
		     &tc,      /* Terminal characteristics buf */
		     sizeof(tc), /* Size of the buffer   */
		     0, 0, 0, 0); /* P3-P6 unused     */

    /*set terminal characteristics */
   tc.t_width=width;
   status = sys$qiow(           /* Queue and wait   */
		     0,           /* Wait on event flag zero  */
		     fd,           /* Channel to input terminal  */
		     IO$_SETMODE,   /* Get current characteristic */
		     &iostatus,       /* Status after operation */
		     0, 0,            /* No AST service   */
		     &tc,             /* Terminal characteristics buf */
		     sizeof(tc),      /* Size of the buffer   */
		     0, 0, 0, 0);     /* P3-P6 unused     */

   if( (sys$dassgn(fd)  & 1)==0)
     exit(status);

    /* here we redraw the screen, on unix, we assume that the terminal
     * driver sends the appropriate signal that most catches to redraw so we
     * do not redraw because it is likely that screen will be redrawn twice */

   if (redraw)
     most_resize_display ();

#else
   (void) redraw;
# ifdef TIOCGWINSZ
    /* this may need work on other unix-- works for sun4 */
   if (-1 != ioctl(TTY_DESCR,TIOCGWINSZ,&wind_struct))
     {
	wind_struct.ws_col = width;
	ioctl(TTY_DESCR,TIOCSWINSZ,&wind_struct);
     }
# endif
#endif /* VMS */
#endif				       /* NOT IBMPC_SYSTEM */
}
