AC_PREREQ(2.59)
AC_INIT(hda-emu.c)
AM_INIT_AUTOMAKE(hda-emu, 0.2.6)

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_DISABLE_SHARED
AM_PROG_LIBTOOL
AC_HEADER_STDC

WFLAGS="-Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security"
WFLAGS="$WFLAGS -fno-delete-null-pointer-checks"
WFLAGS="$WFLAGS -Wdeclaration-after-statement"
WFLAGS="$WFLAGS -Wno-pointer-sign"
WFLAGS="$WFLAGS -fwrapv"
WFLAGS="$WFLAGS -fno-asynchronous-unwind-tables"
OPTFLAGS="-O -g"
CFLAGS="$CFLAGS $OPTFLAGS $WFLAGS"

AC_ARG_ENABLE([readline],
  AS_HELP_STRING([--enable-readline], [turn on readline support]),
  readline="$enableval", readline="yes")
if test "$readline" = "yes"; then
  AC_CHECK_LIB([readline], [main], [HAVE_LIBREADLINE="yes"])
  if test "$HAVE_LIBREADLINE" = "yes"; then
    LIBREADLINE="-lreadline -lncurses"
    AC_DEFINE([HAVE_LIBREADLINE], [1], [Define if you have libreadline])
  fi
fi
AC_SUBST(LIBREADLINE)

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

hdadir="$( cd "$hdadir" && pwd )"

if test -n "$hdadir"; then

test -d "$hdadir" || \
  AC_ERROR([Invalid HD-audio directory: $hdadir])
test -f "$hdadir/hda_codec.c" || \
  AC_ERROR([Invalid HD-audio directory: $hdadir])

rm -f hda
ln -s $hdadir hda

else
dnl check existing kernel codes

if ! test -f hda/hda_codec.c; then
  AC_ERROR([No HD-audio driver codes present in kernel directory; initialize via --with-hdadir])
fi

fi

AC_DEFUN([CHECK_CODEC], [
  AC_MSG_CHECKING(for codec $1)
  if test -f hda/patch_$1.c; then
    AC_MSG_RESULT(yes)
    AC_DEFINE($2)
    codec_found=yes
  else
    AC_MSG_RESULT(no)
    codec_found=
  fi
  AM_CONDITIONAL($2, test "$codec_found" = "yes")
])

CHECK_CODEC(analog, CONFIG_SND_HDA_CODEC_ANALOG)
CHECK_CODEC(atihdmi, CONFIG_SND_HDA_CODEC_ATIHDMI)
CHECK_CODEC(cmedia, CONFIG_SND_HDA_CODEC_CMEDIA)
CHECK_CODEC(conexant, CONFIG_SND_HDA_CODEC_CONEXANT)
CHECK_CODEC(intelhdmi, CONFIG_SND_HDA_CODEC_INTELHDMI)
CHECK_CODEC(nvhdmi, CONFIG_SND_HDA_CODEC_NVHDMI)
CHECK_CODEC(hdmi, CONFIG_SND_HDA_CODEC_HDMI)
CHECK_CODEC(realtek, CONFIG_SND_HDA_CODEC_REALTEK)
CHECK_CODEC(sigmatel, CONFIG_SND_HDA_CODEC_SIGMATEL)
CHECK_CODEC(si3054, CONFIG_SND_HDA_CODEC_SI3054)
CHECK_CODEC(via, CONFIG_SND_HDA_CODEC_VIA)
CHECK_CODEC(ca0110, CONFIG_SND_HDA_CODEC_CA0110)
CHECK_CODEC(ca0132, CONFIG_SND_HDA_CODEC_CA0132)
CHECK_CODEC(cirrus, CONFIG_SND_HDA_CODEC_CIRRUS)

AC_DEFUN([CHECK_HELPER], [
  AC_MSG_CHECKING(for helper $1)
  if test -f hda/$1; then
    AC_MSG_RESULT(yes)
    AC_DEFINE($2)
    codec_found=yes
  else
    AC_MSG_RESULT(no)
    codec_found=
  fi
  AM_CONDITIONAL($2, test "$codec_found" = "yes")
])

CHECK_HELPER([hda_auto_parser.c], [HAVE_HDA_AUTO_PARSER])
CHECK_HELPER([hda_jack.c], [HAVE_HDA_JACK])
CHECK_HELPER([hda_eld.c], [HAVE_HDA_ELD])
CHECK_HELPER([hda_beep.c], [HAVE_HDA_BEEP])
CHECK_HELPER([hda_bind.c], [HAVE_HDA_BIND])

