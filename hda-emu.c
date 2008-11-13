/*
 * hda-emu - simple HD-audio codec emulator for debugging snd-hda-intel driver
 *
 * Main routine
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
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>

#include "hda-types.h"
#include "hda-log.h"

#include <sound/core.h>
#include <sound/pcm.h>
#include "kernel/hda_codec.h"
#include "kernel/hda_local.h"

extern int cmd_loop(FILE *fp);

/*
 * logging
 */

static int log_level = HDA_LOG_VERB;
static FILE *logfp;
static int no_log_echo;

void hda_log(int level, const char *fmt, ...)
{
	va_list ap, ap2;

	if (level > log_level)
		return;

	va_start(ap, fmt);
	va_copy(ap2, ap);
	vfprintf(logfp, fmt, ap);
	if (!no_log_echo && logfp != stdout)
		vprintf(fmt, ap2);
	va_end(ap);
}

void hda_log_echo(int level, const char *fmt, ...)
{
	va_list ap;

	if (no_log_echo || logfp == stdout || level > log_level)
		return;
	va_start(ap, fmt);
	vfprintf(logfp, fmt, ap);
	va_end(ap);
}

int hda_log_init(const char *file)
{
	if (!file || !strcmp(file, "-") || !strcmp(file, "stdout"))
		logfp = stdout;
	else {
		logfp = fopen(file, "w");
		if (!logfp) {
			fprintf(stderr, "cannot open log file %s\n", file);
			exit(1);
		}
	}
	return 0;
}

/*
 */

static struct snd_card card;
static struct xhda_codec proc;
static struct pci_dev mypci;

static int cmd_send(struct hda_bus *bus, unsigned int cmd)
{
	unsigned int nid = (cmd >> 20) & 0x7f;
	unsigned int verb = (cmd >> 8) & 0xfff;
	unsigned int parm = cmd & 0xff;
	hda_log(HDA_LOG_VERB,
		"send: NID=0x%x, VERB=0x%x(%s), PARM=0x%x",
		nid, verb, get_verb_name(cmd), parm);
	if (verb == 0xf00)
		hda_log(HDA_LOG_VERB, "(%s)",
			get_parameter_name(cmd));
	hda_log(HDA_LOG_VERB, "\n");

	if (hda_cmd(&proc, cmd) < 0)
		hda_log(HDA_LOG_VERB, "invalid command: "
			"NID=0x%x, verb=0x%x, parm=0x%x\n",
			nid, verb, parm);
	return 0;
}

static unsigned int resp_get(struct hda_bus *bus)
{
	hda_log(HDA_LOG_VERB, "receive: 0x%x\n", proc.rc);
	return proc.rc;
}

void hda_exec_verb(int nid, int verb, int parm)
{
	u32 val;

	val = (u32)(proc.addr & 0x0f) << 28;
	val |= (u32)nid << 20;
	val |= verb << 8;
	val |= parm;

	if (hda_cmd(&proc, val) < 0) {
		hda_log(HDA_LOG_VERB, "invalid command: "
			"NID=0x%x, verb=0x%x, parm=0x%x\n",
			nid, verb, parm);
	} else {
		hda_log(HDA_LOG_VERB, "Command response:: 0x%x\n", proc.rc);
	}
}

/*
 */

void hda_log_dump_proc(void)
{
	struct snd_info_buffer buf;
	int saved_level = log_level;
	if (!card.proc)
		return;

	buf.fp = logfp;
	/* don't show verbs */
	log_level = HDA_LOG_INFO;
	card.proc->func(card.proc, &buf);
	log_level = saved_level;
}

void hda_log_jack_state(int nid)
{
	int state = hda_get_jack_state(&proc, nid);
	if (state < 0)
		hda_log(HDA_LOG_ERR, "Invalid Jack NID 0x%x\n", nid);
	else
		hda_log(HDA_LOG_INFO, "Jack state [0x%x] = %d\n", nid, state);
}

static void issue_unsol(int caddr, int res);

void hda_log_set_jack(int nid, int val)
{
	int err = hda_set_jack_state(&proc, nid, val);
	if (err > 0 && (err & (1 << 7)))
		issue_unsol(proc.addr, err);
}

/*
 */

static struct hda_bus *bus;

void hda_test_suspend(void)
{
	snd_hda_suspend(bus, PMSG_SUSPEND);
}

void hda_test_resume(void)
{
	snd_hda_resume(bus);
}

/*
 */

static struct hda_codec *_codec;

static void issue_unsol(int caddr, int res)
{
	caddr |= 1 << 4;
	/* ALC880 has incompatible unsol tag */
	if (_codec->vendor_id == 0x10ec0880)
		res = (res & 0x3f) << 28;
	else
		res = (res & 0x3f) << 26;

	snd_hda_queue_unsol_event(bus, res, caddr);
}

void hda_codec_reset(void)
{
	snd_hda_codec_reset(_codec);
}

