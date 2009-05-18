/*
 * hda-emu - simple HD-audio codec emulator for debugging snd-hda-intel driver
 *
 * Pretty simple command-line shell
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

#include <sound/core.h>
#include <sound/control.h>

struct delayed_work *__work_pending;

extern struct list_head snd_ctl_list_head;

static char *gettoken(char **bufp)
{
	char *p;
	char *token = *bufp;

	while (*token && isspace(*token))
		token++;
	if (!*token || *token == '\n')
		return NULL;
	p = token;
	while (*p && !(isspace(*p) || *p == '\n'))
		p++;
	if (*p)
		*p++ = 0;
	*bufp = p;
	return token;
}

struct usage_table {
	const char *cmd;
	const char *line;
	const char *desc;
};
	
static struct usage_table usage_str[] = {
	{ "list", "list",
	  "Show all control elements" },
	{ "get", "get numid",
	  "Get the contents of the given control element" },
	{ "set", "set numid val [val2]",
	  "Set the contents of the given control element" },
	{ "dump", "dump [nid [filename]]",
	  "Dump codec contents in the proc file format; nid = 0 means all widgets " },
	{ "jack", "jack numid [val]",
	  "Get jack state or set jack state; val = 0 or 1" },
	{ "unsol", "unsol numid",
	  "Issue an unsolicited event" },
	{ "option", "option variable [val]",
	  "Get/set module option value" },
	{ "help", "help [command]",
	  "Show help texts" },
	{ "verb", "verb nid cmd parameter",
	  "Execute a verb" },
	{ "PCM", "PCM [pcm-id dir [rate [channels [format-bits]]]]",
	  "List PCM streams or test the given PCM stream" },
	{ "pm", "pm",
	  "Test suspend/resume cycle" },
#ifdef CONFIG_SND_HDA_RECONFIG
	{ "fs", "fs {get|set|list} file args...",
	  "Read or write sysfs files" },
#endif
	{ "quit", "quit",
	  "Quit the program" },
	{ }
};

static void usage(const char *cmd)
{
	struct usage_table *tbl;

	for (tbl = usage_str; tbl->cmd; tbl++) {
		if (cmd && !strcmp(cmd, tbl->cmd)) {
			hda_log(HDA_LOG_INFO, "Usage: %s\n", tbl->line);
			hda_log(HDA_LOG_INFO, "%s\n", tbl->desc);
			return;
		}
	}
	hda_log(HDA_LOG_INFO, "Available commands:");
	for (tbl = usage_str; tbl->cmd; tbl++)
		hda_log(HDA_LOG_INFO, " %s", tbl->cmd);
	hda_log(HDA_LOG_INFO, "\n");
	hda_log(HDA_LOG_INFO, "Run \"help CMD\" for details\n");
}

static void show_elements(char *line)
{
	struct snd_kcontrol *kctl;

	list_for_each_entry(kctl, &snd_ctl_list_head, list) {
		hda_log(HDA_LOG_INFO, "%d: %s:%d (%d)\n",
			kctl->id.numid, kctl->id.name, kctl->id.index,
			kctl->count);
	}
}

static char *get_enum_name(struct snd_kcontrol *kctl, int item)
{
	static struct snd_ctl_elem_info uinfo;
	memset(&uinfo, 0, sizeof(uinfo));
	uinfo.id = kctl->id;
	uinfo.value.enumerated.item = item;
	kctl->info(kctl, &uinfo);
	return uinfo.value.enumerated.name;
}

static void get_element(char *line)
{
	struct snd_kcontrol *kctl;
	struct snd_ctl_elem_info uinfo;
	struct snd_ctl_elem_value uval;
	char *p;
	int i, err;

	p = gettoken(&line);
	if (!p) {
		usage("get");
		return;
	}
	kctl = snd_ctl_find_numid(NULL, atoi(p));
	if (!kctl) {
		hda_log(HDA_LOG_INFO, "No element %s\n", p);
		return;
	}
	
	memset(&uinfo, 0, sizeof(uinfo));
	uinfo.id = kctl->id;
	err = kctl->info(kctl, &uinfo);
	if (err < 0) {
		hda_log(HDA_LOG_INFO, "Error in info for %s\n", p);
		return;
	}

	memset(&uval, 0, sizeof(uval));
	uval.id = kctl->id;
	err = kctl->get(kctl, &uval);
	if (err < 0) {
		hda_log(HDA_LOG_INFO, "Error in get for %s\n", p);
		return;
	}

	hda_log(HDA_LOG_INFO, "%d %s:%d\n",
		kctl->id.numid, kctl->id.name, kctl->id.index);
	switch (uinfo.type) {
	case SNDRV_CTL_ELEM_TYPE_INTEGER:
	case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
		hda_log(HDA_LOG_INFO, "MIN/MAX: %ld/%ld, ",
			uinfo.value.integer.min, uinfo.value.integer.max);
		hda_log(HDA_LOG_INFO, " VAL:");
		for (i = 0; i < uinfo.count; i++)
			hda_log(HDA_LOG_INFO, " [%ld]",
				uval.value.integer.value[i]);
		hda_log(HDA_LOG_INFO, "\n");
		break;
	case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
		hda_log(HDA_LOG_INFO, "ITEM:");
		for (i = 0; i < uinfo.value.enumerated.items; i++)
			hda_log(HDA_LOG_INFO, " %d:%s,",
				i, get_enum_name(kctl, i));
		hda_log(HDA_LOG_INFO, " VAL:");
		for (i = 0; i < uinfo.count; i++)
			hda_log(HDA_LOG_INFO, " [%s]",
				get_enum_name(kctl, uval.value.enumerated.item[i]));
		hda_log(HDA_LOG_INFO, "\n");
		break;
	case SNDRV_CTL_ELEM_TYPE_IEC958:
		hda_log(HDA_LOG_INFO, "Can't handle IEC958 yet\n");
		break;
	default:
		hda_log(HDA_LOG_INFO, "Unsupported type %d\n", uinfo.type);
		break;
	}
}

static void set_element(char *line)
{
	struct snd_kcontrol *kctl;
	struct snd_ctl_elem_info uinfo;
	struct snd_ctl_elem_value uval;
	char *p;
	unsigned int numid;
	int i, err;

	p = gettoken(&line);
	if (!p) {
		usage("set");
		return;
	}
	numid = atoi(p);
	kctl = snd_ctl_find_numid(NULL, numid);
	if (!kctl) {
		hda_log(HDA_LOG_INFO, "No element %d\n", numid);
		return;
	}
	
	if (!(kctl->vd[0].access & SNDRV_CTL_ELEM_ACCESS_WRITE)) {
		hda_log(HDA_LOG_INFO, "Have no write access\n");
		return;
	}

	memset(&uinfo, 0, sizeof(uinfo));
	uinfo.id = kctl->id;
	err = kctl->info(kctl, &uinfo);
	if (err < 0) {
		hda_log(HDA_LOG_INFO, "Error in info for %d\n", numid);
		return;
	}

	switch (uinfo.type) {
	case SNDRV_CTL_ELEM_TYPE_INTEGER:
	case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
	case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
		break;
	default:
		hda_log(HDA_LOG_INFO, "Can't set type %d\n", uinfo.type);
		return;
	}

	memset(&uval, 0, sizeof(uval));
	uval.id = kctl->id;
	for (i = 0; i < uinfo.count; i++) {
		p = gettoken(&line);
		if (!p) {
			hda_log(HDA_LOG_INFO, "No value #%d is given\n", i);
			return;
		}
		switch (uinfo.type) {
		case SNDRV_CTL_ELEM_TYPE_INTEGER:
		case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
			uval.value.integer.value[i] = atoi(p);
			break;
		case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
			uval.value.enumerated.item[i] = atoi(p);
			break;
		}
	}
	err = kctl->put(kctl, &uval);
	if (err < 0) {
		hda_log(HDA_LOG_INFO, "Error in get for %d\n", numid);
		return;
	}
}

static void dump_proc(char *line)
{
	unsigned int nid = 0;
	char *p, *file = NULL;

	p = gettoken(&line);
	if (p) {
		nid = strtoul(p, NULL, 0);
		file = gettoken(&line);
	}
	hda_log_dump_proc(nid, file);
}

static void handle_module_option(char *line)
{
	char *opt;

	opt = gettoken(&line);
	if (!opt) {
		usage("option");
		return;
	}
	if (!strcmp(opt, "power_save")) {
		extern int *power_save_parameter;
		char *p = gettoken(&line);
		if (!p)
			hda_log(HDA_LOG_INFO, "Power-save = %d\n",
				hda_get_power_save());
		else
			hda_set_power_save(atoi(p));
	} else {
		hda_log(HDA_LOG_INFO, "Available options: power_save\n");
	}
	return;
}

static void set_jack(char *line)
{
	char *p;
	int nid, val;

	p = gettoken(&line);
	if (!p) {
		usage("jack");
		return;
	}
	nid = strtoul(p, NULL, 0);
	p = gettoken(&line);
	if (!p) {
		hda_log_jack_state(nid);
		return;
	}
	if (*p == '1' || !strcmp(p, "on"))
		val = 1;
	else
		val = 0;
	hda_log_set_jack(nid, val);
}

static void issue_unsol(char *line)
{
	char *p;
	int nid;

	p = gettoken(&line);
	if (!p) {
		usage("unsol");
		return;
	}
	nid = strtoul(p, NULL, 0);
	hda_log_issue_unsol(nid);
}

static void run_verb(char *line)
{
	char *parm[3];
	unsigned int verb, val;
	int i;

	for (i = 0; i < 3; i++) {
		parm[i] = gettoken(&line);
		if (!parm[i]) {
			usage("verb");
			return;
		}
	}
	if (hda_encode_verb_parm(parm[1], parm[2], &verb, &val) < 0) {
		hda_log(HDA_LOG_ERR, "Invalid verb/parameter %s/%s\n",
			parm[1], parm[2]);
		return;
	}
	hda_exec_verb(strtoul(parm[0], NULL, 0), verb, val);
}

static int get_pcm_substream(char *id)
{
	id = strchr(id, ':');
	if (!id)
		return 0;
	return strtoul(id + 1, NULL, 0);
}

static void test_pm(char *line)
{
	hda_log(HDA_LOG_INFO, "** SUSPENDING **\n");
	hda_test_suspend();
	hda_log(HDA_LOG_INFO, "** RESUMING **\n");
	hda_test_resume();
	hda_log(HDA_LOG_INFO, "** TEST_PM DONE **\n");
}

static void test_pcm(char *line)
{
	char *id;
	int stream, dir, rate = 48000, channels = 2, format = 16;
	int substream = 0;

	id = gettoken(&line);
	if (!id) {
		hda_list_pcms();
		return;
	}
	stream = strtoul(id, NULL, 0);

	id = gettoken(&line);
	if (!id) {
		hda_log(HDA_LOG_ERR, "No stream direction is given\n");
		return;
	}
	switch (*id) {
	case 'p':
	case 'P':
		dir = 0;
		substream = get_pcm_substream(id);
		break;
	case 'c':
	case 'C':
		dir = 1;
		substream = get_pcm_substream(id);
		break;
	default:
		dir = strtoul(id, NULL, 0);
		if (dir != 0 || dir != 1) {
			hda_log(HDA_LOG_ERR, "Invalid direction %s\n", id);
			return;
		}
		break;
	}

	id = gettoken(&line);
	if (id) {
		rate = strtoul(id, NULL, 0);
		id = gettoken(&line);
		if (id) {
			channels = strtoul(id, NULL, 0);
			id = gettoken(&line);
			if (id)
				format = strtoul(id, NULL, 0);
		}
	}
	hda_test_pcm(stream, substream, dir, rate, channels, format);
}

#ifdef CONFIG_SND_HDA_RECONFIG
struct hda_codec;

const char *snd_hda_get_hint(struct hda_codec *codec, const char *key)
{
	return NULL;
}

int snd_hda_get_bool_hint(struct hda_codec *codec, const char *key)
{
	return -ENOENT;
}

#ifdef HAVE_USER_PINCFGS
static void get_drv_pincfgs(void)
{
	hda_log_show_driver_pin_configs();
}

static void get_init_pincfgs(void)
{
	hda_log_show_init_pin_configs();
}

static void get_user_pincfgs(void)
{
	hda_log_show_user_pin_configs();
}

static void set_user_pincfgs(char *line)
{
	char *token;
	unsigned int nid, val;
	token = gettoken(&line);
	if (!token)
		goto error;
	nid = strtoul(token, NULL, 0);
	token = gettoken(&line);
	if (!val)
		goto error;
	val = strtoul(token, NULL, 0);
	hda_log_set_user_pin_configs(nid, val);
	return;
 error:
	hda_log(HDA_LOG_ERR, "Specify NID and PINCFG values\n");
	return;
}
#endif /* HAVE_USER_PINCFGS */

