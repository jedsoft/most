/*
 * Use sysconf.h for unix!!!  This file is for NON-unix systems.
 *
 * This file is used to indicate capabilities of the C compiler and
 * operating system.
 */

/* Basic include files. */

#if defined(__os2__) || defined (__DECC) || defined(VAXC)
# define HAVE_STDLIB_H 1
#else
# define HAVE_MALLOC_H 1
#endif

#if defined (__os2__)
# define HAVE_UNISTD_H 1
# define HAVE_MEMORY_H 1
#endif

/*
 * Basic C library functions.
 */

#if defined(__os2__)
# define HAVE_PUTENV 1
#endif

#define HAVE_GETCWD 1
#define HAVE_MEMSET 1
#define HAVE_MEMCPY 1
#define HAVE_MEMCHR 1

#ifdef VMS
# ifndef __GNUC__
#  include <unixio.h>
# endif
#endif

#define MAX_PATHLEN 256

#define MOST_INT long
#define MOST_UINT unsigned long
#define MOST_INT_D_FMT "%ld"

#define MOST_HAS_MMAP	0
