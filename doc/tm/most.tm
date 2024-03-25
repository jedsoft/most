#i docbook_man.tm

#d man_options_entry#2 \varlistentry{\term{$1}}{\p $2 \p-end}\__newline__

#d arg#1 <arg>$1</arg>\__newline__
#d desc-list \variablelist
#d desc-item#2 \varlistentry{\term{$1}}{\p $2 \p-end}\__newline__
#d desc-item-continue <varlistentry><term></term><listitem>\p
#d desc-item-continue-end </listitem></varlistentry>
#d desc-list-end \variablelist-end

#d envdesc#2 \varlistentry{\term{$1}}{\p $2 \p-end}\__newline__
#d envdesc-begin \variablelist
#d envdesc-end \variablelist-end
#d envdesc-verb-begin <varlistentry><term></term><listitem>
#d envdesc-verb-end </listitem></varlistentry>

#d most \command{most}
#d slang \literal{S-Lang}
#d lit#1 \literal{$1}
#d underline#1 <emphasis role="underline">$1</emphasis>
#d i \underline{i}

\manpage{most}{1}{browse or page through a text file}
\mansynopsis{most}{
  \arg{\option{-1}}
  \arg{\option{-b}}
  \arg{\option{-C}}
  \arg{\option{-c}}
  \arg{\option{-d}}
  \arg{\option{-M}}
  \arg{\option{-r}}
  \arg{\option{-s}}
  \arg{\option{-t}}
  \arg{\option{-u}}
  \arg{\option{-v}}
  \arg{\option{-w}}
  \arg{\option{-z}}
  \arg{\option{+/\replaceable{string}}}
  \arg{\option{+\replaceable{line-number}}}
  \arg{\option{+d}}
  \arg{\option{+s}}
  \arg{\option{+u}}
  \arg{\option{\replaceable{file...}}}
}

\refsect1{DESCRIPTION}
  \p
  \most is a paging program that displays, one windowful at a time,
  the contents of a file on a terminal.  It pauses after each
  windowful and prints on the window status line the screen the file
  name, current line number, and the percentage of the file so far
  displayed.
  \pp
  Unlike other paging programs, \most is capable of displaying an
  arbitrary number of windows as long as each window occupies at least
  two screen lines.  Each window may contain the same file or a
  different file.  In addition, each window has its own mode.  For
  example, one window may display a file with its lines wrapped while
  another may be truncating the lines. Windows may be `locked'
  together in the sense that if one of the locked windows scrolls, all
  locked windows will scroll.  \most is also capable of ignoring lines
  that are indented beyond a user specified value.  This is useful
  when viewing computer programs to pick out gross features of the
  code.  See the `:o' command for a description of this feature.
\pp
  In addition to displaying ordinary text files, \most can also
  display binary files as well as files with arbitrary ascii
  characters.  When a file is read into a buffer, \most examines the
  first 32 bytes of the file to determine if the file is a binary file
  and then switches to the appropriate mode.  However, this feature
  may be disabled with the -k option.  See the description of the -b,
  -k, -v, and -t options for further details.
\pp
  Text files may contain combinations of underscore and backspace
  characters causing a printer to underline or overstrike.  When \most
  recognizes this, it inserts the appropriate escape sequences to
  achieve the desired effect.  In addition, some files cause the
  printer to overstrike some characters by embedding carriage return
  characters in the middle of a line.  When this occurs, \most displays
  the overstruck character with a bold attribute.  This feature
  facilitates the reading of UNIX man pages or a document produced by
  runoff.  In particular, viewing this document with \most should
  illustrate this behavior provided that the underline characters
  have not been stripped.  This may be turned off with the -v option.
\pp
  By default, lines with more characters than the terminal width are
  not wrapped but are instead truncated. When truncation occurs, this
  is indicated by a `$' in the far right column of the terminal
  screen.  The RIGHT and LEFT arrow keys may be used to view lines
  which extend past the margins of the screen.  The -w option may be
  used to override this feature.  When a window is wrapped, the
  character `\\' will appear at the right edge of the window.
\pp
  Commands are listed below.
\p-end
\refsect1-end

