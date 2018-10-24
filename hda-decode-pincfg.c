/*
 * hda-emu - simple HD-audio codec emulator for debugging snd-hda-intel driver
 *
 * Decode HD-audio pin config values
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

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include "hda-types.h"
#include "hda-log.h"
#include <sound/core.h>
#include <sound/hda_codec.h>

enum { DECODE, ENCODE };

static void usage(int mode)
{
	if (mode == DECODE) {
		fprintf(stderr,
			"usage: \n"
			"  hda-decode-pincfg VALUE\n"
			"    Decode the encoded command value\n"
			);
	} else {
		fprintf(stderr,
			"usage: \n"
			"  hda-encode-pincfg [options]\n"
			"    Encode the pin-configuration value\n"
			"    -j str   Specifies port connectivity\n"
			"    -l str   Specifies port location\n"
			"    -t str   Specifies port default device\n"
			"    -c str   Specifies port connection type\n"
			"    -C str   Specifies port color\n"
			"    -a val   Specifies port default association\n"
			"    -s val   Specifies port sequence\n"
			"    -m val   Specifies port misc value\n"
			"    -b val   Specifies pincfg value to base on\n"
			);
	}
	exit(1);
}

static const char *jack_connections[16] = {
	"Unknown", "1/8", "1/4", "ATAPI",
	"RCA", "Optical","Digital", "Analog",
	"DIN", "XLR", "RJ11", "Comb",
	NULL, NULL, NULL, "Other"
};

static const char *get_jack_connection(u32 cfg)
{
	cfg = (cfg & AC_DEFCFG_CONN_TYPE) >> AC_DEFCFG_CONN_TYPE_SHIFT;
	if (jack_connections[cfg])
		return jack_connections[cfg];
	else
		return "UNKNOWN";
}

static const char *jack_colors[16] = {
	"Unknown", "Black", "Grey", "Blue",
	"Green", "Red", "Orange", "Yellow",
	"Purple", "Pink", NULL, NULL,
	NULL, NULL, "White", "Other",
};

static const char *get_jack_color(u32 cfg)
{
	cfg = (cfg & AC_DEFCFG_COLOR) >> AC_DEFCFG_COLOR_SHIFT;
	if (jack_colors[cfg])
		return jack_colors[cfg];
	else
		return "UNKNOWN";
}

static const char *jack_loc_bases[7] = {
	"N/A", "Rear", "Front", "Left", "Right", "Top", "Bottom",
};
static unsigned char jack_loc_specials_idx[] = {
	0x07, 0x08,
	0x17, 0x18, 0x19,
	0x37, 0x38
};
static const char *jack_loc_specials[] = {
	"Rear Panel", "Drive Bar",
	"Riser", "HDMI", "ATAPI",
	"Mobile-In", "Mobile-Out"
};

static const char *get_jack_location(u32 cfg)
{
	int i;
	cfg = (cfg & AC_DEFCFG_LOCATION) >> AC_DEFCFG_LOCATION_SHIFT;
	for (i = 0; i < ARRAY_SIZE(jack_loc_specials_idx); i++) {
		if (cfg == jack_loc_specials_idx[i])
			return jack_loc_specials[i];
	}
	if ((cfg & 0x0f) < 7)
		return jack_loc_bases[cfg & 0x0f];
	return "UNKNOWN";
}

static const char *jack_locations[4] = { "Ext", "Int", "Sep", "Oth" };

static const char *get_jack_connectivity(u32 cfg)
{
	return jack_locations[(cfg >> (AC_DEFCFG_LOCATION_SHIFT + 4)) & 3];
}

static const char *jack_types[16] = {
	"Line Out", "Speaker", "HP Out", "CD",
	"SPDIF Out", "Digital Out", "Modem Line", "Modem Hand",
	"Line In", "Aux", "Mic", "Telephony",
	"SPDIF In", "Digitial In", "Reserved", "Other"
};

static const char *get_jack_type(u32 cfg)
{
	return jack_types[(cfg & AC_DEFCFG_DEVICE)
				>> AC_DEFCFG_DEVICE_SHIFT];
}

static const char *jack_conns[4] = { "Jack", "N/A", "Fixed", "Both" };

static void decode(unsigned int caps)
{
	hda_log(HDA_LOG_INFO, "Pin Default 0x%08x: [%s] %s at %s %s\n",
		caps,
		jack_conns[(caps & AC_DEFCFG_PORT_CONN) >> AC_DEFCFG_PORT_CONN_SHIFT],
		get_jack_type(caps),
		get_jack_connectivity(caps),
		get_jack_location(caps));
	hda_log(HDA_LOG_INFO, "  Conn = %s, Color = %s\n",
		get_jack_connection(caps),
		get_jack_color(caps));
	hda_log(HDA_LOG_INFO, "  DefAssociation  = 0x%x, Sequence = 0x%x, Misc = 0x%x\n",
		(caps & AC_DEFCFG_DEF_ASSOC) >> AC_DEFCFG_ASSOC_SHIFT,
		caps & AC_DEFCFG_SEQUENCE,
		(caps & AC_DEFCFG_MISC) >> AC_DEFCFG_MISC_SHIFT);
}

/*
 */

