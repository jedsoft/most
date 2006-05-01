$!  Most Makefile for VMS
$!
$! S-Lang Include directory (where slang.olb is located)
$!
$  Link_Flags = ""
$  Link_Libs = ""
$  slang_dir = "[-.-.slang.src]"
$  slang_lib = ",''slang_dir'slang/LIBR"
$  defs = ""
$  include = slang_dir
$!
$! If compiling with GCC, uncomment next line
$! goto has_gcc
$ if (p1 .eqs. "GCC") then goto has_gcc
$!
$! There should be no need to change anything below here
$!
$ C_C = "cc/standard=vaxc"
$ goto start
$ has_gcc:
$  C_C = "gcc/warnings/nocase_hack"
$  link_libs = "GNU_CC:[000000]GCCLIB/LIB"
$!
$ start:
$ copy mostconf.h config.h
$ axp = 0
$ axp = f$getsyi("HW_MODEL").ge.1024
$ exec_name = "most.exe"
$ files = "main,buffer,file,window,line,display,sysdep,"
$ files = files + "keym,most,search,help,cmd,edit,keyparse"
$!
$  if (p1 .eqs. "LINK") then goto do_link
$!
$!  simple make
$!
$  if (defs .nes. "") then defs = "/define=(" + defs + ")"
$  if (include .nes. "") then include = "/include=(" + include + ")"
$  count = 0
$  next_file:
$    f = f$element(count, ",", files)
$    count = count + 1
$    if (f .eqs. ",") then goto do_link
$    objf = f$search("''f'.obj")
$    if (objf .eqs. "") then goto compile_it
$    tobj = f$file_attr(objf, "RDT")
$    tc  = f$file_attr("''f'.c", "RDT")
$    if (f$cvtime(tc) .lts. f$cvtime(tobj)) then goto next_file
$  compile_it:
$  write sys$output "''C_C' ''defs' ''include' ''f'.c"
$      'C_C' 'defs' 'include' 'f'.c
$    goto next_file
$  do_link:
$  if axp
$  then
$    set verify
$    link/exec=most.exe 'Link_Flags' 'files', 'slang_dir'slang/LIBR 'Link_Libs'
$    set noverify
$  else
$    set verify
$    link/exec=most.exe 'Link_Flags' 'files', 'slang_dir'slang/LIB 'Link_Libs',  -
       sys$input/opt
     SYS$LIBRARY:VAXCRTL/SHARE
$    set noverify
$  endif
$exit
