AC_PREREQ(2.59)
AC_INIT(hda-emu.c)
AM_INIT_AUTOMAKE(hda-emu, 0.2.5)

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_DISABLE_SHARED
AM_PROG_LIBTOOL
AC_HEADER_STDC

WFLAGS="-W -Wimplicit -Wstrict-prototypes -Wextra -Wno-sign-compare -Wno-sign-conversion -Wno-missing-field-initializers -Wno-unused"
OPTFLAGS="-O2 -g"
CFLAGS="$CFLAGS $OPTFLAGS $WFLAGS"

AC_ARG_ENABLE(old-workq,
  AS_HELP_STRING([--enable-old-workq], [use old workqueue API]),
  old_workq="$enableval", old_workq="no")
if test "$old_workq" = "yes"; then
  AC_DEFINE(OLD_WORKQUEUE)
fi

AC_ARG_WITH(hdadir,
  AS_HELP_STRING([--with-hdadir=dir],
    [path where HD-audio kernel files are stored]),
  hdadir="$withval", hdadir="")

if test -n "$hdadir"; then

test -d "$hdadir" || \
  AC_ERROR([Invalid HD-audio directory: $hdadir])
test -f "$hdadir/hda_codec.c" || \
  AC_ERROR([Invalid HD-audio directory: $hdadir])

rm -f kernel/hda_*.h kernel/hda_*.c kernel/patch_*.c
ln -s $hdadir/hda_codec.c kernel
ln -s $hdadir/hda_codec.h kernel
ln -s $hdadir/hda_local.h kernel
ln -s $hdadir/hda_generic.c kernel

AC_ARG_ENABLE(own-proc,
  AS_HELP_STRING([--enable-own-proc],
    [use own proc-file output function]),
  own_proc="$enableval", own_proc="no")
if test "$own_proc" = "yes"; then
  rm -f kernel/hda_proc.c
  ln -s ../hda_own_proc.c kernel/hda_proc.c
else
  ln -s $hdadir/hda_proc.c kernel
fi

else
dnl check existing kernel codes

if ! test -f kernel/hda_codec.c; then
  AC_ERROR([No HD-audio driver codes present in kernel directory; initialize via --with-hdadir])
fi

fi

AC_DEFUN([CHECK_FILE], [
  AC_MSG_CHECKING(for file $1)
  if test -n "$hdadir"; then
    if test -f $hdadir/$1; then
      AC_MSG_RESULT(yes)
      ln -s $hdadir/$1 kernel
      AC_DEFINE($2)
    else
      AC_MSG_RESULT(no, creating dummy file)
      touch kernel/$1
    fi
  else
    if test -s kernel/$1; then
      AC_MSG_RESULT(yes)
      AC_DEFINE($2)
    else
      AC_MSG_RESULT(no, creating dummy file)
      touch kernel/$1
    fi
  fi
])

AC_DEFUN([CHECK_CODEC], [
  AC_MSG_CHECKING(for codec $1)
  if test -n "$hdadir"; then
    if test -f $hdadir/patch_$1.c; then
      AC_MSG_RESULT(yes)
      ln -s $hdadir/patch_$1.c kernel
      AC_DEFINE($2)
    else
      AC_MSG_RESULT(no, creating dummy file)
      touch kernel/patch_$1.c
    fi
  else
    if test -s kernel/patch_$1.c; then
      AC_MSG_RESULT(yes)
      AC_DEFINE($2)
    else
      AC_MSG_RESULT(no, creating dummy file)
      touch kernel/patch_$1.c
    fi
  fi
])

CHECK_CODEC(analog, CONFIG_SND_HDA_CODEC_ANALOG)
CHECK_CODEC(atihdmi, CONFIG_SND_HDA_CODEC_ATIHDMI)
CHECK_CODEC(cmedia, CONFIG_SND_HDA_CODEC_CMEDIA)
CHECK_CODEC(conexant, CONFIG_SND_HDA_CODEC_CONEXANT)
CHECK_CODEC(intelhdmi, CONFIG_SND_HDA_CODEC_INTELHDMI)
CHECK_CODEC(nvhdmi, CONFIG_SND_HDA_CODEC_NVHDMI)
CHECK_CODEC(realtek, CONFIG_SND_HDA_CODEC_REALTEK)
CHECK_CODEC(sigmatel, CONFIG_SND_HDA_CODEC_SIGMATEL)
CHECK_CODEC(si3054, CONFIG_SND_HDA_CODEC_SI3054)
CHECK_CODEC(via, CONFIG_SND_HDA_CODEC_VIA)
CHECK_CODEC(ca0110, CONFIG_SND_HDA_CODEC_CA0110)

CHECK_FILE(hda_patch.h, HDA_PATCH_H)
CHECK_FILE(hda_beep.c, CONFIG_SND_HDA_INPUT_BEEP)
CHECK_FILE(hda_beep.h, CONFIG_SND_HDA_INPUT_BEEP)
CHECK_FILE(hda_eld.c, CONFIG_SND_HDA_ELD)

