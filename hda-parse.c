/*
 * hda-emu - simple HD-audio codec emulator for debugging snd-hda-intel driver
 *
 * Parse a codec proc file
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
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "hda-types.h"
#include "hda-log.h"

enum {
	PARSE_START,
	PARSE_ROOT,
	PARSE_DEFAULT_PCM,
	PARSE_GPIO,
	PARSE_NODE,
	PARSE_NODE_PCM,
	PARSE_NODE_CONNECTIONS,
	/* post-codec */
	PARSE_POST,
	PARSE_SYSFS,
};

static struct xhda_codec *codec;
static struct xhda_node *current_node;
static int parse_mode;

static const char *strmatch(const char *buf, const char *str)
{
	int len = strlen(str);
	if (!strncmp(buf, str, len))
		return buf + len;
	return NULL;
}

static struct xhda_node *create_node(unsigned int nid, unsigned int wcaps)
{
	struct xhda_node *node;
	struct xhda_node **prevp;

	node = xalloc(sizeof(*node));
	node->nid = nid;
	node->wcaps = wcaps;

	if (nid > codec->num_widgets)
		codec->num_widgets = nid;
	/* insert it */
	for (prevp = &codec->afg.next; *prevp; prevp = &(*prevp)->next) {
		if ((*prevp)->nid >= nid) {
			if ((*prevp)->nid == nid) {
				hda_log(HDA_LOG_ERR,
					"Duplicated widget NID 0x%x\n", nid);
				free(node);
				return NULL;
			}
			break;
		}
	}
	node->next = *prevp;
	*prevp = node;
	return node;
}

static int parse_codec_recursive(const char *buffer);

static int parse_node(const char *buf)
{
	int nid, wcaps;
	const char *p;

	if (sscanf(buf, "Node 0x%02x", &nid) != 1) {
		hda_log(HDA_LOG_ERR, "No NID defined\n");
		hda_log(HDA_LOG_ERR, "  bad line: %s\n", buf);
		return -EINVAL;
	}
	p = strstr(buf, "wcaps ");
	if (!p) {
		hda_log(HDA_LOG_ERR, "No wcaps defined\n");
		hda_log(HDA_LOG_ERR, "  bad line: %s\n", buf);
		return -EINVAL;
	}
	if (sscanf(p, "wcaps 0x%x:", &wcaps) != 1) {
		hda_log(HDA_LOG_ERR, "No wcaps defined\n");
		hda_log(HDA_LOG_ERR, "  bad line: %s\n", buf);
		return -EINVAL;
	}
	current_node = create_node(nid, wcaps);
	if (!current_node)
		return -ENOMEM;
	parse_mode = PARSE_NODE;
	return 0;
}

static int parse_old_pcm_info(const char *buf, struct xhda_pcm_info *pcm)
{
	if (sscanf(buf, "rates 0x%x, bits 0x%02x, types 0x%x",
		   &pcm->rates, &pcm->bits, &pcm->formats) != 3) {
		hda_log(HDA_LOG_ERR, "invalid old amp_caps\n");
		hda_log(HDA_LOG_ERR, "  bad line: %s\n", buf);
		return -EINVAL;
	}
	return 0;
}

static int parse_amp_caps(const char *buf, struct xhda_amp_caps *caps)
{
	int n;
	if (strmatch(buf, "N/A"))
		return 0;
	n = sscanf(buf, "ofs=0x%02x, nsteps=0x%02x, stepsize=0x%02x, mute=%d",
		   &caps->ofs, &caps->nsteps, &caps->stepsize, &caps->mute);
	if (n != 4) {
		hda_log(HDA_LOG_ERR, "invalid amp_caps\n");
		hda_log(HDA_LOG_ERR, "  bad line: %s\n", buf);
		return -EINVAL;
	}
	caps->override = 1;
	return 0;
}

