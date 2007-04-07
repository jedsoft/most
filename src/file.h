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
#define MOST_MAX_FILES 4096
#define MOST_MAX_FILES 4096
#define MOST_GUNZIP_POPEN_FORMAT "gzip -dc \"%s\""
#define MOST_BZIP2_POPEN_FORMAT "bzip2 -dc \"%s\""

extern void most_reread_file (void);
extern void most_read_to_line (int);
extern Most_Window_Type *most_file_visible (char *);
extern void most_user_get_file(void);
extern int most_read_file_dsc(int);
extern void most_get_cdir(char *);
extern int most_get_dir(char *);
extern void most_do_next_file(int *);
extern int most_find_file(char *);
extern int most_head(char *, char *);

extern char *Most_File_Ring[MOST_MAX_FILES];
extern int Most_Num_Files;
extern char Most_C_Dir[MAX_PATHLEN];
extern int Most_Tail_Mode;
extern int most_close_buffer_file (Most_Buffer_Type *);
