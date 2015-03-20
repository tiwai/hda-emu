SUBDIRS = kernel include
if HAVE_HDA_CORE
SUBDIRS += lib
endif

bin_PROGRAMS = hda-emu \
	hda-decode-verb hda-encode-verb \
	hda-decode-pincfg hda-encode-pincfg

hda_emu_SOURCES = snd-control.c snd-vmaster.c snd-wrapper.c \
	hda-emu.c hda-parse.c hda-spec.c hda-int.c hda-ctlsh.c hda-log.c \
	ctljack.c
hda_emu_LDADD = kernel/libhda.a
if HAVE_HDA_CORE
hda_emu_LDADD += lib/libhdacore.a
endif
hda_emu_LDADD += @LIBREADLINE@

hda_decode_verb_SOURCES = hda-decode-verb.c hda-log.c hda-int.c
hda_encode_verb_SOURCES =

hda_decode_pincfg_SOURCES = hda-decode-pincfg.c hda-log.c
hda_encode_pincfg_SOURCES =

INCLUDES = -I$(top_srcdir)/include

EXTRA_DIST = README AUTHORS codecs/*

AUTOMAKE_OPTIONS = foreign

hda-encode-verb: hda-decode-verb
	rm -f $@
	$(LN_S) $< $@

hda-encode-pincfg: hda-decode-pincfg
	rm -f $@
	$(LN_S) $< $@

install-exec-hook:
	rm -f $(DESTDIR)$(bindir)/hda-encode-verb
	(cd $(DESTDIR)$(bindir) && $(LN_S) hda-decode-verb hda-encode-verb)
	rm -f $(DESTDIR)$(bindir)/hda-encode-pincfg
	(cd $(DESTDIR)$(bindir) && $(LN_S) hda-decode-pincfg hda-encode-pincfg)
