/*
 * hda-emu - simple HD-audio codec emulator for debugging snd-hda-intel driver
 *
 *  Codec-specific verbs and parameters
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
#include "hda-types.h"
#include "hda-log.h"

/*
 * get/put value in the cache; too stupid linear searching
 */
static int get_cached_verb(struct xhda_codec *codec, struct xhda_node *node,
			   unsigned int cmd)
{
	unsigned int verb = (cmd >> 8) & 0x7ff; /* common value for get/set */
	struct xhda_value_cache *val;

	for (val = codec->value_cache; val; val = val->next)
		if (val->verb == verb)
			return val->val;
	return 0;
}

static int set_verb_val(struct xhda_codec *codec, unsigned int verb,
			unsigned int cmd, unsigned int shift)
{
	struct xhda_value_cache *val;
	unsigned int mask = 0xff << (shift * 8);

	cmd = (cmd & 0xff) << (shift * 8);
	for (val = codec->value_cache; val; val = val->next)
		if (val->verb == verb) {
			val->val = (val->val & ~mask) | cmd;
			return 0;
		}
	val = calloc(1, sizeof(*val));
	if (!val) {
		hda_log(HDA_LOG_ERR, "No memory left\n");
		return -ENOMEM;
	}
	val->verb = verb;
	val->val = cmd;
	val->next = codec->value_cache;
	codec->value_cache = val;
	return 0;
}

static int set_cached_verb(struct xhda_codec *codec, struct xhda_node *node,
			   unsigned int cmd)
{
	return set_verb_val(codec, (cmd >> 8) & 0x7ff, cmd, 0);
}

/* second byte */
static int set_cached_verb2(struct xhda_codec *codec, struct xhda_node *node,
			    unsigned int cmd)
{
	return set_verb_val(codec, ((cmd >> 8) & 0x7ff) - 1, cmd, 1);
}

/* third byte */
static int set_cached_verb3(struct xhda_codec *codec, struct xhda_node *node,
			    unsigned int cmd)
{
	return set_verb_val(codec, ((cmd >> 8) & 0x7ff) - 2, cmd, 2);
}

/* fourth byte */
static int set_cached_verb4(struct xhda_codec *codec, struct xhda_node *node,
			    unsigned int cmd)
{
	return set_verb_val(codec, ((cmd >> 8) & 0x7ff) - 3, cmd, 3);
}

/*
 * IDT 92HD7xx
 */
static struct xhda_verb_table idt_92hd7xx_verbs[] = {
	{ 0xf70, get_cached_verb, "get_gpio_polarity" },
	{ 0x770, set_cached_verb, "set_gpio_polarity" },
	{ 0xf71, get_cached_verb, "get_gpio_drive" },
	{ 0x771, set_cached_verb, "set_gpio_drive" },
	{ 0xf78, get_cached_verb, "get_dmic" },
	{ 0x778, set_cached_verb, "set_dmic" },
	{ 0xfa0, get_cached_verb, "get_analog_loop" }, /* undocumented */
	{ 0x7a0, set_cached_verb, "set_analog_loop" },
	{ 0x7e0, set_cached_verb, "set_vs_power" },  /* undocumented */
	{ 0xfe0, get_cached_verb, "get_vs_power" },
	{ 0xfec, get_cached_verb, "get_power_map" },  /* undocumented */
	{ 0x7ec, set_cached_verb, "set_power_map" },
	{ 0xfee, get_cached_verb, "get_analog_beep" },
	{ 0x7ee, set_cached_verb, "set_analog_beep" },
	{ } /* terminator */
};

/*
 * IDT 92HD8xx
 */
static struct xhda_verb_table idt_92hd8xx_verbs[] = {
	{ 0xf70, get_cached_verb, "get_gpio_polarity" },
	{ 0x770, set_cached_verb, "set_gpio_polarity" },
	{ 0xf71, get_cached_verb, "get_gpio_drive" },
	{ 0x771, set_cached_verb, "set_gpio_drive" },
	{ 0xf74, get_cached_verb, "get_aux_audio" },
	{ 0x774, set_cached_verb, "set_aux_audio" },
	{ 0xf78, get_cached_verb, "get_dmic" },
	{ 0x778, set_cached_verb, "set_dmic" },
	{ 0xf80, get_cached_verb, "get_dac_mode" },
	{ 0x780, set_cached_verb, "set_dac_mode" },
	{ 0xf84, get_cached_verb, "get_adc_mode" },
	{ 0x784, set_cached_verb, "set_adc_mode" },
	{ 0xf88, get_cached_verb, "get_eapd_mode" },
	{ 0x788, set_cached_verb, "set_eapd_mode1" },
	{ 0x789, set_cached_verb2, "set_eapd_mode2" },
	{ 0xfc0, get_cached_verb, "get_port_use" },
	{ 0x7c0, set_cached_verb, "set_port_use" },
	{ 0x7d8, set_cached_verb, "set_vs_power" },
	{ 0xfd8, get_cached_verb, "get_vs_power" },
	{ 0xfec, get_cached_verb, "get_power_map" },
	{ 0x7ec, set_cached_verb, "set_power_map" },
	{ 0xfee, get_cached_verb, "get_analog_beep" },
	{ 0x7ee, set_cached_verb, "set_analog_beep" },
	{ 0xff4, get_cached_verb, "get_analog_btl" },
	{ 0x7f4, set_cached_verb, "set_analog_btl1" },
	{ 0x7f5, set_cached_verb2, "set_analog_btl2" },
	{ 0x7f6, set_cached_verb3, "set_analog_btl3" },
	{ 0xff8, get_cached_verb, "get_ana_capless" },
	{ 0x7f8, set_cached_verb, "set_ana_capless1" },
	{ 0x7f9, set_cached_verb2, "set_ana_capless2" },
	{ 0x7fa, set_cached_verb3, "set_ana_capless3" },
	{ } /* terminator */
};