struct str_alias {
	const char *str;
	int val;
};

static void list_avail_parm(const char **list, int nlist,
			    const struct str_alias *alias)
{
	int i;
	for (i = 0 ; i < nlist; i++)
		fprintf(stderr, "  %s\n", list[i]);
	if (!alias)
		return;
	for (; alias->str; alias++)
		fprintf(stderr, "  %s\n", alias->str);
}

static int get_parm_idx(const char **list, int nlist,
			const struct str_alias *alias,
			const char *str, const char *type)
{
	int i;
	for (i = 0; i < nlist; i++) {
		if (!list[i])
			continue;
		if (!strcasecmp(str, list[i]))
			return i;
		if (!strcmp(list[i], "N/A")) {
			if (!strcasecmp(str, "OFF") || !strcasecmp(str, "NO"))
				return i;
		}
	}
	if (alias) {
		for (; alias->str; alias++) {
			if (!strcasecmp(str, alias->str))
				return alias->val;
		}
	}
	if (type) {
		fprintf(stderr, "Invalid jack %s '%s'\n", type, str);
		fprintf(stderr, "Available values are:\n");
		list_avail_parm(list, nlist, alias);
		exit(1);
	}
	return -1;
}

static unsigned int parse_jack_port(const char *str)
{
	return get_parm_idx(jack_conns, ARRAY_SIZE(jack_conns),
			    NULL, str, "port");
}

static struct str_alias jack_type_aliases[] = {
	{ "HP", 2 },
	{ }
};

static unsigned int parse_jack_type(const char *str)
{
	return get_parm_idx(jack_types, ARRAY_SIZE(jack_types),
			    jack_type_aliases, str, "type");
}

static unsigned int parse_jack_location(const char *str)
{
	static struct str_alias loc_pfx[] = {
		{ "External ", 0 },
		{ "Internal ", 1 },
		{ "Separate ", 2 },
		{ "Other", 3 },
		{ "Ext ", 0 },
		{ "Int ", 1 },
		{ "Sep ", 2 },
		{ "Oth ", 3 },
		{ }
	};
	const struct str_alias *a;
	int val, pfx;
	int i;

	val = get_parm_idx(jack_loc_specials, ARRAY_SIZE(jack_loc_specials),
			   NULL, str, NULL);
	if (val >= 0)
		return jack_loc_specials_idx[val];

	pfx = 0;
	for (a = loc_pfx; a->str; a++) {
		if (strcasestr(str, a->str)) {
			pfx = a->val;
			break;
		}
	}

	val = -1;
	for (i = 0; i < 7; i++) {
		if (strcasestr(str, jack_loc_bases[i])) {
			val = i;
			break;
		}
	}
	if (val < 0) {
		fprintf(stderr, "Invalid jack location '%s'\n", str);
		fprintf(stderr, "Available prefix values are:\n");
		list_avail_parm(NULL, 0, loc_pfx);
		fprintf(stderr, "Available values are:\n");
		list_avail_parm(jack_loc_bases, ARRAY_SIZE(jack_loc_bases), NULL);
		list_avail_parm(jack_loc_specials, ARRAY_SIZE(jack_loc_specials), NULL);
		exit(1);
	}
	val |= (pfx << 4);
	return val;
}

