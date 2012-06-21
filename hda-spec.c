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
	val = xalloc(sizeof(*val));
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
	{ 0x7e7, set_cached_verb, "idt_set_gpio" },
	{ 0xfec, get_cached_verb, "get_power_map" },  /* undocumented */
	{ 0x7ec, set_cached_verb, "set_power_map" },
	{ 0x7ed, set_cached_verb, "set_power_map2" },
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
	{ 0x7e7, set_cached_verb, "idt_set_gpio" },
	{ 0xfec, get_cached_verb, "get_power_map" },
	{ 0x7ec, set_cached_verb, "set_power_map" },
	{ 0x7ed, set_cached_verb, "set_power_map2" },
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

static struct xhda_verb_table vt1702_verbs[] = {
	{ 0xf73, get_cached_verb, "via_low_current" }, /* undocumented */
	{ 0xf81, get_cached_verb, "via_analog_loopback" }, /* undocumented */
	{ 0xf82, get_cached_verb, "via_gpio_ctrl" }, /* undocumented */
	{ 0xf88, get_cached_verb, "via_mixer_ctrl" }, /* undocumented */
	{ } /* terminator */
};

static struct xhda_verb_table vt1708b_verbs[] = {
	{ 0xf70, get_cached_verb, "via_low_current" }, /* undocumented */
	{ 0xf81, get_cached_verb, "via_analog_loopback" }, /* undocumented */
	{ } /* terminator */
};

static struct xhda_verb_table vt1708s_verbs[] = {
	{ 0xf73, get_cached_verb, "via_low_current" }, /* undocumented */
	{ 0xf81, get_cached_verb, "via_analog_loopback" }, /* undocumented */
	{ 0xf88, get_cached_verb, "via_mixer_ctrl" }, /* undocumented */
	{ 0xf98, get_cached_verb, "via_boost_ctrl" }, /* undocumented */
	{ } /* terminator */
};

static struct xhda_verb_table vt1716s_verbs[] = {
	{ 0xf73, get_cached_verb, "via_low_current" }, /* undocumented */
	{ 0xf81, get_cached_verb, "via_analog_loopback" }, /* undocumented */
	{ 0xf88, get_cached_verb, "via_mixer_ctrl" }, /* undocumented */
	{ 0xf8a, get_cached_verb, "via_boost_ctrl" }, /* undocumented */
	{ 0xf90, get_cached_verb, "via_mono_ctrl" }, /* undocumented */
	{ } /* terminator */
};

static struct xhda_verb_table vt1718s_verbs[] = {
	{ 0xfb2, get_cached_verb, "via_enagle_gain" }, /* undocumented */
	{ 0xf81, get_cached_verb, "via_analog_loopback" }, /* undocumented */
	{ 0xf88, get_cached_verb, "via_boost_ctrl" }, /* undocumented */
	{ 0xf73, get_cached_verb, "via_low_current" }, /* undocumented */
	{ 0xfb2, get_cached_verb, "via_mw_gain" }, /* undocumented */
	{ } /* terminator */
};

static struct xhda_verb_table vt2002p_verbs[] = {
	{ 0xf81, get_cached_verb, "via_analog_loopback" }, /* undocumented */
	{ 0xf93, get_cached_verb, "via_low_current" }, /* undocumented */
	{ 0xfb9, get_cached_verb, "via_boost_ctrl" }, /* undocumented */
	{ 0xfb8, get_cached_verb, "via_loop_ctrl" }, /* undocumented */
	{ 0xfe0, get_cached_verb, "via_classd_ctrl1" }, /* undocumented */
	{ 0xfe2, get_cached_verb, "via_classd_ctrl2" }, /* undocumented */
	{ 0xfe9, get_cached_verb, "via_classd_ctrl3" }, /* undocumented */
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
	{ .id = 0x111d7666, .verbs = idt_92hd8xx_verbs },
	{ .id = 0x111d7667, .verbs = idt_92hd8xx_verbs },
	{ .id = 0x111d7668, .verbs = idt_92hd8xx_verbs },
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
	{ .id = 0x111d76c0, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76c1, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76c2, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76c3, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76c4, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76c5, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76c6, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76c7, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76c8, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76c9, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76ca, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76cb, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76cc, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76cd, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76ce, .verbs = idt_92hd7xx_verbs },
	{ .id = 0x111d76d1, .verbs = idt_92hd8xx_verbs },
	{ .id = 0x111d76d4, .verbs = idt_92hd8xx_verbs },
	{ .id = 0x111d76d5, .verbs = idt_92hd8xx_verbs },
	{ .id = 0x111d76d9, .verbs = idt_92hd8xx_verbs },
	{ .id = 0x111d76e0, .verbs = idt_92hd8xx_verbs },
	{ .id = 0x111d76e3, .verbs = idt_92hd8xx_verbs },
	{ .id = 0x111d76e5, .verbs = idt_92hd8xx_verbs },
	{ .id = 0x111d76e7, .verbs = idt_92hd8xx_verbs },

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

