PATCHES = \
	patch_analog.c \
	patch_cmedia.c \
	patch_conexant.c \
	patch_realtek.c \
	patch_si3054.c \
	patch_sigmatel.c \
	patch_via.c \
	patch_ca0110.c \
	patch_ca0132.c \
	patch_cirrus.c

if UNIFIED_HDMI
PATCHES += patch_hdmi.c
else
PATCHES += patch_atihdmi.c \
	patch_intelhdmi.c \
	patch_nvhdmi.c
endif

noinst_LIBRARIES = libhda.a
libhda_a_SOURCES = hda_codec.c hda_generic.c hda_proc.c \
	 hda_beep.c hda_eld.c hda_jack.c \
	 misc_wrapper.c \
	 $(PATCHES)
noinst_HEADERS = hda_codec.h hda_local.h hda_beep.h hda_patch.h \
	init_hooks.h

INCLUDES = -I$(top_srcdir)/include

jackctl.c: ../jackctl.c
	ln -s ../jackctl.c

misc_wrapper.c: ../misc_wrapper.c
	ln -s ../misc_wrapper.c