AC_ARG_ENABLE(own-proc,
  AS_HELP_STRING([--enable-own-proc],
    [use own proc-file output function]),
  own_proc="$enableval", own_proc="no")
AM_CONDITIONAL(USE_OWN_PROC, test "$own_proc" = "yes")

echo "Generating kernel/init_hooks.h..."
rm -f kernel/init_hooks.h
if test -f hda/hda_bind.c; then
cat hda/hda_bind.c | grep '^module_init(' | \
    sed -e's/^module_init(\(.*\))/void call_init_\1(void);/g' >> kernel/init_hooks.h
fi
cat hda/patch_*.c | grep '^module_init(' | \
    sed -e's/^module_init(\(.*\))/void call_init_\1(void);/g' >> kernel/init_hooks.h
cat hda/patch_*.c | grep '^module_hda_codec_driver(' | \
    sed -e's/^module_hda_codec_driver(\(.*\))/void call_init_\1_init(void);/g' >> kernel/init_hooks.h
echo 'static void gather_codec_hooks(void) {' >> kernel/init_hooks.h
if test -f hda/hda_bind.c; then
cat hda/hda_bind.c | grep '^module_init(' | \
    sed -e's/^module_init(\(.*\))/call_init_\1();/g' >> kernel/init_hooks.h
fi
cat hda/patch_*.c | grep '^module_init(' | \
    sed -e's/^module_init(\(.*\))/call_init_\1();/g' >> kernel/init_hooks.h
cat hda/patch_*.c | grep '^module_hda_codec_driver(' | \
    sed -e's/^module_hda_codec_driver(\(.*\))/call_init_\1_init();/g' >> kernel/init_hooks.h
echo '}' >> kernel/init_hooks.h

AC_MSG_CHECKING(for presence of power_save option)
HAVE_POWER_SAVE=
if grep -q 'snd_hda_power_up' hda/hda_codec.h; then
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
if grep -q '^static int power_save' hda/hda_codec.c; then
  AC_MSG_RESULT(yes)
  OLD_POWER_SAVE=1
else
  AC_MSG_RESULT(no)
fi
if test "$OLD_POWER_SAVE" = "1"; then
  AC_DEFINE(OLD_POWER_SAVE)
fi
fi

NEW_HDA_INFRA=
AC_MSG_CHECKING(for new hda structure)
if grep -q hda_bus_template hda/hda_codec.h; then
  AC_MSG_RESULT(no)
else
  AC_MSG_RESULT(yes)
  NEW_HDA_INFRA=yes
fi
if test "$NEW_HDA_INFRA" = "yes"; then
  AC_DEFINE(NEW_HDA_INFRA)
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
#include "hda/hda_codec.h"
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

AC_MSG_CHECKING(for new pm_notify callback)
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Iinclude"
HAVE_NEW_PM_NOTIFY=
AC_TRY_COMPILE([
#include "hda-types.h"
#include "hda-log.h"
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include "hda/hda_codec.h"
],[
  extern struct hda_bus_ops *ops;
  ops->pm_notify(NULL, 0);
],
  [AC_MSG_RESULT(yes); HAVE_NEW_PM_NOTIFY="1"],
  [AC_MSG_RESULT(no)],
  [AC_MSG_RESULT(unknown)]
)
CFLAGS=$ac_save_CFLAGS
if test "$HAVE_NEW_PM_NOTIFY" = "1"; then
  AC_DEFINE(HAVE_NEW_PM_NOTIFY)
fi

AC_MSG_CHECKING(for attach_pcm bus ops)
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Iinclude"
HAVE_HDA_ATTACH_PCM=
AC_TRY_COMPILE([
#include "hda-types.h"
#include "hda-log.h"
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include "hda/hda_codec.h"
],[
  extern struct hda_bus_ops *ops;
  ops->attach_pcm(NULL, NULL, NULL);
],
  [AC_MSG_RESULT(yes); HAVE_HDA_ATTACH_PCM="1"],
  [AC_MSG_RESULT(no)],
  [AC_MSG_RESULT(unknown)]
)
CFLAGS=$ac_save_CFLAGS
if test "$HAVE_HDA_ATTACH_PCM" = "1"; then
  AC_DEFINE(HAVE_HDA_ATTACH_PCM)
fi

