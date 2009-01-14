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
	int err;

	hda_log(HDA_LOG_VERB,
		"send: NID=0x%x, VERB=0x%x(%s), PARM=0x%x",
		nid, verb, get_verb_name(&proc, cmd), parm);
	if (verb == 0xf00)
		hda_log(HDA_LOG_VERB, "(%s)",
			get_parameter_name(&proc, cmd));
	hda_log(HDA_LOG_VERB, "\n");

	err = hda_cmd(&proc, cmd);
	if (err < 0) {
		hda_log(HDA_LOG_VERB, "invalid command: "
			"NID=0x%x, verb=0x%x, parm=0x%x\n",
			nid, verb, parm);
		return err;
	}
	return 0;
}

static unsigned int resp_get(struct hda_bus *bus)
{
	hda_log(HDA_LOG_VERB, "receive: 0x%x\n", proc.rc);
	return proc.rc;
}

#ifdef OLD_HDA_CMD
static int old_cmd_send(struct hda_codec *codec, hda_nid_t nid,
			int direct, unsigned int verb,
			unsigned int para)
{
	u32 val;

	val = (u32)(codec->addr & 0x0f) << 28;
	val |= (u32)direct << 27;
	val |= (u32)nid << 20;
	val |= verb << 8;
	val |= para;

	return cmd_send(codec->bus,  val);
}

static unsigned int old_resp_get(struct hda_codec *codec)
{
	return resp_get(codec->bus);
}
#endif

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
 * power_save module option handling
 */
#ifdef HDA_OLD_POWER_SAVE
extern int *power_save_parameter; /* defined in kerenl/hda_codec.c */
#else
static int power_save;
#endif

int hda_get_power_save(void)
{
#ifdef HDA_OLD_POWER_SAVE
	return *power_save_parameter;
#else
	return power_save;
#endif
}

void hda_set_power_save(int val)
{
#ifdef HDA_OLD_POWER_SAVE
	*power_save_parameter = val;
#else
	power_save = val;
#endif
}

/*
 */