static void get_not_yet(void)
{
	hda_log(HDA_LOG_ERR, "Not implemented yet\n");
}

static void set_not_yet(char *line)
{
	hda_log(HDA_LOG_ERR, "Not implemented yet\n");
}

static void clear_codec(char *line)
{
	hda_codec_reset();
	/* clear internal list */
}

static void reconfig_codec(char *line)
{
	hda_codec_reconfig();
}

#define get_hints		get_not_yet
#define set_hints		set_not_yet
#define get_vendor_id		get_not_yet
#define set_vendor_id		set_not_yet
#define get_subsystem_id	get_not_yet
#define set_subsystem_id	set_not_yet
#define get_revision_id		get_not_yet
#define set_revision_id		set_not_yet

struct sysfs_entry {
	const char *name;
	void (*get)(void);
	void (*set)(char *line);
};

static struct sysfs_entry sysfs_entries[] = {
#ifdef HAVE_USER_PINCFGS
	{ "driver_pin_configs", get_drv_pincfgs, NULL },
	{ "init_pin_configs", get_init_pincfgs, NULL },
	{ "user_pin_configs", get_user_pincfgs, set_user_pincfgs },
#endif
	{ "hints", get_hints, set_hints },
	{ "vendor_id", get_vendor_id, set_vendor_id },
	{ "subsystem_id", get_subsystem_id, set_subsystem_id },
	{ "revision_id", get_revision_id, set_revision_id },
	{ "clear", NULL, clear_codec },
	{ "reconfig", NULL, reconfig_codec },
	{ },
};

