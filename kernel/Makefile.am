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
	patch_via.c

noinst_LIBRARIES = libhda.a
libhda_a_SOURCES = hda_codec.c hda_generic.c hda_proc.c \
	 hda_beep.c hda_eld.c \
	 $(PATCHES)
noinst_HEADERS = hda_codec.h hda_local.h hda_beep.h hda_patch.h \
	init_hooks.h

init_hooks.h: $(HDA_PATCHES)
	@rm -f $@
	@cat $(HDA_PATCHES) | grep '^module_init(' | \
		sed -e's/^module_init(\(.*\))/void call_init_\1(void);/g' > $@
	@echo 'static void gather_codec_hooks(void) {' >> $@
	@cat $(HDA_PATCHES) | grep '^module_init(' | \
		sed -e's/^module_init(\(.*\))/call_init_\1();/g' >> $@
	@echo '}' >> $@

EXTRA_DIST = README

INCLUDES = -I$(top_srcdir)/include
