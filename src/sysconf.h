/* src/sysconf.h.  Generated from config.hin by configure.  */
/* -*- C -*- */

/* Note: this is for authentic unix systems only.  
 * See mostconf.h for other systems.
 */

#ifndef MOST_CONFIG_H
#define MOST_CONFIG_H

/* Define if you want the MMAP support */
#define USE_MMAP 1

/* define if you have long long type */
#define HAVE_LONG_LONG 1

/* #undef off_t */
/* #undef size_t */
#define SIZEOF_OFF_T 8
#define SIZEOF_SHORT 2
#define SIZEOF_INT 4
#define SIZEOF_LONG 4
#define SIZEOF_FLOAT 4
#define SIZEOF_DOUBLE 8
#define SIZEOF_LONG_LONG 8

/* The following set defines may be necessary to activate long file support */
#define _FILE_OFFSET_BITS 64
/* #undef _LARGE_FILES */
#define _LARGEFILE_SOURCE 1

/* define if you have stdlib.h */
#define HAVE_STDLIB_H 1

/* define if you have unistd.h */
#define HAVE_UNISTD_H 1

/* define if you have memory.h */
#define HAVE_MEMORY_H 1

/* define if you have malloc.h */
#define HAVE_MALLOC_H 1

/* define if you have memset */
#define HAVE_MEMSET 1

/* define if you have memcpy */
#define HAVE_MEMCPY 1

/* define if you have this. */
#define HAVE_GETCWD 1

#define HAVE_MMAP 1

#define HAVE_TCGETPGRP 1
#define HAVE_GETPGRP 1

#define HAVE_DIRENT_H 1
/* #undef HAVE_SYS_NDIR_H */
/* #undef HAVE_SYS_DIR_H */
/* #undef HAVE_NDIR_H */

#define HAVE_SNPRINTF 1
#define HAVE_SYS_MMAN_H 1

/* #undef mode_t */
/* #undef pid_t */
/* #undef uid_t */
/* #undef pid_t */


#ifdef _AIX
# ifndef _POSIX_SOURCE
#  define _POSIX_SOURCE
# endif
# ifndef _ALL_SOURCE
#  define _ALL_SOURCE
# endif
/* This may generate warnings but the fact is that without it, xlc will 
 * INCORRECTLY inline many str* functions. */
/* # undef __STR__ */
#endif

#define MAX_PATHLEN 1024
#if defined(USE_MMAP) && defined(HAVE_MMAP) && defined(HAVE_SYS_MMAN_H)
# define MOST_HAS_MMAP	1
#else
# define MOST_HAS_MMAP	0
#endif

#if defined(HAVE_LONG_LONG) && (SIZEOF_OFF_T == SIZEOF_LONG_LONG) && (SIZEOF_LONG_LONG > SIZEOF_LONG)
typedef long long MOST_INT;
typedef unsigned long long MOST_UINT;
# define MOST_INT_D_FMT "%lld"
#else
# if (SIZEOF_OFF_T == SIZEOF_INT)
typedef int MOST_INT;
typedef unsigned int MOST_UINT;
#  define MOST_INT_D_FMT "%d"
# else
typedef long MOST_INT;
typedef unsigned long MOST_UINT;
#  define MOST_INT_D_FMT "%ld"
# endif
#endif

#endif /* MOST_CONFIG_H */