static struct sysfs_entry *find_sysfs_entry(char *file)
{
	struct sysfs_entry *s;
	for (s = sysfs_entries; s->name; s++)
		if (!strcmp(s->name, file))
			return s;
	hda_log(HDA_LOG_INFO, "No such a file: %s\n", file);
	return NULL;
}

static void get_sysfs(char *file)
{
	struct sysfs_entry *s;
	s = find_sysfs_entry(file);
	if (!s)
		return;
	if (!s->get) {
		hda_log(HDA_LOG_INFO, "%s has no read premission\n", file);
		return;
	}
	s->get();
}

static void set_sysfs(char *file, char *line)
{
	struct sysfs_entry *s;
	s = find_sysfs_entry(file);
	if (!s)
		return;
	if (!s->set) {
		hda_log(HDA_LOG_INFO, "%s has no write permission\n", file);
		return;
	}
	s->set(line);
}

static void list_sysfs(void)
{
	struct sysfs_entry *s;
	hda_log(HDA_LOG_INFO, "Available sysfs entries:\n");
	for (s = sysfs_entries; s->name; s++)
		hda_log(HDA_LOG_INFO, "  %s (%s%c)\n",
			s->name, (s->get ? "R" : ""), (s->set ? "W" : ""));
}

static void handle_sysfs(char *line)
{
	char *cmd;

	cmd = gettoken(&line);
	if (!cmd)
		goto error;
	if (*cmd == 'l')
		list_sysfs();
	else if (*cmd == 'g' || *cmd == 's') {
		char *file;
		file = gettoken(&line);
		if (!file)
			goto error;
		if (*cmd == 'g')
			get_sysfs(file);
		else
			set_sysfs(file, line);
	}
	return;
 error:
	usage("fs");
	return;
}
#endif

