/* -*- C -*- */

/* Note: this is for DJGPP and WIN32 systems only.  
 * See mostconf.h for other systems, e.g., Unix.
 */

#ifndef MOST_CONFIG_H
# define MOST_CONFIG_H

/* define if you have stdlib.h */
# define HAVE_STDLIB_H 1

/* define if you have unistd.h */
# define HAVE_UNISTD_H 1

/* define if you have memory.h */
# define HAVE_MEMORY_H 1

/* define if you have malloc.h */
# define HAVE_MALLOC_H 1

/* define if you have memset */
# define HAVE_MEMSET 1

/* define if you have memcpy */
# define HAVE_MEMCPY 1

/* define if you have this. */
# define HAVE_GETCWD 1

# define HAVE_DIRENT_H 1

# define MAX_PATHLEN 1024

#endif /* MOST_CONFIG_H */
