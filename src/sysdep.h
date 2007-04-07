#ifndef __DAVIS_SYSDEP_H__
# define __DAVIS_SYSDEP_H__
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

# include <stdio.h>

# if defined(sequent) || defined(apollo)
#  define HAS_TERMIOS 0
# else
#  define HAS_TERMIOS 1
# endif

# ifdef VMS
extern int most_do_emacs_command(void);

extern int most_do_shell_command();
extern int most_expand_file_name(char *,char *);
extern char *most_unix2vms(char *);

# endif /* VMS */

extern int most_reinit_terminal (void);
extern void most_init_tty(void);
extern void most_reset_tty(void);
extern int most_getkey(void);
extern void most_get_term_dimensions(int *, int *);
extern int most_delete_file(char *);
extern void most_set_width(int, int);
extern char *most_get_time(void);

extern void most_resize_display (void);
#endif /* __DAVIS_SYSDEP_H__ */