/*
 */

int cmd_loop(FILE *fp)
{
	char line[256];
	char *buf, *p;

	for (;;) {
		fprintf(stderr, "> ");
		buf = fgets(line, sizeof(line), fp);
		if (!buf)
			break;
		hda_log_echo(HDA_LOG_INFO, "> %s", buf);
		p = gettoken(&buf);
		if (!p) {
			usage(NULL);
			continue;
		}
		switch (*p) {
		case 'l':
			show_elements(buf);
			break;
		case 'g':
			get_element(buf);
			break;
		case 's':
			set_element(buf);
			break;
		case 'd':
			dump_proc(buf);
			break;
		case 'o':
			handle_module_option(buf);
			break;
		case 'j':
			set_jack(buf);
			break;
		case 'u':
			issue_unsol(buf);
			break;
		case 'v':
			run_verb(buf);
			break;
		case 'P':
			test_pcm(buf);
			break;
		case 'p':
			test_pm(buf);
			break;
#ifdef CONFIG_SND_HDA_RECONFIG
		case 'f':
			handle_sysfs(buf);
			break;
#endif
		case 'h':
			usage(gettoken(&buf));
			break;
		case 'q':
			return 0;
		default:
			usage(NULL);
			break;
		}
		flush_scheduled_work();
	}
	return 0;
}
