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
#ifndef _DAVIS_BUFFER_H_
# define _DAVIS_BUFFER_H_

typedef struct
{
   char file[MAX_PATHLEN];            /* filename */
   unsigned char *beg;       /* beginning of buffer */
   unsigned char *end;       /* end of buffer */
   int mark;                 /* marked line in buffer */
   unsigned int flags;
   int fd;			       /* file descriptor--- -1 if closed */
   FILE *fp;			       /* use pclose when EOF is reached */
   int size;
# ifdef VMS
   int rec;			       /* record size for reads */
# endif
# if MOST_HAS_MMAP
   int is_mmaped;
   unsigned int mmap_size;
# endif
}
Most_Buffer_Type;

extern Most_Buffer_Type *Most_Buf;
extern unsigned char *Most_Beg, *Most_Eob;
extern MOST_INT Most_Num_Lines;

/* Offset of current position from beginning of buffer */
extern MOST_UINT Most_C_Offset;

extern MOST_INT Most_C_Line;
/*
 *  Current line number.  If at the beginning of the buffer, it is 1.  If
 *  we are at the last point of the buffer it is the number of lines.
 */

/* This routine does not move the point */
extern unsigned char *most_beg_of_line(void);

extern MOST_INT most_forward_line(MOST_INT);
/* This routine moves the point forward n lines. n can be negative.
   It returns the number moved. */

extern void most_goto_line (MOST_INT);
/* Move the point somewhere on the nth line of the buffer returning
   C_POS */

extern MOST_INT most_what_line (unsigned char *);
/* return the line number of position 'argument'. Does not move point */

/* count the number of lines in the region delimited by beg and end.
   Counts lines from beg up to end but does not count end.
   Does not move point. */
extern MOST_INT most_count_lines(unsigned char *, unsigned char *);
extern int most_extract_line(unsigned char **, unsigned char **);

extern Most_Buffer_Type *most_switch_to_buffer(Most_Buffer_Type *);
extern Most_Buffer_Type *most_create_buffer(char *);
extern void most_find_row_column(unsigned char *, MOST_INT *, MOST_INT *);

#endif