static int parse_amp_vals(const char *buf, struct xhda_amp_vals *vals,
			  struct xhda_amp_vals *orig_vals)
{
	while (*buf && isspace(*buf))
		buf++;
	if (!*buf || *buf == '\n')
		return 0; /* no values */
	if (*buf == '[') {
		int n = 0;
		/* new format */
		do {
			int v1, v2;
			if (sscanf(buf, "[0x%02x 0x%02x]", &v1, &v2) != 2)
				break;
			vals->vals[n][0] = v1;
			vals->vals[n][1] = v2;
			n++;
			while (*buf != ']')
				buf++;
			buf++;
			while (*buf && isspace(*buf))
				buf++;
		} while (*buf && *buf != '\n');
	} else {
		/* old format */
		char *p = (char *)buf;
		vals->vals[0][0] = strtoul(p, &p, 0);
		vals->vals[0][1] = strtoul(p, &p, 0);
	}
	if (orig_vals)
		*orig_vals = *vals;
	return 0;
}

struct dig_conv_bits {
	const char *name;
	unsigned int bits;
};

/* DIGITAL1 bits */
#define AC_DIG1_ENABLE			(1<<0)
#define AC_DIG1_V			(1<<1)
#define AC_DIG1_VCFG			(1<<2)
#define AC_DIG1_EMPHASIS		(1<<3)
#define AC_DIG1_COPYRIGHT		(1<<4)
#define AC_DIG1_NONAUDIO		(1<<5)
#define AC_DIG1_PROFESSIONAL		(1<<6)
#define AC_DIG1_LEVEL			(1<<7)

static struct dig_conv_bits dig_conv_bits[] = {
	{ " Enabled", AC_DIG1_ENABLE },
	{ " ValidityCfg", AC_DIG1_VCFG },
	{ " Validity", AC_DIG1_V },
	{ " Preemphasis", AC_DIG1_EMPHASIS },
	{ " Copyright", AC_DIG1_COPYRIGHT },
	{ " Non-Audio", AC_DIG1_NONAUDIO },
	{ " Pro", AC_DIG1_PROFESSIONAL },
	{ " GenLevel", AC_DIG1_LEVEL },
	{}
};

static int parse_node_digital(const char *buf, struct xhda_node *node)
{
	for (;;) {
		const struct dig_conv_bits *dig;
		for (dig = dig_conv_bits; dig->name; dig++) {
			const char *p = strmatch(buf, dig->name);
			if (p) {
				node->dig_conv |= dig->bits;
				buf = p;
				break;
			}
		}
		if (!dig->name)
			return 0;
	}
}

static int parse_node_items(const char *buf)
{
	struct xhda_node *node = current_node;
	const char *head = buf;
	const char *p;

	if (*head == ' ') {
		int i;
		for (i = 0; i < 2 && *head == ' '; i++)
			head++;
	}
	if ((p = strmatch(head, "Amp-In caps: "))) {
		return parse_amp_caps(p, &node->amp_in_caps);
	} else if ((p = strmatch(head, "Amp-Out caps: "))) {
		return parse_amp_caps(p, &node->amp_out_caps);
	} else if ((p = strmatch(head, "Amp-In vals: "))) {
		return parse_amp_vals(p, &node->amp_in_vals,
				      &node->orig_amp_in_vals);
	} else if ((p = strmatch(head, "Amp-Out vals: "))) {
		return parse_amp_vals(p, &node->amp_out_vals,
				      &node->orig_amp_out_vals);
	} else if ((p = strmatch(head, "Pin-ctls: "))) {
		node->pinctl = strtoul(p, NULL, 0);
		if (node->pincap)
			hda_verify_pin_ctl(node, 0, &node->pinctl);
		node->orig_pinctl = node->pinctl;
	} else if ((p = strmatch(head, "Pincap "))) {
		const char *end;
		/* a special hack to avoid the old bug */
		if (strmatch(p, "0x08") &&
		    ((end = strchr(p, ':'))) &&
		    (size_t)(end - p) != 10)
			node->pincap = strtoul(p + 4, NULL, 16);
		else
			node->pincap = strtoul(p, NULL, 0);
	} else if ((p = strmatch(head, "Pin Default "))) {
		node->pin_default = strtoul(p, NULL, 0);
	} else if ((p = strmatch(head, "Unsolicited: "))) {
		/* not yet */
	} else if ((p = strmatch(head, "Power: "))) {
		sscanf(p, "setting=D%d, actual=D%d",
		       &node->power_setting, &node->power_current);
	} else if ((p = strmatch(head, "Processing caps: "))) {
		sscanf(p, "benign=%d, ncoeff=%d",
		       &node->coef_benign, &node->coef_num);
	} else if ((p = strmatch(head, "Digital:"))) {
		return parse_node_digital(p, node);
	} else if ((p = strmatch(head, "Digital category: "))) {
		node->dig_category = strtoul(p, NULL, 0);
	} else if ((p = strmatch(head, "PCM:"))) {
		if (*p == ' ')
			return parse_old_pcm_info(p + 1, &node->pcm);
		parse_mode = PARSE_NODE_PCM;
	} else if ((p = strmatch(head, "Connection: "))) {
		node->num_nodes = strtoul(p, NULL, 0);
		if (node->num_nodes >= 128) {
			hda_log(HDA_LOG_WARN, "Clearing invalid connection# %d @ NID 0x%x\n", node->num_nodes, node->nid);
			node->num_nodes = 0;
		}
		if (node->num_nodes)
			parse_mode = PARSE_NODE_CONNECTIONS;
	}
	return 0;
}

