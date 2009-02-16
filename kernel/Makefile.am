PATCHES = \
	patch_analog.c \
	patch_atihdmi.c \
	patch_cmedia.c \
	patch_conexant.c \
	patch_intelhdmi.c \
	patch_nvhdmi.c \
	patch_realtek.c \
	patch_si3054.c \
	patch_sigmatel.c \
	patch_via.c \
	patch_ca0110.c

noinst_LIBRARIES = libhda.a
libhda_a_SOURCES = hda_codec.c hda_generic.c hda_proc.c \
	 hda_beep.c hda_eld.c \
	 $(PATCHES)
noinst_HEADERS = hda_codec.h hda_local.h hda_beep.h hda_patch.h \
	init_hooks.h

INCLUDES = -I$(top_srcdir)/include