/*
 * STAC codecs
 */
static struct xhda_verb_table stac927x_verbs[] = {
	{ 0xfeb, get_cached_verb, "get_analog_loop" }, /* undocumented */
	{ 0x7eb, set_cached_verb, "set_analog_loop" },
	{ } /* terminator */
};

static struct xhda_verb_table stac9205_verbs[] = {
	{ 0xfe0, get_cached_verb, "get_analog_loop" }, /* undocumented */
	{ 0x7e0, set_cached_verb, "set_analog_loop" },
	{ } /* terminator */
};

/*
 */

struct verb_ext_list {
	unsigned int id;
	struct xhda_verb_table *verbs;
	struct xhda_verb_table *params;
};

static struct verb_ext_list extensions[] = {
	{ .id = 0x111d7603, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d7604, .verbs = idt_92hd8xx_verbs },
	{ .id = 0x111d7605, .verbs = idt_92hd8xx_verbs },
	{ .id = 0x111d7608, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d7674, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d7675, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d7676, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76b0, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76b1, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76b2, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76b3, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76b4, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76b5, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76b6, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76b7, .verbs = idt_92hd7xx_verbs },

	{ .id = 0x83847618, .verbs = stac927x_verbs },
	{ .id = 0x83847619, .verbs = stac927x_verbs },
	{ .id = 0x83847616, .verbs = stac927x_verbs },
	{ .id = 0x83847617, .verbs = stac927x_verbs },
	{ .id = 0x83847614, .verbs = stac927x_verbs },
	{ .id = 0x83847615, .verbs = stac927x_verbs },
	{ .id = 0x83847620, .verbs = stac927x_verbs },
	{ .id = 0x83847621, .verbs = stac927x_verbs },
	{ .id = 0x83847622, .verbs = stac927x_verbs },
	{ .id = 0x83847623, .verbs = stac927x_verbs },
	{ .id = 0x83847624, .verbs = stac927x_verbs },
	{ .id = 0x83847625, .verbs = stac927x_verbs },
	{ .id = 0x83847626, .verbs = stac927x_verbs },
	{ .id = 0x83847627, .verbs = stac927x_verbs },
	{ .id = 0x83847628, .verbs = stac927x_verbs },
	{ .id = 0x83847629, .verbs = stac927x_verbs },
	{ .id = 0x83847645, .verbs = stac927x_verbs },
	{ .id = 0x83847646, .verbs = stac927x_verbs },

	{ .id = 0x83847698, .verbs = stac9205_verbs },
	{ .id = 0x838476a0, .verbs = stac9205_verbs },
	{ .id = 0x838476a1, .verbs = stac9205_verbs },
	{ .id = 0x838476a2, .verbs = stac9205_verbs },
	{ .id = 0x838476a3, .verbs = stac9205_verbs },
	{ .id = 0x838476a4, .verbs = stac9205_verbs },
	{ .id = 0x838476a5, .verbs = stac9205_verbs },
	{ .id = 0x838476a6, .verbs = stac9205_verbs },
	{ .id = 0x838476a7, .verbs = stac9205_verbs },

	{ }
};

void add_codec_extensions(struct xhda_codec *codec)
{
	struct verb_ext_list *v;

	for (v = extensions; v->id; v++) {
		if (v->id == codec->vendor_id) {
			codec->extended_verbs = v->verbs;
			codec->extended_parameters = v->params;
			break;
		}
	}

	/* other quirks */
	switch (codec->vendor_id) {
	case 0x14f15045:
	case 0x14f15047:
	case 0x14f15051:
		codec->pin_amp_workaround = 1;
		break;
	}

	/* Realtek specific COEF */
	if (!strcmp(codec->parsed_name, "Realtek ALC259"))
		hda_set_proc_coef(codec, 0x20, 0x00, 0x17);
	else if (!strcmp(codec->parsed_name, "Realtek ALC271X"))
		hda_set_proc_coef(codec, 0x20, 0x00, 0x18);

}
