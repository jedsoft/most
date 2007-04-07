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

/* editor functions */
#include <stdio.h>
#include <string.h>

#ifdef VMS
# include <ssdef.h>
# include <rmsdef.h>
# include <dvidef.h>
# include <jpidef.h>
/* #include <libdef.h> */
# include <descrip.h>
# include <iodef.h>
# include <ttdef.h>
#endif

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include <slang.h>

#include "most.h"
#include "edit.h"
#include "window.h"
#include "file.h"
#include "sysdep.h"


/* quote_char is one of: 0, ", or '.  It represents the character
 * that will be used to quote the filename.
 */
char *most_escape_filename (char *file, char quote_char)
{
   char *efile, *e;
   
   if (NULL == (efile = SLmalloc (5 + 2*strlen(file))))
     return NULL;

   e = efile;

#ifndef VMS
   if (quote_char == 0)
     {
	/* No quote char was used for the filename command.  Add it here. */
	*e++ = '"';
     }
   else if (quote_char == '\'')
     {
	/* Suppose the file is Bob's Notes.  With ' as the quote, when passed
	 * to the shell, it would look like 'Bob's Notes'.  Unfortunately, this
	 * would produce an unmatched single quote when processed by the shell.
	 * So, the following escaped form will be used:
	 *    ''"Bob's Notes"''
	 */
	*e++ = '\'';
	*e++ = '"';
     }
#endif

   while (1)
     {
	char ch = *file++;
	switch (ch)
	  {
	   case 0:
#ifndef VMS
	     if (quote_char == 0)
	       *e++ = '"';
	     else if (quote_char == '\'')
	       {
		  *e++ = '"';
		  *e++ = '\'';
	       }
#endif
	     *e = 0;
	     return efile;
#ifndef VMS
	     /* For a double-quoted string, only the following characters
	      * may be escaped with a backslash.  Unfortuinately, escaping 
	      * others such as &, *, |, etc will not work.
	      */
	   case '\\': case '$': case '"': case '`': case '\n':
	     *e++ = '\\';
	     *e++ = ch;
	     break;
#endif
	   default:
	     *e++ = ch;
	  }
     }
}

static int create_edit_command (char *edit, char *cmd, unsigned int sizeof_cmd, char *file)
{
   int d, s;
   char ch, *p = edit;
   char *efile;
   char quote_char = '"';
   /* Look for %d and %s */

   d = s = 0;

   while (0 != (ch = *p++))
     {
	char q = 0;

	if (((ch == '\'') || (ch == '"'))
	    && (*p == '%'))
	  {
	     q = ch;
	     ch = '%';
	     p++;
	  }
	if (ch != '%') continue;

	ch = *p++;
	if (!d && (ch == 'd'))
	  {
	     if (s == 0) d = 1; else d = 2;
	  }
	else if (!s && (ch == 's'))
	  {
	     if (d == 0) s = 1; else s = 2;
	     if (q && (q != *p))
	       {
		  most_message ("Unmatched quote character in editor definition", 1);
		  return -1;
	       }
	     quote_char = q;
	  }
	else
	  {
	     most_message ("Invalid editor definition.", 1);
	     return -1;
	  }
     }

   if (NULL == (efile = most_escape_filename (file, quote_char)))
     return -1;


   if ((d == 0) && (s == 0))
     {
	/* No %d, %s */
	_pSLsnprintf (cmd, sizeof_cmd, "%s \"%s\"", edit, efile);
	SLfree (efile);
	return 0;
     }
   
   if (d == 0)
     {
	_pSLsnprintf (cmd, sizeof_cmd, edit, efile);
	SLfree (efile);
	return 0;
     }

   if (d && s)
     {
	if (d == 1)
	  _pSLsnprintf (cmd, sizeof_cmd, edit, Most_C_Line, efile);
	else 
	  _pSLsnprintf (cmd, sizeof_cmd, edit, efile, Most_C_Line);
	
	SLfree (efile);
	return 0;
     }

   most_message ("Unsupported Editor definition", 1);
   SLfree (efile);
   return -1;
}

#ifdef VMS
int call_edt_tpu(int tpu, char *file)
{
   char the_file[MAX_PATHLEN], *strp;
   extern void edt$edit();
   extern void tpu$tpu();
   struct dsc$descriptor_s  file_desc;

   if (tpu == 1)  /* tpu */
     _pSLsnprintf (the_file, sizeof(the_file), "TPU /START=%d ", Most_C_Line);
   else
     the_file[0] = '\0';

   strcat(the_file, file);

   /*  lose the version number */
   strp = the_file;
   while((*strp != '\0') && (*strp != ';')) strp++;
   *strp = '\0';

   file_desc.dsc$w_length = strlen(the_file);
   file_desc.dsc$a_pointer = the_file;
   file_desc.dsc$b_class = DSC$K_CLASS_S; /* scalar, string type */
   file_desc.dsc$b_dtype = DSC$K_DTYPE_T; /* ascii string */

   if (tpu == 1)
     tpu$tpu(&file_desc);
   else
     edt$edit(&file_desc);
   
   return 1;
}
#endif

void most_edit_cmd(void)
{
   char *editor;
   char cmd[2*MAX_PATHLEN + 30];
#ifdef VMS
   int tpu = -1;
#endif
   char *file;
   
   file = Most_Buf->file;

   if ((0 == *file) || ('*' == *file))
     return;

   if (Most_Secure_Mode)
     {
	most_message ("Editing not permitted in secure mode.", 1);
	return;
     }

   if ((NULL == (editor = getenv("MOST_EDITOR")))
       && (NULL == (editor = getenv("SLANG_EDITOR")))
       && (NULL == (editor = getenv("EDITOR"))))
#ifdef VMS
     editor = "EDT";
#else
   editor = "vi";
#endif

#ifdef VMS
   if (!strcmp(editor,"EDT")) tpu = 0;
   else if (!strcmp(editor,"TPU")) tpu = 1;
   else
#endif
     if (-1 == create_edit_command(editor, cmd, sizeof(cmd), file))
       return;

   most_reset_tty ();
   most_reset_display ();

#ifdef VMS
   if (tpu != -1) (void) call_edt_tpu(tpu, file);
   else
#endif
     SLsystem (cmd);

   most_reread_file ();
   most_init_tty ();
   most_init_display ();
   most_redraw_display ();
}