static int parse_node_connections(const char *buf)
{
	struct xhda_node *node = current_node;
	char *p = (char *)buf;
	int i;

	for (i = 0; i < node->num_nodes; i++) {
		while (*p && (*p == '*' || isspace(*p)))
			p++;
		node->node[i] = strtoul(p, &p, 0);
		if (*p == '*') {
			node->curr_conn = i;
			p++;
		}
	}
	parse_mode = PARSE_ROOT;
	current_node = NULL; /* reset */
	return 0;
}

static int parse_pcm_items(const char *buf, struct xhda_pcm_info *pcm)
{
	const char *p;

	while (*buf && isspace(*buf))
		buf++;
	if ((p = strmatch(buf, "rates [")))
		pcm->rates = strtoul(p, NULL, 0);
	else if ((p = strmatch(buf, "bits ["))) {
		pcm->bits = strtoul(p, NULL, 0);
		if (pcm->bits >= 0x1000) {
			/* workaround a temporary bug */
			pcm->bits >>= 16;
		}
	} else if ((p = strmatch(buf, "formats [")))
		pcm->formats = strtoul(p, NULL, 0);
	else
		return 1;
	return 0;
}

static int parse_default_pcm_items(const char *buf)
{
	int err = parse_pcm_items(buf, &codec->afg.pcm);
	if (err < 0)
		return err;
	if (err > 0) {
		parse_mode = PARSE_ROOT;
		return parse_codec_recursive(buf);
	}
	return 0;
}

static int parse_default_pcm(const char *buf)
{
	if (*buf == ' ') {
		/* old format */
		return parse_old_pcm_info(buf + 1, &codec->afg.pcm);
	}
	parse_mode = PARSE_DEFAULT_PCM;
	return 0;
}

static int parse_node_pcm(const char *buf)
{
	int err = parse_pcm_items(buf, &current_node->pcm);
	if (err < 0)
		return err;
	if (err > 0) {
		parse_mode = PARSE_NODE;
		return parse_codec_recursive(buf);
	}
	return 0;
}

static int parse_gpio(const char *buf)
{
	unsigned int gpio_io, gpio_o, gpio_i, gpio_unsol, gpio_wake;
	struct xhda_node *node = current_node ? current_node : &codec->afg;

	while (*buf && isspace(*buf))
		buf++;
	if (sscanf(buf, "io=%d, o=%d, i=%d, unsolicited=%d, wake=%d",
		   &gpio_io, &gpio_o, &gpio_i, &gpio_unsol, &gpio_wake) != 5) {
		/* an invalid line; skip */
		return 0;
	}
	node->gpio_cap = gpio_io |
		(gpio_o << 8) |
		(gpio_i << 16) |
		(gpio_unsol << 30) |
		(gpio_wake << 31);
	parse_mode = PARSE_GPIO;
	return 0;
}

