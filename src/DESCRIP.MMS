#
#  MMS file to build MOST.
#
#  Written by Hunter Goatley <goathunter@WKUVX1.BITNET> ( 4-OCT-1993 15:41)
#  Removed reference to DIR.C, Michael Lemke <ai26@alf001.sternwarte.uni-erlangen.de> ( 3-NOV-1995 17:25:44.05)
#  Moved CC =.. into if clause.  New MMS doesn't like redefinition of macros.
#   6-AUG-1997 14:43:25.04 Michael Lemke <ai26@alf001.sternwarte.uni-erlangen.de> ( 3-NOV-1995 17:25:44.05)
#
.IFDEF __MATTS_MMS__
.ELSE
EXE = .EXE
OBJ = .OBJ
.ENDIF


.IFDEF __ALPHA__
CC = CC/STANDARD=VAXC
OPTFILE =
OPTIONS =
NATIVE  = /NONATIVE_ONLY
.ELSE
CC = CC/STANDARD/VAXC
OPTFILE = ,VAXCRTL.OPT
OPTIONS = $(OPTFILE)/OPTIONS
NATIVE  =
.ENDIF

.IFDEF SLANG_OLB
.ELSE
SLANG_DIR = [-.SLANG.SRC]
SLANG_OLB = $(SLANG_DIR)SLANG$(OLB)
.ENDIF

CFLAGS = $(CFLAGS)/DEFINE=(USE_SLANG)/INCLUDE=$(SLANG_DIR)
LINKFLAGS = $(LINKFLAGS)$(NATIVE)/NOTRACE

OBJS =	MAIN$(OBJ),-
	BUFFER$(OBJ),-
	FILE$(OBJ),-
	WINDOW$(OBJ),-
	LINE$(OBJ),-
	DISPLAY$(OBJ),-
	SYSDEP$(OBJ),-
	KEYM$(OBJ),-
	MOST$(OBJ),-
	SEARCH$(OBJ),-
	HELP$(OBJ),-
	CMD$(OBJ),-
	EDIT$(OBJ),-
	KEYPARSE$(OBJ)
#	DIR$(OBJ),-

MOST$(EXE) :	config.h $(OBJS)$(OPTFILE)
	$(LINK)$(LINKFLAGS) $(OBJS)$(OPTIONS),$(SLANG_OLB)/LIBRARY

config.h : mostconf.h
	copy mostconf.h config.h

#
#  Build the linker options file for OpenVMS VAX and VAX C.
#
VAXCRTL.OPT :
        @ open/write tmp vaxcrtl.opt
        @ write tmp "SYS$SHARE:VAXCRTL.EXE/SHARE"
        @ close tmp



