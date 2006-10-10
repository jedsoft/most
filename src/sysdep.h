#ifndef __DAVIS_SYSDEP_H__
# define __DAVIS_SYSDEP_H__

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