static unsigned int parse_jack_connection(const char *str)
{
	return get_parm_idx(jack_connections, ARRAY_SIZE(jack_connections),
			    NULL, str, "connection");
}

static unsigned int parse_jack_color(const char *str)
{
	return get_parm_idx(jack_colors, ARRAY_SIZE(jack_colors),
			    NULL, str, "color");
}

static unsigned int encode(const char *jack,
			   const char *type,
			   const char *location,
			   const char *connection,
			   const char *color,
			   int assoc, int seq, int misc, int base)
{
	unsigned int caps = base;

	if (jack) {
		caps &= ~AC_DEFCFG_PORT_CONN;
		caps |= parse_jack_port(jack) << AC_DEFCFG_PORT_CONN_SHIFT;
	}
	if (type) {
		caps &= ~AC_DEFCFG_DEVICE;
		caps |= parse_jack_type(type) << AC_DEFCFG_DEVICE_SHIFT;
	}
	if (location) {
		caps &= ~AC_DEFCFG_LOCATION;
		caps |= parse_jack_location(location) << AC_DEFCFG_LOCATION_SHIFT;
	}
	if (connection) {
		caps &= ~AC_DEFCFG_CONN_TYPE;
		caps |= parse_jack_connection(connection) << AC_DEFCFG_CONN_TYPE_SHIFT;
	}
	if (color) {
		caps &= ~AC_DEFCFG_COLOR;
		caps |= parse_jack_color(color) << AC_DEFCFG_COLOR_SHIFT;
	}
	if (misc >= 0) {
		caps &= ~AC_DEFCFG_MISC;
		caps |= (misc << AC_DEFCFG_MISC_SHIFT) & AC_DEFCFG_MISC;
	}
	if (assoc >= 0) {
		caps &= ~AC_DEFCFG_DEF_ASSOC;
		caps |= (assoc << AC_DEFCFG_ASSOC_SHIFT) & AC_DEFCFG_DEF_ASSOC;
	}
	if (seq >= 0) {
		caps &= ~AC_DEFCFG_SEQUENCE;
		caps |= seq & AC_DEFCFG_SEQUENCE;
	}
	return caps;
}	

int main(int argc, char **argv)
{
	int mode = DECODE;
	int c;
	const char *jack = NULL;
	const char *type = NULL;
	const char *location = NULL;
	const char *connection = NULL;
	const char *color = NULL;
	int assoc = -1, seq = -1, misc = -1, base = 0;
	unsigned int val;

	hda_log_init(NULL, 0);
	hda_log_level_set(HDA_LOG_INFO);

	if (strstr(argv[0], "hda-encode-pincfg"))
		mode = ENCODE;

	while ((c = getopt(argc, argv, "dej:t:l:c:C:a:s:m:b:h")) != -1) {
		switch (c) {
		case 'd':
			mode = DECODE;
			break;
		case 'e':
			mode = ENCODE;
			break;
		case 'j':
			jack = optarg;
			break;
		case 't':
			type = optarg;
			break;
		case 'l':
			location = optarg;
			break;
		case 'c':
			connection = optarg;
			break;
		case 'C':
			color = optarg;
			break;
		case 'a':
			assoc = strtoul(optarg, NULL, 0);
			break;
		case 's':
			seq = strtoul(optarg, NULL, 0);
			break;
		case 'm':
			misc = strtoul(optarg, NULL, 0);
			break;
		case 'b':
			base = strtoul(optarg, NULL, 0);
			break;
		case 'h':
		default:
			usage(mode);
		}
	}

	switch (mode) {
	case DECODE:
		if (optind >= argc)
			usage(mode);
		decode(strtoul(argv[optind], NULL, 0));
		break;
	case ENCODE:
		if (!base) {
			if (!jack)
				jack = "Jack";
			if (!type)
				type = "Line Out";
			if (!location)
				location = "Ext Left";
			if (!connection)
				connection = "1/8";
			if (!color)
				color = "Unknown";
		}
		val = encode(jack, type, location, connection, color,
			     assoc, seq, misc, base);
		hda_log(HDA_LOG_INFO, "Pin config value = 0x%x\n", val);
		decode(val);
		break;
	}
	return 0;
}