int hda_codec_reconfig(void)
{
	int err;

	snd_hda_codec_reset(_codec);
	err = snd_hda_codec_configure(_codec);
	if (err < 0)
		return err;
	/* rebuild PCMs */
	err = snd_hda_build_pcms(bus);
	if (err < 0)
		return err;
	/* rebuild mixers */
	err = snd_hda_codec_build_controls(_codec);
	if (err < 0)
		return err;
	return 0;
}

/*
 */

static int attach_pcm(struct hda_bus *bus, struct hda_codec *codec,
		      struct hda_pcm *cpcm)
{
	hda_log(HDA_LOG_INFO,
		"Attach PCM dev %d, name %s, play #%d, capture #%d\n",
		cpcm->device, cpcm->name,
		cpcm->stream[SNDRV_PCM_STREAM_PLAYBACK].substreams,
		cpcm->stream[SNDRV_PCM_STREAM_CAPTURE].substreams);
	return 0;
}

static void pm_notify(struct hda_bus *bus)
{
	hda_log(HDA_LOG_INFO, "PM-Notified\n");
}


/*
 */
static void usage(void)
{
	fprintf(stderr, "usage: "
		"hda-emu [options] proc-file\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "  -l level       specifies log level\n");
	fprintf(stderr, "  -i index       specifies codec index to parse\n");
	fprintf(stderr, "  -p vendor:dev  specifies PCI subsystem ID\n");
	fprintf(stderr, "  -m model       specifies model option string\n");
	fprintf(stderr, "  -o file        log to the given file\n");
	fprintf(stderr, "  -q             don't echo but only to log file\n");
}

int main(int argc, char **argv)
{
	int c;
	FILE *fp;
	int idx = 0;
	int pci_subvendor = 0;
	int pci_subdevice = 0;
	char *opt_model = NULL;
	char *logfile = NULL;
	struct hda_bus_template temp;
	struct hda_codec *codec;

	while ((c = getopt(argc, argv, "l:i:p:m:do:q")) != -1) {
		switch (c) {
		case 'l':
			log_level = atoi(optarg);
			break;
		case 'i':
			idx = atoi(optarg);
			break;
		case 'p':
			if (sscanf(optarg, "%x:%x",
				   &pci_subvendor, &pci_subdevice) != 2) {
				fprintf(stderr, "invalid arg %s\n", optarg);
				return 1;
			}
			break;
		case 'm':
			opt_model = optarg;
			break;
		case 'o':
			logfile = optarg;
			break;
		case 'q':
			no_log_echo = 1;
			break;
		default:
			usage();
			return 1;
		}
	}

	if (optind >= argc) {
		usage();
		return 1;
	}

	fp = fopen(argv[optind], "r");
	if (!fp) {
		fprintf(stderr, "cannot open %s\n", argv[1]);
		return 1;
	}

	hda_log_init(logfile);

	hda_log(HDA_LOG_INFO, "# Parsing..\n");
	if (parse_codec_proc(fp, &proc, idx) < 0) {
		hda_log(HDA_LOG_INFO, "error at reading proc\n");
		return 1;
	}

	mypci.subsystem_vendor = pci_subvendor;
	mypci.subsystem_device = pci_subdevice;

	memset(&temp, 0, sizeof(temp));

	temp.pci = &mypci;
	temp.modelname = opt_model;
	temp.ops.command = cmd_send;
	temp.ops.get_response = resp_get;
	temp.ops.attach_pcm = attach_pcm;
	temp.ops.pm_notify = pm_notify;

	if (snd_hda_bus_new(&card, &temp, &bus) < 0) {
		hda_log(HDA_LOG_ERR, "cannot create snd_hda_bus\n");
		return 1;
	}

	if (snd_hda_codec_new(bus, proc.addr, &codec)) {
		hda_log(HDA_LOG_ERR, "cannot create codec\n");
		return 1;
	}
	_codec = codec;
	hda_log(HDA_LOG_INFO, "# Initializing..\n");
	if (codec->patch_ops.init) {
		if (codec->patch_ops.init(codec) < 0) {
			hda_log(HDA_LOG_ERR, "codec init error\n");
			return 1;
		}
	}
	hda_log(HDA_LOG_INFO, "# Building controls...\n");
	if (codec->patch_ops.build_controls){
		if (codec->patch_ops.build_controls(codec) < 0) {
			hda_log(HDA_LOG_ERR, "codec build_controls error\n");
			return 1;
		}
	}
	hda_log(HDA_LOG_INFO, "# Building PCMs...\n");
	if (codec->patch_ops.build_pcms) {
		if (codec->patch_ops.build_pcms(codec) < 0) {
			hda_log(HDA_LOG_ERR, "cannot create PCMs\n");
			return 1;
		}
	}

	/* power-down after init phase */
	snd_hda_power_down(codec);
	
	cmd_loop(stdin);

	return 0;
}
