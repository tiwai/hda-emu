SUBDIRS = kernel include
bin_PROGRAMS = hda-emu
hda_emu_SOURCES = snd-control.c snd-vmaster.c snd-wrapper.c \
	hda-emu.c hda-parse.c hda-spec.c hda-int.c hda-ctlsh.c
hda_emu_LDADD = kernel/libhda.a

INCLUDES = -I$(top_srcdir)/include

EXTRA_DIST = README AUTHORS codecs/*

AUTOMAKE_OPTIONS = foreign