if test "$HAVE_HDA_ATTACH_PCM" = "1"; then
AC_MSG_CHECKING(for get_response with caddr)
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Iinclude"
HAVE_GET_RESPONSE_WITH_CADDR=
AC_TRY_COMPILE([
#include "hda-types.h"
#include "hda-log.h"
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include "hda/hda_codec.h"
],[
  extern struct hda_bus_ops *ops;
  ops->get_response(NULL, 1);
],
  [AC_MSG_RESULT(yes); HAVE_GET_RESPONSE_WITH_CADDR="1"],
  [AC_MSG_RESULT(no)],
  [AC_MSG_RESULT(unknown)]
)
CFLAGS=$ac_save_CFLAGS
if test "$HAVE_GET_RESPONSE_WITH_CADDR" = "1"; then
  AC_DEFINE(HAVE_GET_RESPONSE_WITH_CADDR)
fi
fi # HAVE_HDA_ATTACH_PCM=1

if test "$NEW_HDA_INFRA" = "yes"; then

AC_DEFINE(CONFIG_SND_HDA_RECONFIG)
AC_DEFINE(HAVE_HDA_PATCH_LOADER)

else

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
#include "hda/hda_codec.h"
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

HAVE_HDA_PATCH_LOADER=
if test "$OLD_HDA_CODEC_NEW" = "1"; then
AC_MSG_CHECKING(for hda-patch)
if grep -q snd_hda_load_patch hda/hda_codec.h; then
  AC_MSG_RESULT(yes)
  HAVE_HDA_PATCH_LOADER=yes
  AC_DEFINE(CONFIG_SND_HDA_RECONFIG)
else
  AC_MSG_RESULT(no)
fi
fi
if test "$HAVE_HDA_PATCH_LOADER" = "yes"; then
  AC_DEFINE(HAVE_HDA_PATCH_LOADER)
fi

fi # NEW_HDA_INFRA

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
#include "hda/hda_codec.h"
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
if grep -q snd_hda_codec_configure hda/hda_*.h; then
  AC_MSG_RESULT(yes)
  HAVE_RECONFIG=yes
  AC_DEFINE(CONFIG_SND_HDA_RECONFIG)
else
  AC_MSG_RESULT(no)
  HAVE_RECONFIG=no
fi

if test "$HAVE_RECONFIG" = "yes"; then
   AC_MSG_CHECKING(for user pin configs)
   if grep -q user_pins hda/hda_codec.h; then
      AC_MSG_RESULT(yes)
      AC_DEFINE(HAVE_USER_PINCFGS)
   else
      AC_MSG_RESULT(no)
   fi
fi

AC_MSG_CHECKING(for new quirk list)
NEW_QUIRK_LIST=
if grep -q SND_PCI_QUIRK_MASK hda/patch_sigmatel.c; then
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
#include "hda/hda_codec.h"
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
#include "hda/hda_codec.h"
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

AC_MSG_CHECKING(for snd_ctl_boolean_mono_info definition)
HAVE_BOOLEAN_INFO=
if grep -q snd_ctl_boolean_mono_info hda/hda_local.h; then
  AC_MSG_RESULT(yes)
  HAVE_BOOLEAN_INFO=1
else
  AC_MSG_RESULT(no)
fi
if test "$HAVE_BOOLEAN_INFO" = "1"; then
  AC_DEFINE(HAVE_BOOLEAN_INFO)
fi

if test "$NEW_HDA_INFRA" != "yes"; then
AC_MSG_CHECKING(for snd_hda_suspend with pmsg)
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Iinclude"
HAVE_HDA_SUSPEND_PMSG=
AC_TRY_COMPILE([
#include "hda-types.h"
#include "hda-log.h"
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include "hda/hda_codec.h"
],[
  snd_hda_suspend(NULL);
],
  [AC_MSG_RESULT(no)],
  [AC_MSG_RESULT(yes); HAVE_HDA_SUSPEND_PMSG="1"],
  [AC_MSG_RESULT(unknown)]
)
CFLAGS=$ac_save_CFLAGS
if test "$HAVE_HDA_SUSPEND_PMSG" = "1"; then
  AC_DEFINE(HAVE_HDA_SUSPEND_PMSG)
fi
fi # !NEW_HDA_INFRA

HAVE_SND_HDA_JACK=
AC_MSG_CHECKING(for snd_hda_jack_xxx)
if grep -q snd_hda_jack_free hda/hda_codec.h; then
  AC_MSG_RESULT(yes)
  HAVE_SND_HDA_JACK=yes