echo "Generating kernel/init_hooks.h..."
rm -f kernel/init_hooks.h
cat kernel/patch_*.c | grep '^module_init(' | \
    sed -e's/^module_init(\(.*\))/void call_init_\1(void);/g' > kernel/init_hooks.h
echo 'static void gather_codec_hooks(void) {' >> kernel/init_hooks.h
cat kernel/patch_*.c | grep '^module_init(' | \
    sed -e's/^module_init(\(.*\))/call_init_\1();/g' >> kernel/init_hooks.h
echo '}' >> kernel/init_hooks.h

AC_MSG_CHECKING(for presence of power_save option)
HAVE_POWER_SAVE=
if grep -q 'snd_hda_power_up' kernel/hda_codec.h; then
  AC_MSG_RESULT(yes)
  HAVE_POWER_SAVE=1
else
  AC_MSG_RESULT(no)
fi
if test "$HAVE_POWER_SAVE" = "1"; then
  AC_DEFINE(HAVE_POWER_SAVE)
  AC_DEFINE(CONFIG_SND_HDA_POWER_SAVE)
fi

if test "$HAVE_POWER_SAVE" = "1"; then
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
fi

AC_MSG_CHECKING(for old bus callbacks)
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Iinclude"
OLD_HDA_CMD=
AC_TRY_COMPILE([
#include "hda-types.h"
#include "hda-log.h"
#include <sound/driver.h>
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
#include <sound/driver.h>
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

AC_MSG_CHECKING(for new HDA PCM)
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Iinclude"
OLD_HDA_PCM=
AC_TRY_COMPILE([
#include "hda-types.h"
#include "hda-log.h"
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include "kernel/hda_codec.h"
],[
  struct hda_pcm pcm;
  pcm.pcm_type = 0;
],
  [AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no);OLD_HDA_PCM="1"],
  [AC_MSG_RESULT(unknown)]
)
CFLAGS=$ac_save_CFLAGS
if test "$OLD_HDA_PCM" = "1"; then
  AC_DEFINE(OLD_HDA_PCM)
fi

AC_MSG_CHECKING(for hda-reconfigure)
if grep -q snd_hda_codec_configure kernel/hda_local.h; then
  AC_MSG_RESULT(yes)
  HAVE_RECONFIG=yes
  AC_DEFINE(CONFIG_SND_HDA_RECONFIG)
else
  AC_MSG_RESULT(no)
  HAVE_RECONFIG=no
fi

if test "$HAVE_RECONFIG" = "yes"; then
   AC_MSG_CHECKING(for user pin configs)
   if grep -q user_pins kernel/hda_codec.h; then
      AC_MSG_RESULT(yes)
      AC_DEFINE(HAVE_USER_PINCFGS)
   else
      AC_MSG_RESULT(no)
   fi
fi

AC_MSG_CHECKING(for new quirk list)
NEW_QUIRK_LIST=
if grep -q SND_PCI_QUIRK_MASK kernel/patch_sigmatel.c; then
  AC_MSG_RESULT(yes)
  NEW_QUIRK_LIST=1
else
  AC_MSG_RESULT(no)
fi
if test "$NEW_QUIRK_LIST" = "1"; then
  AC_DEFINE(NEW_QUIRK_LIST)
fi

AC_MSG_CHECKING(for hda_codec.pin_amp_workaround)
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Iinclude"
HDA_CODEC_PIN_AMP_WORKAROUND=
AC_TRY_COMPILE([
#include "hda-types.h"
#include "hda-log.h"
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include "kernel/hda_codec.h"
],[
  struct hda_codec codec;
  codec.pin_amp_workaround = 1;
],
  [AC_MSG_RESULT(yes); HDA_CODEC_PIN_AMP_WORKAROUND="1"],
  [AC_MSG_RESULT(no)],
  [AC_MSG_RESULT(unknown)]
)
CFLAGS=$ac_save_CFLAGS
if test "$HDA_CODEC_PIN_AMP_WORKAROUND" = "1"; then
  AC_DEFINE(HDA_CODEC_PIN_AMP_WORKAROUND)
fi

AC_MSG_CHECKING(for hda_codec.function_id)
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Iinclude"
HDA_CODEC_FUNCTION_ID=
AC_TRY_COMPILE([
#include "hda-types.h"
#include "hda-log.h"
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include "kernel/hda_codec.h"
],[
  struct hda_codec codec;
  codec.function_id = 1;
],
  [AC_MSG_RESULT(yes); HDA_CODEC_FUNCTION_ID="1"],
  [AC_MSG_RESULT(no)],
  [AC_MSG_RESULT(unknown)]
)
CFLAGS=$ac_save_CFLAGS
if test "$HDA_CODEC_FUNCTION_ID" = "1"; then
  AC_DEFINE(HDA_CODEC_FUNCTION_ID)
fi

AC_OUTPUT(Makefile kernel/Makefile include/Makefile)