static int parse_gpio_items(const char *buf)
{
	struct xhda_node *node = current_node ? current_node : &codec->afg;
	unsigned int i, enable, dir, wake, sticky, data;
	unsigned int unsol = 0;

	if (sscanf(buf, "  IO[%d]: enable=%d, dir=%d, wake=%d, sticky=%d, data=%d, unsol=%d",
		   &i, &enable, &dir, &wake, &sticky, &data, &unsol) != 7) {
		if (sscanf(buf, "  IO[%d]: enable=%d, dir=%d, wake=%d, sticky=%d, data=%d",
			   &i, &enable, &dir, &wake, &sticky, &data) != 6) {
			/* an invalid line; abort */
			parse_mode = PARSE_NODE;
			return parse_codec_recursive(buf);
		}
	}
	node->gpio_mask |= (enable << i);
	node->gpio_dir |= (dir << i);
	node->gpio_wake |= (wake << i);
	node->gpio_sticky |= (sticky << i);
	node->gpio_data |= (data << i);
	node->gpio_unsol |= (unsol << i);
	return 0;
}

static int in_modem_whitelist(struct xhda_codec *codec)
{
	switch (codec->vendor_id) {
	case 0x14f15045:
	case 0x14f15047:
	case 0x14f15051:
	case 0x14f15067:
	case 0x14f15069:
		return 1;
	}
	return 0;
}

static int parse_root(const char *buffer)
{
	const char *p;

	if ((p = strmatch(buffer, "Address: "))) {
		codec->addr = strtoul(p, NULL, 0);
	} else if ((p = strmatch(buffer, "Function Id: "))) {
		unsigned int id;
		id = strtoul(p, NULL, 0);
		if (strmatch(buffer, "AFG Function Id: ")) {
			if (id != 1)
				hda_log(HDA_LOG_WARN, "AFG ID != 1 (%d)\n", id);
			codec->function_id = id;
		} else if (strmatch(buffer, "MFG Function Id: "))
			codec->modem_function_id = id;
		else
			codec->function_id = 0x01; /* workaround for old bug */
		return 0;
	} else if ((p = strmatch(buffer, "Vendor Id: "))) {
		codec->vendor_id = strtoul(p, NULL, 0);
		return 0;
	} else if ((p = strmatch(buffer, "Subsystem Id: "))) {
		codec->subsystem_id = strtoul(p, NULL, 0);
		return 0;
	} else if ((p = strmatch(buffer, "Revision Id: "))) {
		codec->revision_id = strtoul(p, NULL, 0);
		return 0;
	} else if ((p = strmatch(buffer, "Default PCM:"))) {
		return parse_default_pcm(p);
	} else if ((p = strmatch(buffer, "Default Amp-In caps: "))) {
		return parse_amp_caps(p, &codec->afg.amp_in_caps);
	} else if ((p = strmatch(buffer, "Default Amp-Out caps: "))) {
		return parse_amp_caps(p, &codec->afg.amp_out_caps);
	} else if ((p = strmatch(buffer, "GPIO: "))) {
		return parse_gpio(p);
	} else if (strmatch(buffer, "Node ")) {
		return parse_node(buffer);
	} else if ((p = strmatch(buffer, "Modem Function Group: "))) {
		codec->mfg.nid = strtoul(p, NULL, 0);
		if (!codec->function_id && !in_modem_whitelist(codec))
			return -EBADFD;
		return 0;
	}
	return 0; /* ignore */
}

static int parse_codec_recursive(const char *buffer)
{
	switch (parse_mode) {
	case PARSE_ROOT:
		return parse_root(buffer);
	case PARSE_DEFAULT_PCM:
		return parse_default_pcm_items(buffer);
	case PARSE_GPIO:
		return parse_gpio_items(buffer);
	case PARSE_NODE:
		return parse_node_items(buffer);
	case PARSE_NODE_PCM:
		return parse_node_pcm(buffer);
	case PARSE_NODE_CONNECTIONS:
		return parse_node_connections(buffer);
	}
	return 0;
}
	