void hda_log_dump_proc(const char *file)
{
	struct snd_info_buffer buf;
	FILE *fp;
	int saved_level = log_level;
	if (!card.proc)
		return;

	if (file) {
		fp = fopen(file, "w");
		if (!fp) {
			hda_log(HDA_LOG_ERR,
				"Cannot open dump file %s\n", file);
			return;
		}
		buf.fp = fp;
	} else
		buf.fp = logfp;
	/* don't show verbs */
	log_level = HDA_LOG_INFO;
	card.proc->func(card.proc, &buf);
	log_level = saved_level;
	if (file)
		fclose(fp);
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

#ifdef CONFIG_SND_HDA_RECONFIG

static void reset_pcm(void);

void hda_codec_reset(void)
{
	snd_hda_codec_reset(_codec);
	reset_pcm();
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

#endif /* CONFIG_SND_HDA_RECONFIG */

/*
 * PCM
 */

#define MAX_PCM_STREAMS		16

static int num_pcm_streams;
static struct hda_pcm pcm_streams[MAX_PCM_STREAMS];

/* get a string corresponding to the given HDA_PCM_TYPE_XXX */
static const char *get_pcm_type_name(int type)
{
	const char *names[] = {
		[HDA_PCM_TYPE_AUDIO] = "audio",
		[HDA_PCM_TYPE_SPDIF] = "SPDIF",
		[HDA_PCM_TYPE_HDMI] = "HDMI",
		[HDA_PCM_TYPE_MODEM] = "modem",
	};
	if (type >= HDA_PCM_TYPE_AUDIO && type <= HDA_PCM_TYPE_MODEM)
		return names[type];
	else
		return "unknown";
}

/* list registered PCM streams, called from hda-ctlsh.c */
void hda_list_pcms(void)
{
	int i;

	for (i = 0; i < num_pcm_streams; i++) {
		struct hda_pcm *p = &pcm_streams[i];
		hda_log(HDA_LOG_INFO, "%d: %s:%d (%s), play=%d, capt=%d\n",
			i, p->name, p->device,
			get_pcm_type_name(p->pcm_type),
			p->stream[0].substreams,
			p->stream[1].substreams);
	}
}

/* get the appropriate ALSA SNDRV_PCM_FORMAT_* from the format bits */
static int get_alsa_format(int bits)
{
	if (bits <= 8)
		return SNDRV_PCM_FORMAT_U8;
	else if (bits <= 16)
		return SNDRV_PCM_FORMAT_S16_LE;
	else
		return SNDRV_PCM_FORMAT_S32_LE;
}

/* test the given PCM stream, called from hda-ctlsh.c */
void hda_test_pcm(int id, int dir, int rate, int channels, int format)
{
	static struct snd_pcm_substream dummy_substream;
	static struct snd_pcm_runtime dummy_runtime;
	struct snd_pcm_substream *substream = &dummy_substream;
	struct snd_pcm_runtime *runtime = &dummy_runtime;
	struct hda_pcm_stream *hinfo;
	unsigned int format_val;
	int err;

	if (id < 0 || id >= num_pcm_streams) {
		hda_log(HDA_LOG_ERR, "Invalid PCM id %d\n", id);
		return;
	}

	memset(substream, 0, sizeof(*substream));
	memset(runtime, 0, sizeof(*runtime));
	substream->runtime = runtime;
	runtime->rate = rate;
	runtime->format = get_alsa_format(format);
	runtime->channels = channels;

	hinfo = &pcm_streams[id].stream[dir];
	hda_log(HDA_LOG_INFO, "Open PCM %s for %s\n",
		pcm_streams[id].name,
		(dir ? "capt" : "play"));
	snd_hda_power_up(_codec);
	err = hinfo->ops.open(hinfo, _codec, substream);
	if (err < 0) {
		hda_log(HDA_LOG_INFO, "Open error = %d\n", err);
		snd_hda_power_down(_codec);
		return;
	}
	
	hda_log(HDA_LOG_INFO, "Prepare PCM, rate=%d, channels=%d, "
		"format=%d bits\n",
		rate, channels, format);
	format_val = snd_hda_calc_stream_format(rate, channels,
						get_alsa_format(format),
						format);
	if (!format_val) {
		snd_hda_power_down(_codec);
		return;
	}
	hda_log(HDA_LOG_INFO, "PCM format_val = 0x%x\n", format_val);
	err = hinfo->ops.prepare(hinfo, _codec, 0, format_val, substream);

	hda_log(HDA_LOG_INFO, "PCM Clean up\n");
	hinfo->ops.cleanup(hinfo, _codec, substream);

	hda_log(HDA_LOG_INFO, "Close PCM\n");
	hinfo->ops.close(hinfo, _codec, substream);
	snd_hda_power_down(_codec);
}

/* attach_pcm callback -- register the stream */
static int attach_pcm(struct hda_bus *bus, struct hda_codec *codec,
		      struct hda_pcm *cpcm)
{
	hda_log(HDA_LOG_INFO,
		"Attach PCM dev %d, name %s, play #%d, capture #%d\n",
		cpcm->device, cpcm->name,
		cpcm->stream[SNDRV_PCM_STREAM_PLAYBACK].substreams,
		cpcm->stream[SNDRV_PCM_STREAM_CAPTURE].substreams);
	if (num_pcm_streams >= MAX_PCM_STREAMS) {
		hda_log(HDA_LOG_ERR, "Too many streams\n");
		return 0;
	}
	pcm_streams[num_pcm_streams++] = *cpcm;
	return 0;
}

#ifdef CONFIG_SND_HDA_RECONFIG
/* clear the all registered PCM streams */
static void reset_pcm(void)
{
	memset(pcm_streams, 0, sizeof(pcm_streams));
	num_pcm_streams = 0;
}
#endif

/*
 * power management
 */

static void pm_notify(struct hda_bus *bus)
{
	hda_log(HDA_LOG_INFO, "PM-Notified\n");
}

#ifdef OLD_HDA_CMD
static void old_pm_notify(struct hda_codec *codec)
{
	pm_notify(codec->bus);
}
#endif


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

#include "kernel/init_hooks.h"

int main(int argc, char **argv)
{
	int c, err;
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
#ifndef OLD_POWER_SAVE
	temp.power_save = &power_save;
#endif
#ifdef OLD_HDA_CMD
	temp.ops.command = old_cmd_send;
	temp.ops.get_response = old_resp_get;
	temp.ops.pm_notify = old_pm_notify;
#else
	temp.ops.command = cmd_send;
	temp.ops.get_response = resp_get;
	temp.ops.attach_pcm = attach_pcm;
	temp.ops.pm_notify = pm_notify;
#endif
	gather_codec_hooks();

	if (snd_hda_bus_new(&card, &temp, &bus) < 0) {
		hda_log(HDA_LOG_ERR, "cannot create snd_hda_bus\n");
		return 1;
	}

#ifdef OLD_HDA_CODEC_NEW
	err = snd_hda_codec_new(bus, proc.addr, &codec);
#else
	err = snd_hda_codec_new(bus, proc.addr, 1, &codec);
#endif
	if (err) {
		hda_log(HDA_LOG_ERR, "cannot create codec\n");
		return 1;
	}
	_codec = codec;

	hda_log(HDA_LOG_INFO, "# Init and building controls...\n");
#ifdef CONFIG_SND_HDA_RECONFIG
	snd_hda_codec_build_controls(codec);
#else
	snd_hda_build_controls(codec->bus);
#endif

	hda_log(HDA_LOG_INFO, "# Building PCMs...\n");
	snd_hda_build_pcms(bus);

	/* power-down after init phase */
	snd_hda_power_down(codec);
	
	cmd_loop(stdin);

	return 0;
}
