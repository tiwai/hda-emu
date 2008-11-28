PATCHES = patch_realtek.c patch_analog.c patch_conexant.c patch_cmedia.c \
	patch_atihdmi.c patch_via.c patch_si3054.c patch_sigmatel.c \
	patch_nvhdmi.c patch_intelhdmi.c

noinst_LIBRARIES = libhda.a
libhda_a_SOURCES = hda_codec.c hda_generic.c hda_proc.c \
	hda_beep.c hda_eld.c $(PATCHES)
noinst_HEADERS = hda_beep.h hda_codec.h hda_local.h hda_patch.h \
	init_hooks.h

EXTRA_DIST = README

INCLUDES = -I$(top_srcdir)/include

init_hooks.h: $(PATCHES)
	@rm -f $@
	@cat $(PATCHES) | grep '^module_init(' | \
		sed -e's/^module_init(\(.*\))/void call_init_\1(void);/g' > $@
	@echo 'static void gather_codec_hooks(void) {' >> $@
	@cat $(PATCHES) | grep '^module_init(' | \
		sed -e's/^module_init(\(.*\))/call_init_\1();/g' >> $@
	@echo '}' >> $@
