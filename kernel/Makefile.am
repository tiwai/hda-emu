PATCHES =
if CONFIG_SND_HDA_CODEC_ANALOG
PATCHES += patch_analog.c
endif
if CONFIG_SND_HDA_CODEC_CMEDIA
PATCHES += patch_cmedia.c
endif
if CONFIG_SND_HDA_CODEC_CONEXANT
PATCHES += patch_conexant.c
endif
if CONFIG_SND_HDA_CODEC_REALTEK
PATCHES += patch_realtek.c
endif
if CONFIG_SND_HDA_CODEC_SI3054
PATCHES += patch_si3054.c
endif
if CONFIG_SND_HDA_CODEC_SIGMATEL
PATCHES += patch_sigmatel.c
endif
if CONFIG_SND_HDA_CODEC_VIA
PATCHES += patch_via.c
endif
if CONFIG_SND_HDA_CODEC_CA0110
PATCHES += patch_ca0110.c
endif
if CONFIG_SND_HDA_CODEC_CA0132
PATCHES += patch_ca0132.c
endif
if CONFIG_SND_HDA_CODEC_CIRRUS
PATCHES += patch_cirrus.c
endif
if CONFIG_SND_HDA_CODEC_ATIHDMI
PATCHES += patch_atihdmi.c
endif
if CONFIG_SND_HDA_CODEC_INTELHDMI
PATCHES += patch_intelhdmi.c
endif
if CONFIG_SND_HDA_CODEC_NVHDMI
PATCHES += patch_nvhdmi.c
endif

if USE_OWN_PROC
HELPERS = hda_own_proc.c
else
HELPERS = hda_proc.c
endif

# HELPERS += hda_beep.c 

if HAVE_HDA_AUTO_PARSER
HELPERS += hda_auto_parser.c
endif

if HAVE_HDA_JACK
HELPERS += hda_jack.c
endif

if HAVE_HDA_ELD
HELPERS += hda_eld.c
endif

noinst_LIBRARIES = libhda.a
libhda_a_SOURCES = hda_codec.c hda_generic.c misc_wrapper.c \
	 $(PATCHES) $(HELPERS)
noinst_HEADERS = init_hooks.h

INCLUDES = -I$(top_srcdir)/include

misc_wrapper.c: ../misc_wrapper.c
	ln -s ../misc_wrapper.c
