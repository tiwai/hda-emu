AC_PREREQ(2.59)
AC_INIT(hda-emu.c)
AM_INIT_AUTOMAKE(hda-emu, 0.0.2)

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_DISABLE_SHARED
AM_PROG_LIBTOOL
AC_HEADER_STDC

WFLAGS="-W -Wimplicit -Wstrict-prototypes -Wextra -Wno-sign-compare -Wno-sign-conversion -Wno-missing-field-initializers -Wno-unused"
OPTFLAGS="-O2 -g"
CFLAGS="$CFLAGS $OPTFLAGS $WFLAGS"

AC_OUTPUT(Makefile kernel/Makefile include/Makefile)