	{ .id = 0x11061708, .verbs = vt1708b_verbs },
	{ .id = 0x11061709, .verbs = vt1708b_verbs },
	{ .id = 0x1106170a, .verbs = vt1708b_verbs },
	{ .id = 0x1106170b, .verbs = vt1708b_verbs },
	{ .id = 0x1106e710, .verbs = vt1708b_verbs },
	{ .id = 0x1106e711, .verbs = vt1708b_verbs },
	{ .id = 0x1106e712, .verbs = vt1708b_verbs },
	{ .id = 0x1106e713, .verbs = vt1708b_verbs },
	{ .id = 0x1106e714, .verbs = vt1708b_verbs },
	{ .id = 0x1106e715, .verbs = vt1708b_verbs },
	{ .id = 0x1106e716, .verbs = vt1708b_verbs },
	{ .id = 0x1106e717, .verbs = vt1708b_verbs },
	{ .id = 0x1106e720, .verbs = vt1708b_verbs },
	{ .id = 0x1106e721, .verbs = vt1708b_verbs },
	{ .id = 0x1106e722, .verbs = vt1708b_verbs },
	{ .id = 0x1106e723, .verbs = vt1708b_verbs },
	{ .id = 0x1106e724, .verbs = vt1708b_verbs },
	{ .id = 0x1106e725, .verbs = vt1708b_verbs },
	{ .id = 0x1106e726, .verbs = vt1708b_verbs },
	{ .id = 0x1106e727, .verbs = vt1708b_verbs },

	{ .id = 0x11060397, .verbs = vt1708s_verbs },
	{ .id = 0x11061397, .verbs = vt1708s_verbs },
	{ .id = 0x11062397, .verbs = vt1708s_verbs },
	{ .id = 0x11063397, .verbs = vt1708s_verbs },
	{ .id = 0x11064397, .verbs = vt1708s_verbs },
	{ .id = 0x11065397, .verbs = vt1708s_verbs },
	{ .id = 0x11066397, .verbs = vt1708s_verbs },
	{ .id = 0x11067397, .verbs = vt1708s_verbs },

	{ .id = 0x11060398, .verbs = vt1702_verbs },
	{ .id = 0x11061398, .verbs = vt1702_verbs },
	{ .id = 0x11062398, .verbs = vt1702_verbs },
	{ .id = 0x11063398, .verbs = vt1702_verbs },
	{ .id = 0x11064398, .verbs = vt1702_verbs },
	{ .id = 0x11065398, .verbs = vt1702_verbs },
	{ .id = 0x11066398, .verbs = vt1702_verbs },
	{ .id = 0x11067398, .verbs = vt1702_verbs },

	{ .id = 0x11060428, .verbs = vt1718s_verbs },
	{ .id = 0x11064428, .verbs = vt1718s_verbs },
	{ .id = 0x11060441, .verbs = vt1718s_verbs },
	{ .id = 0x11064441, .verbs = vt1718s_verbs },

	{ .id = 0x11060433, .verbs = vt1716s_verbs },
	{ .id = 0x1106a721, .verbs = vt1716s_verbs },

	{ .id = 0x11060438, .verbs = vt2002p_verbs },
	{ .id = 0x11064438, .verbs = vt2002p_verbs },
	{ .id = 0x11060448, .verbs = vt2002p_verbs },

	{ .id = 0x11060440, .verbs = vt1708s_verbs },
	{ .id = 0x11060446, .verbs = vt2002p_verbs },
	{ .id = 0x11068446, .verbs = vt2002p_verbs },

	{ }
};

/*
 * fixups
 */

static struct xhda_node *find_node(struct xhda_codec *codec, unsigned int nid)
{
	struct xhda_node *node;

	for (node = &codec->afg; node; node = node->next)
		if (node->nid == nid)
			return node;
	return NULL;
}

static void fixup_via_mixer(struct xhda_codec *codec)
{
	int i;
	struct xhda_node *node = find_node(codec, 0x21);

	if (!node)
		return;
	for (i = 0; i < node->num_nodes; i++) {
		if (node->node[i] == 0x08)
			return;
	}
	node->node[node->num_nodes++] = 0x08;
}

struct fixup_list {
	unsigned int vendor_id;
	void (*func)(struct xhda_codec *);
};

static struct fixup_list fixups[] = {
	{ 0x11060428, fixup_via_mixer },
	{ 0x11064428, fixup_via_mixer },
	{ 0x11060438, fixup_via_mixer },
	{ 0x11064438, fixup_via_mixer },
	{ 0x11060448, fixup_via_mixer },
	{ }
};

static void apply_fixups(struct xhda_codec *codec)
{
	struct fixup_list *p;

	for (p = fixups; p->vendor_id; p++) {
		if (p->vendor_id == codec->vendor_id) {
			p->func(codec);
			return;
		}
	}
}

void add_codec_extensions(struct xhda_codec *codec)
{
	struct verb_ext_list *v;

	apply_fixups(codec);

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
	if (!codec->parsed_name)
		return;
	if (!strcmp(codec->parsed_name, "Realtek ALC259"))
		hda_set_proc_coef(codec, 0x20, 0x00, 0x2017);
	else if (!strcmp(codec->parsed_name, "Realtek ALC258"))
		hda_set_proc_coef(codec, 0x20, 0x00, 0x3010);
	else if (!strcmp(codec->parsed_name, "Realtek ALC271X"))
		hda_set_proc_coef(codec, 0x20, 0x00, 0x3010);
	else if (!strcmp(codec->parsed_name, "Realtek ALC272X"))
		hda_set_proc_coef(codec, 0x20, 0x00, 0x4010);
	else if (!strcmp(codec->parsed_name, "Realtek ALC269VB"))
		hda_set_proc_coef(codec, 0x20, 0x00, 0x10);
	else if (!strcmp(codec->parsed_name, "Realtek ALC887-VD"))
		hda_set_proc_coef(codec, 0x20, 0x00, 0x30);
	else if (!strcmp(codec->parsed_name, "Realtek ALC888-VD"))
		hda_set_proc_coef(codec, 0x20, 0x00, 0x30);
	else if (!strcmp(codec->parsed_name, "Realtek ALC661"))
		hda_set_proc_coef(codec, 0x20, 0x00, 0x8020);

}
