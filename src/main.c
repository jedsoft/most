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

#include <slang.h>
#include "most.h"

int main(int argc, char *argv[])
{
#if 0
   static volatile int debug_me = 1;
   while (debug_me == 1)
     {
	sleep (1);
     }
#endif
   if (argc > 1)
     {
	if ((0 == strcmp (argv[1], "--version"))
	    || (0 == strcmp (argv[1], "--help")))
	  {
	     most_usage ();
	     exit (1);
	  }
     }
   
   most_initialize_most ();
   return most (argc, argv);
}

