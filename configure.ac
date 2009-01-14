AC_PREREQ(2.59)
AC_INIT(hda-emu.c)
AM_INIT_AUTOMAKE(hda-emu, 0.0.4)

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_DISABLE_SHARED
AM_PROG_LIBTOOL
AC_HEADER_STDC

WFLAGS="-W -Wimplicit -Wstrict-prototypes -Wextra -Wno-sign-compare -Wno-sign-conversion -Wno-missing-field-initializers -Wno-unused"
OPTFLAGS="-O2 -g"
CFLAGS="$CFLAGS $OPTFLAGS $WFLAGS"

AC_MSG_CHECKING(for old power_save option)
OLD_POWER_SAVE=
if grep -q '^static int power_save' kernel/hda_codec.c; then
  AC_MSG_RESULT(yes)
  OLD_POWER_SAVE=1
else
  AC_MSG_RESULT(no)
fi
if test "$OLD_POWER_SAVE" = "1"; then
  AC_DEFINE(OLD_POWER_SAVE)
fi

AC_MSG_CHECKING(for old bus callbacks)
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Iinclude"
OLD_HDA_CMD=
AC_TRY_COMPILE([
#include "hda-types.h"
#include "hda-log.h"
#include <sound/core.h>
#include <sound/pcm.h>
#include "kernel/hda_codec.h"
],[
  extern struct hda_bus_ops *ops;
  ops->command(NULL, 0, 0, 0, 0);
],
  [AC_MSG_RESULT(yes); OLD_HDA_CMD="1"],
  [AC_MSG_RESULT(no)],
  [AC_MSG_RESULT(unknown)]
)
CFLAGS=$ac_save_CFLAGS
if test "$OLD_HDA_CMD" = "1"; then
  AC_DEFINE(OLD_HDA_CMD)
fi

AC_MSG_CHECKING(for old snd_hda_codec_new)
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Iinclude"
OLD_HDA_CODEC_NEW=
AC_TRY_COMPILE([
#include "hda-types.h"
#include "hda-log.h"
#include <sound/core.h>
#include <sound/pcm.h>
#include "kernel/hda_codec.h"
],[
  int err = snd_hda_codec_new(NULL, 0, NULL);
],
  [AC_MSG_RESULT(yes); OLD_HDA_CODEC_NEW="1"],
  [AC_MSG_RESULT(no)],
  [AC_MSG_RESULT(unknown)]
)
CFLAGS=$ac_save_CFLAGS
if test "$OLD_HDA_CODEC_NEW" = "1"; then
  AC_DEFINE(OLD_HDA_CODEC_NEW)
fi

AC_OUTPUT(Makefile kernel/Makefile include/Makefile)

