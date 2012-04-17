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
#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "hda-types.h"
#include "hda-log.h"

#include <sound/core.h>
#include <sound/control.h>
#include <sound/tlv.h>

static void usage(char *);

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

static int getint(char **bufp, int *val)
{
	char *token = gettoken(bufp);
	if (!token)
		return -ENOENT;
	*val = strtoul(token, NULL, 0);
	return 0;
}

static int getbool(char **bufp, int *val)
{
	char *token = gettoken(bufp);
	if (!token)
		return -ENOENT;
	if (*token == '1' || !strcmp(token, "on"))
		*val = 1;
	else
		*val = 0;
	return 0;
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

static void db_string(int db, char *str, int len)
{
	const char *pfx;
	if (db < 0) {
		pfx = "-";
		db = -db;
	} else {
		pfx = "";
	}
	snprintf(str, len, "%s%d.%02d", pfx, db / 100, db % 100);
}

static void show_db_info(struct snd_kcontrol *kctl,
			 struct snd_ctl_elem_info *uinfo,
			 struct snd_ctl_elem_value *uval)
{
	unsigned int _tlv[64];
	const unsigned int *tlv;
	int i, err;
	int mindb, maxdb, step;
	char db1[8], db2[8];

	if (kctl->vd[0].access & SNDRV_CTL_ELEM_ACCESS_TLV_CALLBACK) {
		err = kctl->tlv.c(kctl, 0, sizeof(_tlv), _tlv);
		if (err < 0)
			return;
		tlv = _tlv;
	} else {
		tlv = kctl->tlv.p;
	}
	if (!tlv)
		return;

	if (*tlv != SNDRV_CTL_TLVT_DB_SCALE)
		return;
	mindb = tlv[2];
	step = tlv[3] & 0xffff;
	maxdb = mindb +
		(uinfo->value.integer.max - uinfo->value.integer.min) * step;
	db_string(mindb, db1, sizeof(db1));
	db_string(maxdb, db2, sizeof(db2));
	hda_log(HDA_LOG_INFO, "\ndB min/max: %sdB,%sdB ", db1, db2);

	for (i = 0; i < uinfo->count; i++) {
		int curv, curdb;
		curv = uval->value.integer.value[i];
		if (curv < uinfo->value.integer.min)
			curv = uinfo->value.integer.min;
		else if (curv > uinfo->value.integer.max)
			curv = uinfo->value.integer.max;
		curdb = mindb + step * (curv - uinfo->value.integer.min);
		db_string(curdb, db1, sizeof(db1));
		hda_log(HDA_LOG_INFO, " [%sdB]", db1);
	}
}

static void get_element(char *line)
{
	struct snd_kcontrol *kctl;
	struct snd_ctl_elem_info uinfo;
	struct snd_ctl_elem_value uval;
	int numid;
	int i, err;

	if (getint(&line, &numid)) {
		usage("get");
		return;
	}
	kctl = snd_ctl_find_numid(NULL, numid);
	if (!kctl) {
		hda_log(HDA_LOG_INFO, "No element %d\n", numid);
		return;
	}
	
	memset(&uinfo, 0, sizeof(uinfo));
	uinfo.id = kctl->id;
	uinfo.id.numid = numid;
	uinfo.id.index = snd_ctl_get_ioffnum(kctl, &uinfo.id);
	err = kctl->info(kctl, &uinfo);
	if (err < 0) {
		hda_log(HDA_LOG_INFO, "Error in info for %d\n", numid);
		return;
	}

	memset(&uval, 0, sizeof(uval));
	uval.id = uinfo.id;
	err = kctl->get(kctl, &uval);
	if (err < 0) {
		hda_log(HDA_LOG_INFO, "Error in get for %d\n", numid);
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
		if (uinfo.type == SNDRV_CTL_ELEM_TYPE_INTEGER &&
		    (kctl->vd[0].access & SNDRV_CTL_ELEM_ACCESS_TLV_READ))
			show_db_info(kctl, &uinfo, &uval);
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
	unsigned int numid;
	int i, err;

	if (getint(&line, &numid)) {
		usage("set");
		return;
	}
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
	uinfo.id.numid = numid;
	uinfo.id.index = snd_ctl_get_ioffnum(kctl, &uinfo.id);
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
	uval.id = uinfo.id;
	for (i = 0; i < uinfo.count; i++) {
		int val;
		if (getint(&line, &val)) {
			hda_log(HDA_LOG_INFO, "No value #%d is given\n", i);
			return;
		}
		switch (uinfo.type) {
		case SNDRV_CTL_ELEM_TYPE_INTEGER:
		case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
			uval.value.integer.value[i] = val;
			break;
		case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
			uval.value.enumerated.item[i] = val;
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
	char *file = NULL;

	if (!getint(&line, &nid))
		file = gettoken(&line);
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
	int nid, val;

	if (getint(&line, &nid)) {
		hda_log_list_jacks();
		return;
	}
	if (getbool(&line, &val)) {
		hda_log_jack_state(nid);
		return;
	}
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

static void show_routes(char *line)
{
	int numid;
	unsigned int flags = SHOW_DIR_IN | SHOW_DIR_OUT;
	char *token;

	for (;;) {
		token = gettoken(&line);
		if (!token) {
			usage("route");
			return;
		}
		if (*token == '-') {
			/* option */
			switch (token[1]) {
			case 'a':
				flags |= SHOW_ALL | SHOW_INACTIVE;
				break;
			case 'x':
				flags |= SHOW_INACTIVE;
				break;
			case 'i':
				flags &= ~SHOW_DIR_OUT;
				flags |= SHOW_DIR_IN;
				break;
			case 'o':
				flags &= ~SHOW_DIR_IN;
				flags |= SHOW_DIR_OUT;
				break;
			default:
				hda_log(HDA_LOG_ERR, "Invalid route option\n");
				usage("route");
				return;
			}
		} else
			break;
	}
	numid = strtoul(token, NULL, 0);
	hda_show_routes(numid, flags);
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
	int op = PCM_TEST_ALL;
	char *stream_token;
	char *token;

	token = gettoken(&line);
	if (!token) {
		hda_list_pcms();
		return;
	}
	if (*token == '-') {
		/* option */
		switch (token[1]) {
		case 's':
			op = PCM_TEST_START;
			break;
		case 'e':
			op = PCM_TEST_END;
			break;
		default:
			hda_log(HDA_LOG_ERR, "Invalid PCM option\n");
			usage("PCM");
			return;
		}
		token = gettoken(&line);
	}
	stream_token = token;
	if (!stream_token) {
		usage("PCM");
		return;
	}
	id = gettoken(&line);
	if (!id) {
		hda_log(HDA_LOG_ERR, "No stream direction is given\n");
		return;
	}
	switch (*id) {
	case 'p':
	case 'P':
		dir = 0;
		break;
	case 'c':
	case 'C':
		dir = 1;
		break;
	default:
		dir = strtoul(id, NULL, 0);
		if (dir != 0 || dir != 1) {
			hda_log(HDA_LOG_ERR, "Invalid direction %s\n", id);
			return;
		}
		break;
	}
	stream = strtoul(stream_token, NULL, 0);
	substream = get_pcm_substream(stream_token);

	if (!getint(&line, &rate) &&
	    !getint(&line, &channels))
		getint(&line, &format);
	hda_test_pcm(stream, op, substream, dir, rate, channels, format);
}

static void help(char *buf)
{
	usage(gettoken(&buf));
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
	unsigned int nid, val;
	if (getint(&line, &nid) || getint(&line, &val))
		goto error;
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
	struct sysfs_entry *matched = NULL;
	int num_matches = 0;

	for (s = sysfs_entries; s->name; s++) {
		if (!strcmp(s->name, file))
			return s;
		if (!strncmp(s->name, file, strlen(file))) {
			matched = s;
			num_matches++;
		}
	}
	if (num_matches == 1)
		return matched;
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
		hda_log(HDA_LOG_INFO, "  %s (%s%s)\n",
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

struct usage_table {
	const char *cmd;
	const char *line;
	const char *desc;
	void (*handler)(char *);
};
	
static struct usage_table usage_str[] = {
	{ "list", "list",
	  "Show all control elements",
	  show_elements },
	{ "get", "get numid",
	  "Get the contents of the given control element",
	  get_element },
	{ "set", "set numid val [val2]",
	  "Set the contents of the given control element",
	  set_element },
	{ "dump", "dump [nid [filename]]",
	  "Dump codec contents in the proc file format; nid = 0 means all widgets",
	  dump_proc },
	{ "jack", "jack numid [val]",
	  "Get jack state or set jack state; val = 0 or 1",
	  set_jack },
	{ "unsol", "unsol numid",
	  "Issue an unsolicited event",
	  issue_unsol },
	{ "route", "route [-opts] numid",
	  "Show routes via the given widget; -a = show all, -x = show inactive pins too, -i|-o = set direction",
	  show_routes },
	{ "option", "option variable [val]",
	  "Get/set module option value",
	  handle_module_option },
	{ "help", "help [command]",
	  "Show help texts",
	  help },
	{ "verb", "verb nid cmd parameter",
	  "Execute a verb",
	  run_verb },
	{ "PCM", "PCM [-s|-e] [pcm-id dir [rate [channels [format-bits]]]]",
	  "List PCM streams or test the given PCM stream",
	  test_pcm },
	{ "pm", "pm",
	  "Test suspend/resume cycle",
	  test_pm },
#ifdef CONFIG_SND_HDA_RECONFIG
	{ "fs", "fs {get|set|list} file args...",
	  "Read or write sysfs files",
	  handle_sysfs },
#endif
	{ "quit", "quit",
	  "Quit the program",
	  NULL },
	{ }
};

static void usage(char *cmd)
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

#ifdef HAVE_LIBREADLINE

/* Line completion functions */

static char *command_generator(const char *text, int state)
{
	static int index, len;
	const char *name;

	if (!state) {
		index = 0;
		len = strlen(text);
	}

	/* Return the next name which partially matches from command list. */
	while ((name = usage_str[index].cmd) != NULL) {
		index++;
		if (strncmp(name, text, len) == 0)
			return strdup(name);
	}

	return NULL;
}

static char **ctlsh_completion(const char *text, int start, int end)
{
	char **matches = NULL;

	if (start == 0)
		matches = rl_completion_matches(text, command_generator);

	return matches;
}

static void init_completion(void)
{
	rl_attempted_completion_function = ctlsh_completion;
}

#else

static FILE *rl_instream;
static FILE *rl_outstream;

static char *readline(const char *prompt)
{
	char line[256], *buf;
	fprintf(stderr, "%s", prompt);
	buf = fgets(line, sizeof(line), rl_instream);
	if (!buf)
		return NULL;
	return strdup(buf);
}

#define add_history(l)
#define init_completion()

#endif /* HAVE_LIBREADLINE */

static struct usage_table *cmd_match(char *p)
{
	struct usage_table *tbl, *match = NULL;
	int len, num_matches = 0;

	len = strlen(p);
	for (tbl = usage_str; tbl->cmd; tbl++) {
		if (strncmp(p, tbl->cmd, len) == 0) {
			num_matches++;
			match = tbl;
		}
	}

	if (num_matches == 1)
		return match;

	return NULL;
}

int cmd_loop(FILE *fp)
{
	char *line;
	char *buf, *p;
	struct usage_table *tbl;

	if (fp)
		rl_instream = fp;
	rl_outstream = stderr;

	init_completion();

	for (;;) {
		line = readline("> ");
		if (!line)
			break;
		if (line && *line)
			add_history(line);
		buf = line;
		hda_log_echo(HDA_LOG_INFO, "> %s", buf);
		p = gettoken(&buf);
		if (!p) {
			usage(NULL);
			free(line);
			continue;
		}

		tbl = cmd_match(p);
		if (tbl == NULL)
			usage(NULL);
		else if (!strcmp(tbl->cmd, "quit"))
			return 0;
		else
			tbl->handler(buf);

		flush_scheduled_work();
		free(line);
	}
	return 0;
}
