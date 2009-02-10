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

#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include "kernel/hda_codec.h"
#include "kernel/hda_local.h"

#ifndef HAVE_POWER_SAVE
#define snd_hda_power_up(x)
#define snd_hda_power_down(x)
#endif

extern int cmd_loop(FILE *fp);

/*
 * proc dump log
 */

static struct snd_card card;
static struct xhda_codec proc;

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
		hda_log(HDA_LOG_ERR, "invalid command: "
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
		hda_log(HDA_LOG_ERR, "invalid command: "
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

struct snd_info_buffer {
	FILE *fp;
	unsigned int nid;
	int printing;
	int processing;
};

extern void (*snd_iprintf_dumper)(struct snd_info_buffer *buf,
				  const char *fmt, va_list ap);

static void log_dump_proc_file(struct snd_info_buffer *buf,
			       const char *fmt, va_list ap)
{
	char line[512];
	int node, in_process;
	char *p;

	if (!buf->nid) {
		vfprintf(buf->fp, fmt, ap);
		return;
	}
	vsnprintf(line, sizeof(line), fmt, ap);
	if (buf->processing) {
		if (buf->printing == 1)
			fputs(line, buf->fp);
		p = strchr(line, '\n');
		if (p && !p[1])
			buf->processing = 0;
		return;
	}
	in_process = buf->processing;
	p = strchr(line, '\n');
	if (p && !p[1])
		buf->processing = 0;
	else
		buf->processing = 1;
	if (in_process) {
		if (buf->printing == 1)
			fputs(line, buf->fp);
		return;
	}
	switch (buf->printing) {
	case 0:
		if (sscanf(line, "Node 0x%02x ", &node) == 1 &&
		    node == buf->nid) {
			fputs(line, buf->fp);
			buf->printing = 1;
		}
		break;
	case 1:
		if (*line == ' ')
			fputs(line, buf->fp);
		else
			buf->printing = 2;
		break;
	}
}

void hda_log_dump_proc(unsigned int nid, const char *file)
{
	struct snd_info_buffer buf;
	FILE *fp;
	int saved_level;

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
		buf.fp = hda_get_logfp();
	buf.nid = nid;
	buf.printing = 0;
	/* don't show verbs */
	saved_level = hda_log_level_set(HDA_LOG_KERN);
	snd_iprintf_dumper = log_dump_proc_file;
	card.proc->func(card.proc, &buf);
	hda_log_level_set(saved_level);
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
	if (err > 0)
		hda_log_issue_unsol(nid);
}

void hda_log_issue_unsol(int nid)
{
	int val = hda_get_unsol_state(&proc, nid);
	if (val & (1 << 7))
		issue_unsol(proc.addr, val);
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

#ifndef OLD_HDA_PCM
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
#endif

/* list registered PCM streams, called from hda-ctlsh.c */
void hda_list_pcms(void)
{
	int i;

	for (i = 0; i < num_pcm_streams; i++) {
		struct hda_pcm *p = &pcm_streams[i];
#ifdef OLD_HDA_PCM
		hda_log(HDA_LOG_INFO, "%d: %s, play=%d, capt=%d\n",
			i, p->name,
			p->stream[0].substreams,
			p->stream[1].substreams);
#else
		hda_log(HDA_LOG_INFO, "%d: %s:%d (%s), play=%d, capt=%d\n",
			i, p->name, p->device,
			get_pcm_type_name(p->pcm_type),
			p->stream[0].substreams,
			p->stream[1].substreams);
#endif
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
#ifdef OLD_HDA_PCM
	hda_log(HDA_LOG_INFO,
		"Attach PCM name %s, play #%d, capture #%d\n",
		cpcm->name,
		cpcm->stream[SNDRV_PCM_STREAM_PLAYBACK].substreams,
		cpcm->stream[SNDRV_PCM_STREAM_CAPTURE].substreams);
#else
	hda_log(HDA_LOG_INFO,
		"Attach PCM dev %d, name %s, type %s, play #%d, capture #%d\n",
		cpcm->device, cpcm->name, get_pcm_type_name(cpcm->pcm_type),
		cpcm->stream[SNDRV_PCM_STREAM_PLAYBACK].substreams,
		cpcm->stream[SNDRV_PCM_STREAM_CAPTURE].substreams);
#endif
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
	fprintf(stderr, "  -C             print messages in color (default)\n");
	fprintf(stderr, "  -M             no color print\n");
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
	unsigned int log_flags = HDA_LOG_FLAG_COLOR;
	struct pci_dev mypci;
	struct hda_bus_template temp;
	struct hda_codec *codec;

	while ((c = getopt(argc, argv, "l:i:p:m:do:qCM")) != -1) {
		switch (c) {
		case 'l':
			hda_log_level_set(atoi(optarg));
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
			log_flags |= HDA_LOG_FLAG_NO_ECHO;
			break;
		case 'C':
			log_flags |= HDA_LOG_FLAG_COLOR;
			break;
		case 'M':
			log_flags &= ~HDA_LOG_FLAG_COLOR;
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

	hda_log_init(logfile, log_flags);

	hda_log(HDA_LOG_INFO, "# Parsing..\n");
	if (parse_codec_proc(fp, &proc, idx) < 0) {
		hda_log(HDA_LOG_INFO, "error at reading proc\n");
		return 1;
	}

	mypci.vendor = proc.pci_vendor;
	mypci.device = proc.pci_device;
	mypci.subsystem_vendor = proc.pci_subvendor;
	mypci.subsystem_device = proc.pci_subdevice;
	mypci.revision = proc.pci_revision;
	/* override PCI SSID */
	if (pci_subvendor || pci_subdevice) {
		mypci.subsystem_vendor = pci_subvendor;
		mypci.subsystem_device = pci_subdevice;
		hda_log(HDA_LOG_INFO, "Using PCI SSID %04x:%04x\n",
			pci_subvendor, pci_subdevice);
	}

	memset(&temp, 0, sizeof(temp));

	temp.pci = &mypci;
	temp.modelname = opt_model;
	if (opt_model)
		hda_log(HDA_LOG_INFO, "Using model option '%s'\n", opt_model);
#ifdef HAVE_POWER_SAVE
#ifndef OLD_POWER_SAVE
	temp.power_save = &power_save;
#endif
#endif
#ifdef OLD_HDA_CMD
	temp.ops.command = old_cmd_send;
	temp.ops.get_response = old_resp_get;
#ifdef HAVE_POWER_SAVE
	temp.ops.pm_notify = old_pm_notify;
#endif
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