/* check the extra information from alsa-info output */
static int check_alsa_info(char *line, int override)
{
	int dummy, classid, vendor, id, rev;

	if (sscanf(line, "%02x:%02x.%1d %04x: %04x:%04x (rev %02x)",
		   &dummy, &dummy, &dummy, &classid, &vendor, &id, &rev) == 7) {
		if (classid == 0x0403 && (override || !codec->pci_vendor)) {
			codec->pci_vendor = vendor;
			codec->pci_device = id;
			codec->pci_revision = rev;
		}
		return 0;
	}
	if (sscanf(line, "%02x:%02x.%1d Audio device [%04x]:",
		   &dummy, &dummy, &dummy, &classid) == 4 &&
	    classid == 0x0403 && 
	    (override || !codec->pci_vendor)) {
		if (sscanf(line + strlen(line) - 12, "[%04x:%04x]",
			   &vendor, &id) == 2) {
			codec->pci_vendor = vendor;
			codec->pci_device = id;
		}
		return 0;
	}
	if (sscanf(line, "\tSubsystem: %04x:%04x", &vendor, &id) == 2 ||
	    sscanf(line, "        Subsystem: %04x:%04x", &vendor, &id) == 2) {
		if (override || !codec->pci_subvendor) {
			codec->pci_subvendor = vendor;
			codec->pci_subdevice = id;
			hda_log(HDA_LOG_INFO, "Getting PCI ID %04x:%04x (%04x:%04x) rev %02x\n",
				codec->pci_vendor, codec->pci_device,
				codec->pci_subvendor, codec->pci_subdevice,
				codec->pci_revision);
		}
		return 1;
	}
	if (!strncmp(line, "\tSubsystem: ", 12) ||
	    !strncmp(line, "        Subsystem: ", 19)) {
		if ((override || !codec->pci_subvendor) &&
		    sscanf(line + strlen(line) - 12, "[%04x:%04x]",
			   &vendor, &id) == 2) {
			codec->pci_subvendor = vendor;
			codec->pci_subdevice = id;
			hda_log(HDA_LOG_INFO, "Getting PCI ID %04x:%04x (%04x:%04x) rev %02x\n",
				codec->pci_vendor, codec->pci_device,
				codec->pci_subvendor, codec->pci_subdevice,
				codec->pci_revision);
			return 1;
		}
	}

	return 0;
}

static int add_sysfs_list(struct xhda_codec *codec, int *vals)
{
	struct xhda_sysfs_value *item = xalloc(sizeof(*item));
	struct xhda_sysfs_value *p;

	memcpy(item->val, vals, sizeof(item->val));
	item->next = NULL;
	if (!codec->sysfs_list->entry.vals)
		codec->sysfs_list->entry.vals = item;
	else {
		for (p = codec->sysfs_list->entry.vals; p->next; p = p->next)
			;
		p->next = item;
	}
	return 0;
}

static int add_sysfs_hint(struct xhda_codec *codec, char *line)
{
	struct xhda_sysfs_hints *item = xalloc(sizeof(*item));
	struct xhda_sysfs_hints *p;

	item->line = strdup(line);
	item->next = NULL;
	if (!codec->sysfs_list->entry.hints)
		codec->sysfs_list->entry.hints = item;
	else {
		for (p = codec->sysfs_list->entry.hints; p->next; p = p->next)
			;
		p->next = item;
	}
	return 0;
}

static int do_parse_sysfs(struct xhda_codec *codec, char *buffer)
{
	if (strmatch(buffer, "!!--"))
		return 0;
	if (strmatch(buffer, "/sys/class/sound/hwC")) {
		char *p;
		struct xhda_sysfs_list *sys;

		sys = xalloc(sizeof(*sys));
		sys->id = strdup(buffer + strlen("/sys/class/sound/"));
		p = strchr(sys->id, ':');
		if (p)
			*p = 0;
		p = strchr(sys->id, '/');
		if (!p) {
			free(sys->id);
			free(sys);
			return 0;
		}
		if (!strcmp(p + 1, "init_verbs"))
			sys->type = XHDA_SYS_VERBS;
		else if (!strcmp(p + 1, "hints"))
			sys->type = XHDA_SYS_HINTS;
		else
			sys->type = XHDA_SYS_PINCFG;
		hda_log(HDA_LOG_INFO, "Adding sysfs entry %s\n", sys->id);
		sys->next = codec->sysfs_list;
		sys->entry.vals = NULL;
		codec->sysfs_list = sys;
		return 0;
	}
	if (strchr(buffer, '=')) {
		add_sysfs_hint(codec, buffer);
	} else if (isdigit(*buffer)) {
		int val[3];
		if (!codec->sysfs_list)
			return 0;
		switch (codec->sysfs_list->type) {
		case XHDA_SYS_VERBS:
			if (sscanf(buffer, "%i %i %i", &val[0], &val[1], &val[2]) != 3)
				return 0;
			break;
		case XHDA_SYS_PINCFG:
			if (sscanf(buffer, "%i %i", &val[0], &val[1]) != 2)
				return 0;
			val[2] = 0;
			break;
		default:
			return 0;
		}
		add_sysfs_list(codec, val);
		return 0;
	}
	if (!*buffer || isspace(*buffer))
		return 0;
	return 1; /* no more data */
}