\refsect1{COLOR SUPPORT}
\p
  \most has supported both 256-color and 24 bit truecolor terminals
  since version 5.2.  Not all terminals are capable of generating
  arbitrary 24 bit colors. If your terminal supports 24 bit colors,
  but \most does not detect it, then set the environment variable
#v+
     COLORTERM=truecolor
#v-
\pp
  to force 24 bit truecolors to be used.
\p
\refsect1-end

\refsect1{OPTIONS}
 \variablelist

  \man_options_entry{\option{-1}}{VT100 mode.  This is meaningful only
  on VMS systems.  This option should be used if the terminal is
  strictly a VT100.  This implies that the terminal does not have the
  ability to delete and insert multiple lines.  VT102s and above have
  this ability.}

  \man_options_entry{\option{-b}}{ Binary mode.  Use this switch when
  you want to view files containing 8 bit characters.  \most will
  display the file 16 bytes per line in hexadecimal notation. A
  typical line looks like:
#v+
   01000000 40001575 9C23A020 4000168D     ....@..u.#. @...
#v-
  \pp
  When used with the -v option, the same line looks like:
#v+
   ^A^@^@^@  @^@^U u 9C #A0    @^@^V8D     ....@..u.#. @...
#v-
  }

  \man_options_entry{\option{-C}}{Disable color support.}
  \man_options_entry{\option{-c}}{Make searches case-sensitive}
  \man_options_entry{\option{-d}}{Omit the backslash mark used to denote a wrapped line.}
  \man_options_entry{\option{-M}}{Disable the use of mmap.}
  \man_options_entry{\option{-r}}{Default to using regexp searches}
  \man_options_entry{\option{-s}}{Squeeze-mode.  Replace multiple blank
  lines with a single blank line.}

  \man_options_entry{\option{-t}}{Display tabs as ^I.  If this option
  is immediately followed by an integer, the integer sets the tab
  width, e.g., -t4}

  \man_options_entry{\option{-u}}{Disable UTF-8 mode even if the
  locale dictates it}

  \man_options_entry{\option{+u}}{Force UTF-8 mode.  By default \most
  will use the current locale to determine if UTF-8 mode should be
  used.  The +u and -u switches allow the behavior to be overridden}

  \man_options_entry{\option{-v}}{Display control characters as in
  `^A' for control A.  Normally \most does not interpret control
  characters.}
  \man_options_entry{\option{-w}}{Wrap lines}
  \man_options_entry{\option{-z}}{Disable gunzip-on-the-fly}

  \man_options_entry{\option{+/\replaceable{string}}}{Start up at the
  line containing the first occurrence of string}

  \man_options_entry{\option{+\replaceable{lineno}}}{Start up at the
  specified line-number}

  \man_options_entry{\option{+d}}{This switch should only be used if
    you want the option to delete a file while viewing it.  This makes
    it easier to clean unwanted files out of a directory. The file is
    deleted with the interactive key sequence `:D' and then confirming
    with `y'.}

  \man_options_entry{\option{+s}}{Secure Mode-- no edit, cd, shell,
  and reading files not already listed on the command line.}

 \variablelist-end
\refsect1-end

\refsect1{COMMAND USAGE}
\p
   The commands take effect immediately; it is not necessary to type a
   carriage return.  In the following commands, \i is a numerical
   argument (1 by default).
\desc-list
\desc-item{SPACE, CTRL-D, NEXT_SCREEN}{
   Display another windowful, or jump \i windowfuls if \i is specified.}
\desc-item{RETURN, DOWN_ARROW, V, CTRL-N}{
Display another line, or \i more lines, if specified.}

\desc-item{UP_ARROW, ^, CTRL-P}{Display previous line, or \i previous
lines, if specified.}

\desc-item{T, ESCAPE<}{Move to top of buffer.}

\desc-item{B, ESCAPE>}{Move to bottom of buffer.}

\desc-item{RIGHT_ARROW, TAB, >}{Scroll window left 60\i columns to view
lines that are beyond the right margin of the window.}

\desc-item{LEFT_ARROW, CTRL-B, <}{Scroll window right 60\i columns to
view lines that are beyond the left margin of the window.}

\desc-item{U, CTRL-U, DELETE, PREV_SCREEN}{Skip back \i windowfuls and
then print a windowful.}

\desc-item{R, CTRL-R}{Redraw the window.}

\desc-item{J, G}{If  \i  is  not  specified, then prompt for a line
number then jump to that line otherwise just jump to line i.}

\desc-item{%}{If \i is not specified, then prompt for a percent number
then jump to that percent of the file otherwise just jump to \i percent
of the file.}

\desc-item{W, w}{If  the  current  screen  width  is 80, make it 132 and
vice-versa.  For other values, this command is ignored.}

\desc-item{Q, CTRL-X CTRL-C, CTRL-K E}{Exit from \most.  On VMS, ^Z also
exits.}

\desc-item{h, CTRL-H, HELP, PF2}{Help.  Give a description of all the
\most commands.  The \most environment variable MOST_HELP must be set
for this to be meaningful.}

\desc-item{f, /, CTRL-F, FIND, GOLD PF3}{Prompt  for  a  string  and
search forward from the current line for ith distinct line containing
the string.  CTRL-G aborts.}

\desc-item{?}{Prompt for a string and search backward for the ith
distinct line containing the string.  CTRL-G aborts.}

\desc-item{n}{Search for the next \i lines containing an occurrence of
the last search string in the direction of the previous search.}

\desc-item{m, SELECT, CTRL-@, CTRL-K M, PERIOD}{Set a mark on the
current line for later reference.}

\desc-item{INSERT_HERE, CTRL-X CTRL-X, COMMA, CTRL-K RETURN, GOLD
PERIOD}{Set a mark on the current line but return to previous mark. 
This allows the user to toggle back and forth between two positions in
the file.}

\desc-item{l, L}{Toggle locking for this window.  The window is locked
if there is a `*' at the left edge of the status line.  Windows locked
together, scroll together.}

