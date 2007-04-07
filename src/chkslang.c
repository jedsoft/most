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
/* It is too bad that this cannot be done at the preprocessor level.
 * Unfortunately, C is not completely portable yet.  Basically the #error
 * directive is the problem.
 */
#include "config.h"

#include <stdio.h>
#ifdef VMS
# include <ssdef.h>
#endif
#include <slang.h>
#include "jdmacros.h"

static char *make_version (unsigned int v)
{
   static char v_string[16];
   unsigned int a, b, c;

   a = v/10000;
   b = (v - a * 10000) / 100;
   c = v - (a * 10000) - (b * 100);
   sprintf (v_string, "%u.%u.%u", a, b, c);
   return v_string;
}

int main (int argc, char **argv)
{
   unsigned int min_version, sl_version;
   unsigned int sug_version;
   int ret;

   if ((argc < 3) || (argc > 4))
     {
	fprintf (stderr, "Usage: %s <PGM> <SLANG-VERSION> <SUGG VERSION>\n", argv[0]);
	return 1;
     }
#ifndef SLANG_VERSION
   sl_version = 0;
#else
   sl_version = SLANG_VERSION;
#endif

   sscanf (argv[2], "%u", &min_version);
   if (argc == 4) sscanf (argv[3], "%u", &sug_version);
   else sug_version = sl_version;

   ret = 0;
   if (sl_version < min_version)
     {
	fprintf (stderr, "This version of %s requires slang version %s.\n",
		 argv[1], make_version(min_version));
#ifdef VMS
	ret = SS$_ABORT;
#else
	ret = 1;
#endif
     }

   if (sl_version < sug_version)
     {
	fprintf (stderr, "Your slang version is %s.\n", make_version(sl_version));
	fprintf (stderr, "To fully utilize this program, you should upgrade the slang library to\n");
	fprintf (stderr, "  version %s\n", make_version(sug_version));
	fprintf (stderr, "This library is available via anonymous ftp from\n\
space.mit.edu in pub/davis/slang.\n");
     }

   return ret;
}