static void clear_codec(struct xhda_codec *codec)
{
	free(codec->parsed_name);
	codec->parsed_name = NULL;
	codec->function_id = 0;
	codec->modem_function_id = 0;
	codec->mfg.nid = 0;
	codec->num_widgets = 1;
}

int parse_codec_proc(FILE *fp, struct xhda_codec *codecp, int codec_index)
{
	char buffer[256], *p;
	int curidx = -1, pciidx = 0;
	int err = 0;

	codec = codecp;
	parse_mode = PARSE_START;
	current_node = NULL;

	codec->num_widgets = 1;
	codec->afg.nid = 0x01;
	while (fgets(buffer, sizeof(buffer), fp)) {
	again:
		if (parse_mode == PARSE_START) {
			if (!strmatch(buffer, "Codec: ")) {
				if (check_alsa_info(buffer, pciidx == codec_index))
					pciidx++;
				continue;
			}
			curidx++;
			if (codec_index < 0 || curidx == codec_index)
				parse_mode = PARSE_ROOT;
			p = strrchr(buffer, '\n');
			if (p)
				*p = 0;
			codec->parsed_name = strdup(buffer + strlen("Codec: "));
			if (!codec->parsed_name)
				return -ENOMEM;
		} else if (strmatch(buffer, "Codec: ")) {
			if (codec->mfg.nid &&
			    codec->num_widgets <= codec->mfg.nid &&
			    !in_modem_whitelist(codec)) {
				hda_log(HDA_LOG_INFO, "Codec %d is a modem codec, skipping\n", curidx);
				parse_mode = PARSE_START;
				clear_codec(codec);
				goto again;
			}
			parse_mode = PARSE_POST;
			continue;
		}

		if (parse_mode < PARSE_POST) {
			if (!isspace(*buffer) &&
			    (*buffer < 'A' || *buffer >= 'Z')) {
				/* no more lines */
				parse_mode = PARSE_POST;
			}
		}
		if (parse_mode == PARSE_POST) {
			if (strmatch(buffer, "!!Sysfs Files"))
				parse_mode = PARSE_SYSFS;
			continue;
		}
		if (parse_mode == PARSE_SYSFS) {
			if (do_parse_sysfs(codec, buffer) > 0)
				parse_mode = PARSE_POST;
			continue;
		}

		if (parse_mode > PARSE_ROOT && *buffer != ' ') {
			current_node = NULL;
			parse_mode = PARSE_ROOT;
		}
		err = parse_codec_recursive(buffer);
		if (err == -EBADFD) {
			if (codec_index < 0) {
				hda_log(HDA_LOG_INFO, "Codec %d is a modem codec, skipping\n", curidx);
				parse_mode = PARSE_START;
				clear_codec(codec);
				continue;
			}
			hda_log(HDA_LOG_ERR, "Codec %d is a modem codec, aborting\n", curidx);
			return err;
		}
		else if (err < 0) {
			hda_log(HDA_LOG_ERR, "ERROR %d\n", err);
			return err;
		}
	}

	if (curidx < codec_index) {
		hda_log(HDA_LOG_ERR, "Codec index %d requested, but found only %d codecs\n", codec_index, curidx+1);
		return -ENODEV;
	}

	add_codec_extensions(codec);

	return 0;
}