\desc-item{CTRL-X 2, CTRL-W 2, GOLD X}{Split this window in half.}

\desc-item{CTRL-X o, CTRL-W o, o, GOLDUP, GOLDDOWN}{Move to other window.}

\desc-item{CTRL-X 0, CTRL-W 0, GOLD V}{Delete this window.}

\desc-item{CTRL-X 1, CTRL-W 1, GOLD O}{Delete all other windows, leaving
only one window.}

\desc-item{E, e}{Edit this file.}

\desc-item{$, ESC $}{This is system dependent.  On VMS, this causes \most
to spawn a subprocess.  When the user exits the process, \most is
resumed.  On UNIX systems, \most simply suspends itself.}

\desc-item{:n}{Skip to the next filename given in the command line.  Use
the arrow keys to scroll forward or backward through the file list.
`Q' quits \most and any other key selects the given file.}

\desc-item{:c}{Toggle case sensitive search.}

\desc-item{:D}{Delete current file.  This command is only meaningful
with the +d switch.}

\desc-item{:o, :O}{Toggle various options.  With this key sequence, \most
displays a prompt asking the user to hit one of: bdtvw.  The `b', `t',
`v', and `w' options have the same meaning as the command line
switches.  For example, the `w' option will toggle wrapping on and off
for the current window.
\p
The `d' option must be used with a prefix integer i.  All lines
indented beyond \i columns will not be displayed.  For example,
consider the fragment:}
\desc-item-continue
#v+
   int main(int argc, char **argv)
   {
     int i;
     for (i = 0; i < argc, i++)
       {
         fprintf(stdout,"%i: %s\n",i,argv[i]);
       }
     return 0;
   }
#v-
The key sequence `1:od' will cause \most to display the file ignoring
all lines indented beyond the first column.  So for the example above,
\most would display:
#v+
   int main(int argc, char **argv)...
   }
#v-
where the `...' indicates lines that follow are not displayed.
\desc-item-continue-end
\desc-item{F}{Toggles follow mode, shows new lines in the file (like
tail -f)}
\desc-list-end
\p-end
\refsect1-end