else
  AC_MSG_RESULT(no)
fi
if test "$HAVE_SND_HDA_JACK" = "yes"; then
  AC_DEFINE(HAVE_SND_HDA_JACK)
fi

AC_MSG_CHECKING(for invidual spdif ctls)
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Iinclude"
INDIVIDUALSPDIF_CTLS=
AC_TRY_COMPILE([
#include "hda-types.h"
#include "hda-log.h"
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include "hda/hda_codec.h"
],[
  struct hda_spdif_out *spdif = snd_hda_spdif_out_of_nid(0, 0);
],
  [AC_MSG_RESULT(yes); INDIVIDUAL_SPDIF_CTLS="1"],
  [AC_MSG_RESULT(no)],
  [AC_MSG_RESULT(unknown)]
)
CFLAGS=$ac_save_CFLAGS
if test "$INDIVIDUAL_SPDIF_CTLS" = "1"; then
  AC_DEFINE(INDIVIDUAL_SPDIF_CTLS)
fi

if test "$INDIVIDUAL_SPDIF_CTLS" != "1"; then
AC_MSG_CHECKING(for new snd_hda_calc_stream_format)
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Iinclude"
STREAM_FORMAT_WITH_SPDIF=
AC_TRY_COMPILE([
#include "hda-types.h"
#include "hda-log.h"
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include "hda/hda_codec.h"
],[
  snd_hda_calc_stream_format(0, 0, 0, 0, 0);
],
  [AC_MSG_RESULT(yes); STREAM_FORMAT_WITH_SPDIF="1"],
  [AC_MSG_RESULT(no)],
  [AC_MSG_RESULT(unknown)]
)
CFLAGS=$ac_save_CFLAGS
if test "$STREAM_FORMAT_WITH_SPDIF" = "1"; then
  AC_DEFINE(STREAM_FORMAT_WITH_SPDIF)
fi
fi

AC_MSG_CHECKING(for snd_hda_calc_stream_format with codec)
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Iinclude"
STREAM_FORMAT_WITH_CODEC=
AC_TRY_COMPILE([
#include "hda-types.h"
#include "hda-log.h"
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include "hda/hda_codec.h"
],[
  snd_hda_calc_stream_format(0, 0, 0, 0, 0, 0);
],
  [AC_MSG_RESULT(yes); STREAM_FORMAT_WITH_CODEC="1"],
  [AC_MSG_RESULT(no)],
  [AC_MSG_RESULT(unknown)]
)
CFLAGS=$ac_save_CFLAGS
if test "$STREAM_FORMAT_WITH_CODEC" = "1"; then
  AC_DEFINE(STREAM_FORMAT_WITH_CODEC)
fi

HAVE_COMMON_PREPARE=
AC_MSG_CHECKING(for common snd_hda_codec_prepare/cleanup)
if grep -q snd_hda_codec_prepare hda/hda_codec.h; then
  AC_MSG_RESULT(yes)
  HAVE_COMMON_PREPARE=yes
else
  AC_MSG_RESULT(no)
fi
if test "$HAVE_COMMON_PREPARE" = "yes"; then
  AC_DEFINE(HAVE_COMMON_PREPARE)
fi

AC_MSG_CHECKING(for snd_hda_codec_get_pincfg)
HAVE_SND_HDA_CODEC_GET_PINCFG=
if grep -q 'snd_hda_codec_get_pincfg' hda/hda_codec.h; then
  AC_MSG_RESULT(yes)
  HAVE_SND_HDA_CODEC_GET_PINCFG=1
else
  AC_MSG_RESULT(no)
fi
if test "$HAVE_SND_HDA_CODEC_GET_PINCFG" = "1"; then
  AC_DEFINE(HAVE_SND_HDA_CODEC_GET_PINCFG)
fi

AC_MSG_CHECKING(for codec user_mutex)
HAVE_CODEC_USER_MUTEX=
if grep -q 'user_mutex' hda/hda_codec.h; then
  AC_MSG_RESULT(yes)
  HAVE_CODEC_USER_MUTEX=1
else
  AC_MSG_RESULT(no)
fi
if test "$HAVE_CODEC_USER_MUTEX" = "1"; then
  AC_DEFINE(HAVE_CODEC_USER_MUTEX)
fi

AC_OUTPUT(Makefile kernel/Makefile include/Makefile)

