SUBDIRS = kernel include
bin_PROGRAMS = hda-emu hda-decode-verb hda-encode-verb

hda_emu_SOURCES = snd-control.c snd-vmaster.c snd-wrapper.c \
	hda-emu.c hda-parse.c hda-spec.c hda-int.c hda-ctlsh.c hda-log.c
hda_emu_LDADD = kernel/libhda.a

hda_decode_verb_SOURCES = hda-decode-verb.c hda-log.c hda-int.c
hda_encode_verb_SOURCES =

INCLUDES = -I$(top_srcdir)/include

EXTRA_DIST = README AUTHORS codecs/*

AUTOMAKE_OPTIONS = foreign

hda-encode-verb: hda-decode-verb
	rm -f $@
	$(LN_S) $< $@

install-exec-hook: hda-encode-verb
	rm -f $(DESTDIR)$(bindir)/$<
	(cd $(DESTDIR)$(bindir) && $(LN_S) hda-decode-verb $<)
