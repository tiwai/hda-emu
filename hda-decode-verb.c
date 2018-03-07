/*
 * hda-emu - simple HD-audio codec emulator for debugging snd-hda-intel driver
 *
 * Decode HD-audio verb/parameter values
 *
 * Copyright (c) Takashi Iwai <tiwai@suse.de>
 *
 *  This driver is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This driver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include "hda-types.h"
#include "hda-log.h"

enum { DECODE, ENCODE };

static void usage(int mode)
{
	if (mode == DECODE) {
		fprintf(stderr,
			"usage: \n"
			"  hda-decode-verb VALUE\n"
			"    Decode the encoded command value\n"
			"  hda-decode-verb VERB PARAMETER\n"
			"    Decode the verb and parameter values\n"
			);
	} else {
		fprintf(stderr,
			"usage: \n"
			"  hda-encode-verb VERB PARAMETER\n"
			"    Encode the verb and parameter values\n"
			);
	}
	exit(1);
}

unsigned int hda_decode_verb(struct xhda_codec *codec, unsigned int val)
{
	unsigned int cid, /*dir,*/ nid, verb, parm;

	hda_log(HDA_LOG_INFO, "raw value = 0x%08x\n", val);

	cid = (val >> 28) & 0xf;
	/* dir = (val >> 27) & 1; */
	nid = (val >> 20) & 0x7f;
	verb = (val >> 8) & 0xfff;
	parm = val & 0xff;
	hda_log(HDA_LOG_INFO,
		"cid = %d, nid = 0x%02x, verb = 0x%x, parm = 0x%02x\n",
		cid, nid, verb, parm);

	return hda_decode_verb_parm(codec, verb, parm);
}


static void encode(const char *vstr, const char *pstr)
{
	unsigned int verb, parm;

	if (hda_encode_verb_parm(vstr, pstr, &verb, &parm) < 0) {
		hda_log(HDA_LOG_ERR, "Invalid parameters\n");
		exit(1);
	}
	hda_log(HDA_LOG_INFO, "verb = 0x%x, parm = 0x%x\n", verb, parm);
}

int main(int argc, char **argv)
{
	int mode = DECODE;
	int c;

	hda_log_init(NULL, 0);
	hda_log_level_set(HDA_LOG_INFO);

	if (strstr(argv[0], "hda-encode-verb"))
		mode = ENCODE;

	while ((c = getopt(argc, argv, "de")) != -1) {
		switch (c) {
		case 'd':
			mode = DECODE;
			break;
		case 'e':
			mode = ENCODE;
			break;
		default:
			usage(mode);
		}
	}

	switch (mode) {
	case DECODE:
		if (optind >= argc)
			usage(mode);
		if (optind + 1 == argc) {
			hda_decode_verb(NULL, strtoul(argv[optind], NULL, 0));
		} else {
			hda_decode_verb_parm(NULL,
					     strtoul(argv[optind], NULL, 0),
					     strtoul(argv[optind + 1], NULL, 0));
		}
		break;
	case ENCODE:
		if (optind + 2 > argc)
			usage(mode);
		encode(argv[optind], argv[optind + 1]);
		break;
	}
	return 0;
}
