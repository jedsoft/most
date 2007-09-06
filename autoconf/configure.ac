dnl -*- sh -*-
AC_INIT(src/most.c)
JD_INIT
JD_ANSI_CC

AC_CONFIG_AUX_DIR(autoconf)
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_CANONICAL_HOST

JD_ELF_COMPILER

# determine X include and needed libs
AC_PATH_XTRA

# Implementation of the --disable-dev-random switch.
AC_MSG_CHECKING([if you want to use mmap if available])
AC_ARG_ENABLE(mmap,
   AC_HELP_STRING([--disable-mmap],[disable the use of mmap]),
   jd_use_mmap=$enableval, jd_use_mmap=yes)
AC_MSG_RESULT($jd_use_mmap)
case "x$jd_use_mmap" in
xyes )
  AC_DEFINE(USE_MMAP)
  ;;
esac

#Check these header since they cause trouble
AC_CHECK_HEADERS( \
stdlib.h \
unistd.h \
memory.h \
malloc.h \
termios.h \
sys/mman.h \
)

AC_HEADER_DIRENT

AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_UID_T
AC_TYPE_SIZE_T

AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(float, 4)
AC_CHECK_SIZEOF(double, 8)

JD_CHECK_LONG_LONG
JD_LARGE_FILE_SUPPORT

AC_CHECK_FUNCS(\
memset \
memcpy \
putenv \
getcwd \
mmap \
snprintf \
getpgrp tcgetpgrp \
)

JD_SET_OBJ_SRC_DIR
JD_TERMCAP
JD_WITH_LIBRARY(slang)
JD_GCC_WARNINGS
JD_GET_MODULES(src/modules.lis)
JD_CREATE_MODULE_ORULES

AC_CONFIG_HEADER(src/sysconf.h:src/config.hin)
AC_OUTPUT(Makefile:autoconf/Makefile.in src/Makefile)

JD_CREATE_ORULE(chkslang)
JD_APPEND_RULES(src/Makefile)