\refsect1{HINTS}
\p
  CTRL-G aborts the commands requiring the user to type something in
  at a prompt.  The back-quote key has a special meaning here.  It is
  used to quote certain characters.  This is useful when search for
  the occurrence of a string with a control character or a string at
  the beginning of a line.  In the latter case, to find the occurrence
  of `The' at the beginning of a line, enter `^JThe where ` quotes the
  CTRL-J.
\p-end
\refsect1-end

\refsect1{ENVIRONMENT}
\p
  \most uses the following environment variables:
\desc-list
\desc-item{MOST_SWITCHES}{This  variable  sets  commonly used switches.
For example, some people prefer to use \most with the -s option so that
excess blank lines are not displayed.  On VMS this is normally
done in the login.com through the line:}
\desc-item-continue
#v+
   $ define MOST_SWITCHES "-s"
#v-
\desc-item-continue-end

\desc-item{MOST_EDITOR, SLANG_EDITOR}{Either  of  these environment
variables specify an editor for \most to invoke to edit a file. The
value can contain %s and %d formatting descriptors that represent the
file name and line number, respectively.  For example, if JED is
your editor, then set MOST_EDITOR to 'jed %s -g %d'.}

\desc-item{MOST_HELP}{This variable may be used to specify an alternate
help file.}

\desc-item{MOST_INITFILE}{Set this variable to specify the
initialization file to load during startup.  The default action is to
load the system configuration file and then a personal configuration
file called .mostrc on Unix, and most.rc on other systems.}
\desc-list-end
\refsect1-end

\refsect1{CONFIGURATION FILE SYNTAX}
\p
When \most starts up, it tries to read a system configuration
file (located at /etc/most.conf) and then a personal configuration
file. These files may be used to specify key-bindings and colors.
\pp
To bind a key to a particular function use the syntax:
#v+
    setkey function-name key-sequence
#v-
\pp
The setkey command requires two arguments.  The function-name argument
specifies the function that is to be executed as a response to the
keys specified by the key-sequence argument are pressed.  For example,
#v+
    setkey   "up"     "^P"
#v-
\pp
indicates that when Ctrl-P is pressed then the function up is to be executed.
\pp
Sometimes, it is necessary to first unbind a key-sequence before
rebinding it in order via the unsetkey function:
#v+
    unsetkey "^F"
#v-
\pp
Colors may be defined through the use of the color keyword in the
configuration file using the syntax:
#v+
    color OBJECT-NAME FOREGROUND-COLOR BACKGROUND-COLOR
#v-
\pp
Here, OBJECT-NAME can be any one of the following items:
#v+
     status           -- the status line
     underline        -- underlined text
     overstrike       -- overstruck text
     normal           -- anything else
#v-
\pp
See the sample configuration files for more information.
\p-end
\refsect1-end

\refsect1{BUGS}
\p
Almost all of the known bugs or limitations of \most are due to a
desire to read and interpret control characters in files.  One
problem concerns the use of backspace characters to underscore or
overstrike other characters.  \most makes an attempt to use terminal
escape sequences to simulate this behavior.  One side effect is the
one does not always get what one expects when scrolling right and left
through a file.  When in doubt, use the -v and -b options of \most.
\pp
The regular-expression searches may fail to find strings that involve
backspace/underscore used for highlighting.  The regular-expression
syntax is described in the S-Lang Library documentation.
\pp
To report bugs or open issues, please visit
https://github.com/jedsoft/most/issues.
\p-end
\refsect1-end

\refsect1{AUTHOR}
\p
  John E. Davis \email{jed@jedsoft.org}
\p-end
\refsect1{ACKNOWLEDGEMENTS}
\p
Over the years, many people have contributed to \most in one way or
another, e.g., via code patches, bug-fixes, comments, or criticisms.
I am particularly grateful to the very early adopters of the program
who took a chance with a fledgling software project headed by someone
learning the underlying language.  These include: 
\pp
  Mats Akerberg, Henk D. Davids, Rex O. Livingston, and Mark Pizzolato
  contributed to the early VMS versions of \most.  In particular, Mark
  worked on it to get it ready for DECUS.
\pp
  Foteos Macrides adapted \most for use in cswing and gopher.  A few
  features of the present version of \most was inspired from his work.
\pp
  I am grateful to Robert Mills for re-writing the search routines to
  use regular expressions.
\pp
  Sven Oliver Moll came up with the idea of automatic detection of
  zipped files.
\pp
  I would also like to thank Shinichi Hama for his valuable criticisms
  of \most.
\pp
  Javier Kohen was instrumental in the support for UTF-8.
\pp
  Thanks to David W. Sanderson for adapting the early documentation to
  nroff man page source format.
\p-end
\refsect1-end
\manpage-end
