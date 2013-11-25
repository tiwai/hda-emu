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
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include "hda-types.h"
#include "hda-log.h"

#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include "hda/hda_codec.h"
#include "hda/hda_local.h"
#ifdef HAVE_HDA_BEEP
#include "hda/hda_beep.h"
#endif

#ifndef HAVE_POWER_SAVE
#define snd_hda_power_up(x)
#define snd_hda_power_down(x)
#endif

extern int cmd_loop(FILE *fp);

/*
 * interface to kernel 
 */

static struct snd_card card = {
	.ctl_files = LIST_HEAD_INIT(card.ctl_files),
	.files_lock = MYLOCK_UNLOCKED,
};
static struct xhda_codec proc;

static struct hda_codec *_codec;

static int ignore_invalid_ftype;

static int cmd_send(struct hda_bus *bus, unsigned int cmd)
{
	unsigned int nid = (cmd >> 20) & 0x7f;
	unsigned int verb = (cmd >> 8) & 0xfff;
	unsigned int parm = cmd & 0xff;
	int err;

	hda_log(HDA_LOG_VERB, "send: NID=0x%x, VERB=0x%x", nid, verb);
	switch (verb & 0xf00) {
	case AC_VERB_GET_AMP_GAIN_MUTE:
		hda_log(HDA_LOG_VERB,
			"(%s,%s:%s#%d), PARM=0x%x",
			get_verb_name(&proc, cmd),
			(verb & (1 << 7) ? "O" : "I"),
			(verb & (1 << 5) ? "L" : "R"),
			verb & 0x0f,
			parm);
		break;
	case AC_VERB_SET_AMP_GAIN_MUTE:
		hda_log(HDA_LOG_VERB,
			"(%s,%s%s:%s%s#%d), PARM=0x%x",
			get_verb_name(&proc, cmd),
			(verb & (1 << 7) ? "O" : ""),
			(verb & (1 << 6) ? "I" : ""),
			(verb & (1 << 5) ? "L" : ""),
			(verb & (1 << 4) ? "R" : ""),
			verb & 0x0f,
			parm);
		break;
	case AC_VERB_SET_PROC_COEF:
	case AC_VERB_SET_COEF_INDEX:
		hda_log(HDA_LOG_VERB,
			"(%s), PARM=0x%x",
			get_verb_name(&proc, cmd), (verb & 0xff) << 8 | parm);
		break;
	default:
		hda_log(HDA_LOG_VERB,
			"(%s), PARM=0x%x",
			get_verb_name(&proc, cmd), parm);
		if (verb == 0xf00)
			hda_log(HDA_LOG_VERB, "(%s)",
				get_parameter_name(&proc, cmd));
		break;
	}
	hda_log(HDA_LOG_VERB, "\n");

	err = hda_cmd(&proc, cmd);
	if (err < 0) {
		if (verb != AC_VERB_PARAMETERS || parm != AC_PAR_FUNCTION_TYPE ||
		    !ignore_invalid_ftype)
			hda_log(HDA_LOG_ERR, "invalid command: "
				"NID=0x%x, verb=0x%x, parm=0x%x\n",
				nid, verb, parm);
		return err;
	}
	return 0;
}

#ifdef HAVE_GET_RESPONSE_WITH_CADDR
static unsigned int resp_get_caddr(struct hda_bus *bus, unsigned int caddr)
{
	hda_log(HDA_LOG_VERB, "receive: 0x%x\n", proc.rc);
	return proc.rc;
}
#else
static unsigned int resp_get(struct hda_bus *bus)
{
	hda_log(HDA_LOG_VERB, "receive: 0x%x\n", proc.rc);
	return proc.rc;
}
#endif

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
extern int *power_save_parameter; /* defined in kernel/hda_codec.c */
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
 * proc dump
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

	if (buf->nid <= 0) {
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
		if (!strncmp(line, "Node 0x", 7))
			buf->printing = 2;
		else
			fputs(line, buf->fp);
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
	if (nid > 1)
		buf.printing = 0;
	else
		buf.printing = 1;
	snd_hda_power_up(_codec);
	/* don't show verbs */
	saved_level = hda_log_level_set(HDA_LOG_KERN);
	snd_iprintf_dumper = log_dump_proc_file;
	card.proc->func(card.proc, &buf);
	hda_log_level_set(saved_level);
	snd_hda_power_down(_codec);
	if (file)
		fclose(buf.fp);
}

/*
 * show available jacks
 */
static const char *_get_jack_location(u32 cfg)
{
	static char *bases[7] = {
		"N/A", "Rear", "Front", "Left", "Right", "Top", "Bottom",
	};
	static unsigned char specials_idx[] = {
		0x07, 0x08,
		0x17, 0x18, 0x19,
		0x37, 0x38
	};
	static char *specials[] = {
		"Rear Panel", "Drive Bar",
		"Riser", "HDMI", "ATAPI",
		"Mobile-In", "Mobile-Out"
	};
	int i;
	cfg = (cfg & AC_DEFCFG_LOCATION) >> AC_DEFCFG_LOCATION_SHIFT;
	if ((cfg & 0x0f) < 7)
		return bases[cfg & 0x0f];
	for (i = 0; i < ARRAY_SIZE(specials_idx); i++) {
		if (cfg == specials_idx[i])
			return specials[i];
	}
	return "UNKNOWN";
}

void hda_log_list_jacks(int raw)
{
	struct xhda_node *node;
	unsigned int type, conn;
	static char *jack_conns[4] = { "Jack", "N/A", "Fixed", "Both" };
	static char *jack_types[16] = {
		"Line Out", "Speaker", "HP Out", "CD",
		"SPDIF Out", "Digital Out", "Modem Line", "Modem Hand",
		"Line In", "Aux", "Mic", "Telephony",
		"SPDIF In", "Digitial In", "Reserved", "Other"
	};
	static char *jack_locations[4] = { "Ext", "Int", "Sep", "Oth" };

	for (node = proc.afg.next; node; node = node->next) {
		unsigned int pin_default;
		type = (node->wcaps & AC_WCAP_TYPE) >> AC_WCAP_TYPE_SHIFT;
		if (type != AC_WID_PIN)
			continue;
		pin_default = node->pin_default;
#ifdef HAVE_SND_HDA_CODEC_GET_PINCFG
		if (!raw)
			pin_default = snd_hda_codec_get_pincfg(_codec, node->nid);
#endif
		conn = (pin_default & AC_DEFCFG_PORT_CONN) >> AC_DEFCFG_PORT_CONN_SHIFT;
		if (conn == AC_JACK_PORT_NONE)
			continue;
		/* if (!(node->pincap & AC_PINCAP_PRES_DETECT))
		   continue;*/
		hda_log(HDA_LOG_INFO,
			"NID 0x%02x: cfg 0x%08x: [%s] %s at %s %s\n",
			node->nid, pin_default,
			jack_conns[(pin_default & AC_DEFCFG_PORT_CONN)
				   >> AC_DEFCFG_PORT_CONN_SHIFT],
			jack_types[(pin_default & AC_DEFCFG_DEVICE)
				   >> AC_DEFCFG_DEVICE_SHIFT],
			jack_locations[(pin_default >> (AC_DEFCFG_LOCATION_SHIFT + 4)) & 3],
			_get_jack_location(pin_default));
	}
}

/*
 * show jack state of the given NID
 */
void hda_log_jack_state(int nid)
{
	int state = hda_get_jack_state(&proc, nid);
	if (state < 0)
		hda_log(HDA_LOG_ERR, "Invalid Jack NID 0x%x\n", nid);
	else
		hda_log(HDA_LOG_INFO, "Jack state [0x%x] = %d\n", nid, state);
}

/*
 * change the jack state of the given NID
 */
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
 * suspend/resume simulation
 */

static struct hda_bus *bus;

void hda_test_suspend(void)
{
#ifdef HAVE_HDA_SUSPEND_PMSG
	snd_hda_suspend(bus, PMSG_SUSPEND);
#else
	snd_hda_suspend(bus);
#endif
}

void hda_test_resume(void)
{
	snd_hda_resume(bus);
}

static inline unsigned int random_bit(unsigned int pincap, unsigned int mask,
				      unsigned int bit)
{
	if (pincap & mask) {
		if (random() & 1)
			return bit;
	}
	return 0;
}

static void randomize_amp(struct xhda_amp_caps *caps,
			  struct xhda_amp_vals *vals, int nums)
{
	int i, c;

	for (i = 0; i < nums; i++)
		for (c = 0; c < 2; c++) {
			if (caps->nsteps)
				vals->vals[i][c] = random() % caps->nsteps;
			else
				vals->vals[i][c] = 0;
			if (caps->mute && (random() & 1))
				vals->vals[i][c] |= 0x80;
		}
}

void hda_test_pm_randomize(void)
{
	struct xhda_node *node;
	for (node = proc.afg.next; node; node = node->next) {
		unsigned int type;
		type = (node->wcaps & AC_WCAP_TYPE) >> AC_WCAP_TYPE_SHIFT;

		if (type == AC_WID_PIN)
			node->pinctl = random() & 0xff;
		if (node->wcaps & AC_WCAP_IN_AMP)
			randomize_amp(node->amp_in_caps.override ?
				      &node->amp_in_caps :
				      &proc.afg.amp_in_caps,
				      &node->amp_in_vals, node->num_nodes);
		if (node->wcaps & AC_WCAP_OUT_AMP)
			randomize_amp(node->amp_out_caps.override ?
				      &node->amp_out_caps :
				      &proc.afg.amp_out_caps,
				      &node->amp_out_vals, 1);
	}
}

void hda_test_pm_reinit(void)
{
	struct xhda_node *node;
	for (node = proc.afg.next; node; node = node->next) {
		node->pinctl = node->orig_pinctl;
		node->amp_in_vals = node->orig_amp_in_vals;
		node->amp_out_vals = node->orig_amp_out_vals;
	}
}

/*
 * unsol even handling
 */

static void issue_unsol(int caddr, int res)
{
	/* no unsol handling during D3 */
	if (proc.afg.power_current == 3)
		return;

	caddr |= 1 << 4;
	/* ALC880 has incompatible unsol tag */
	if (_codec->vendor_id == 0x10ec0880)
		res = (res & 0x3f) << 28;
	else
		res = (res & 0x3f) << 26;

	snd_hda_queue_unsol_event(bus, res, caddr);
}

#ifdef CONFIG_SND_HDA_RECONFIG

/*
 * sysfs reset simulation
 */
static void reset_pcm(void);

void hda_codec_reset(void)
{
	snd_hda_codec_reset(_codec);
	reset_pcm();
}

/*
 * sysfs reconfig simulation
 */
int hda_codec_reconfig(void)
{
	int err;

	snd_hda_power_up(_codec);
	snd_hda_codec_reset(_codec);
	err = snd_hda_codec_configure(_codec);
	if (err < 0)
		goto error;
	/* rebuild PCMs */
	err = snd_hda_build_pcms(bus);
	if (err < 0)
		goto error;
	/* rebuild mixers */
	err = snd_hda_codec_build_controls(_codec);
 error:
	snd_hda_power_down(_codec);
	return err;
}

#ifdef HAVE_USER_PINCFGS
/*
 * sysfs pin_configs simulations
 */
static void show_pincfgs(struct snd_array *list)
{
	int i;
	for (i = 0; i < list->used; i++) {
		struct hda_pincfg *pin = snd_array_elem(list, i);
		hda_log(HDA_LOG_INFO, "0x%02x 0x%08x\n", pin->nid, pin->cfg);
	}
}

void hda_log_show_driver_pin_configs(void)
{
	show_pincfgs(&_codec->driver_pins);
}

void hda_log_show_init_pin_configs(void)
{
	show_pincfgs(&_codec->init_pins);
}

void hda_log_show_user_pin_configs(void)
{
	show_pincfgs(&_codec->user_pins);
}

void hda_log_set_user_pin_configs(unsigned int nid, unsigned int cfg)
{
	snd_hda_add_pincfg(_codec, &_codec->user_pins, nid, cfg);
}
#endif /* HAVE_USER_PINCFGS */

extern int _show_hints(struct hda_codec *codec, const char *key);
extern int _parse_hints(struct hda_codec *codec, const char *buf);

void hda_log_show_hints(char *hint)
{
	_show_hints(_codec, hint);
}

void hda_log_set_hints(char *hint)
{
	_parse_hints(_codec, hint);
}

#endif /* CONFIG_SND_HDA_RECONFIG */

/*
 * PCM
 */

#define MAX_PCM_STREAMS		16

static int num_pcm_streams;
static struct hda_pcm *pcm_streams[MAX_PCM_STREAMS];

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
		struct hda_pcm *p = pcm_streams[i];
		if (!p->stream[0].substreams && !p->stream[1].substreams)
			continue;
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

static  char *fmt_names[64] = {
	"S8", "U8", "S16_LE", "S16_BE", "U16_LE", "U16_BE", "S24_LE", "S24_BE",
	"U24_LE", "U24_BE", "S32_LE", "S32_BE", "U32_LE", "U32_BE",
	"FLOAT_LE", "FLOAT_BE", "FLOAT64_LE", "FLOAT64_BE",
	"IEC958_LE", "IEC958_BE", "MU_LAW", "A_LAW", "IMA_ADPCM", "MPEG" "GSM",
	NULL, NULL, NULL, NULL, NULL, NULL, "SPECIAL",
	"S24_3LE", "S24_3BE", "U24_3LE", "U24_3BE"
	"S24_3LE", "S20_3BE", "U20_3LE", "U20_3BE",
	"S18_3LE", "S18_3BE", "U18_3LE", "U18_3BE",
};

static int rate_consts[] = {
	5512, 8000, 11025, 16000, 22050, 32000, 44100, 48000, 64000,
	88200, 96000, 176400, 192000,
};

/* test the given PCM stream, called from hda-ctlsh.c */
void hda_test_pcm(int id, int op, int subid,
		  int dir, int rate, int channels, int format)
{
#ifndef HAVE_HDA_ATTACH_PCM
	static struct snd_pcm_substream dummy_substream;
	static struct snd_pcm_runtime dummy_runtime;
	static struct snd_pcm_str dummy_pstr;
#endif
	struct snd_pcm_substream *substream;
	struct snd_pcm_runtime *runtime;
	static struct snd_pcm_str *pstr;
	struct hda_pcm_stream *hinfo;
	unsigned int format_val;
	unsigned int ctls = 0;
	int i, err;

	if (id < 0 || id >= num_pcm_streams || !pcm_streams[id]) {
		hda_log(HDA_LOG_ERR, "Invalid PCM id %d\n", id);
		return;
	}
	if (!pcm_streams[id]->stream[0].substreams &&
	    !pcm_streams[id]->stream[1].substreams) {
		hda_log(HDA_LOG_ERR, "Empty PCM for id %d\n", id);
		return;
	}

	if (!pcm_streams[id]->stream[dir].substreams) {
		hda_log(HDA_LOG_INFO, "No substream in PCM %s for %s\n",
			pcm_streams[id]->name,
			(dir ? "capt" : "play"));
		return;
	}
	if (subid < 0 || subid >= pcm_streams[id]->stream[dir].substreams) {
		hda_log(HDA_LOG_INFO,
			"Invalid substream %d for PCM %s for %s\n",
			subid, pcm_streams[id]->name,
			(dir ? "capt" : "play"));
		return;
	}

#ifdef HAVE_HDA_ATTACH_PCM
	pstr = &pcm_streams[id]->pcm->streams[dir];
	substream = &pstr->substream[subid];
	runtime = substream->runtime;
	if (op == PCM_TEST_END) {
		if (!substream->ref_count) {
			hda_log(HDA_LOG_ERR, "PCM stream not opened\n");
			return;
		}
	} else {
		if (substream->ref_count) {
			hda_log(HDA_LOG_ERR, "PCM stream already opened\n");
			return;
		}
		runtime = calloc(1, sizeof(*runtime));
		if (!runtime) {
			hda_log(HDA_LOG_ERR, "cannot malloc\n");
			exit(1);
		}
		substream->runtime = runtime;
		substream->ref_count = 1;
		pstr->substream_opened = 1;
	}
#else
	substream = &dummy_substream;
	runtime = &dummy_runtime;
	pstr = &dummy_pstr;
	memset(substream, 0, sizeof(*substream));
	memset(runtime, 0, sizeof(*runtime));
	substream->stream = dir;
	substream->number = subid;
	substream->runtime = runtime;
	substream->ref_count = 1;
	substream->pstr = pstr;
	pstr->substream_opened = 1;
#endif

	runtime->rate = rate;
	runtime->format = get_alsa_format(format);
	runtime->channels = channels;

	hinfo = &pcm_streams[id]->stream[dir];

	runtime->hw.channels_min = hinfo->channels_min;
	runtime->hw.channels_max = hinfo->channels_max;
	runtime->hw.formats = hinfo->formats;
	runtime->hw.rates = hinfo->rates;

#ifdef INDIVIDUAL_SPDIF_CTLS
	{
		struct hda_spdif_out *spdif;
		spdif = snd_hda_spdif_out_of_nid(_codec, hinfo->nid);
		ctls = spdif ? spdif->ctls : 0;
	}
#elif defined(STREAM_FORMAT_WITH_SPDIF)
	ctls = _codec->spdif_ctls;
#endif

	if (op != PCM_TEST_END) {
		hda_log(HDA_LOG_INFO, "Open PCM %s for %s\n",
			pcm_streams[id]->name,
			(dir ? "capt" : "play"));
		snd_hda_power_up(_codec);
		err = hinfo->ops.open(hinfo, _codec, substream);
		if (err < 0) {
			hda_log(HDA_LOG_INFO, "Open error = %d\n", err);
			snd_hda_power_down(_codec);
			return;
		}
	
		hda_log(HDA_LOG_INFO, "Available PCM parameters:\n");
		hda_log(HDA_LOG_INFO, "  channels: %d/%d\n",
			runtime->hw.channels_min,
			runtime->hw.channels_max);
		hda_log(HDA_LOG_INFO, "  formats:");
		for (i = 0; i < ARRAY_SIZE(fmt_names); i++) {
			if (runtime->hw.formats & (1ULL << i)) {
				if (fmt_names[i])
					hda_log(HDA_LOG_INFO, " %s", fmt_names[i]);
				else
					hda_log(HDA_LOG_INFO, " Uknown#%d", i);
			}
		}
		hda_log(HDA_LOG_INFO, "\n  rates:");
		for (i = 0; i < ARRAY_SIZE(rate_consts); i++) {
			if (runtime->hw.rates & (1UL << i))
				hda_log(HDA_LOG_INFO, " %d", rate_consts[i]);
		}
		hda_log(HDA_LOG_INFO, "\n");
		if (channels < runtime->hw.channels_min ||
		    channels > runtime->hw.channels_max) {
			hda_log(HDA_LOG_ERR, "Channels count (%d) not available for %s\n", 
				channels, (dir ? "capture" : "playback"));
			snd_hda_power_down(_codec);
			return;
		}
		hda_log(HDA_LOG_INFO, "Prepare PCM, rate=%d, channels=%d, "
			"format=%d bits\n",
			rate, channels, format);
		format_val = snd_hda_calc_stream_format(rate, channels,
							get_alsa_format(format),
							format
#if defined(INDIVIDUAL_SPDIF_CTLS) || defined(STREAM_FORMAT_WITH_SPDIF)
							, ctls
#endif
							);
		if (!format_val) {
			snd_hda_power_down(_codec);
			return;
		}
		hda_log(HDA_LOG_INFO, "PCM format_val = 0x%x\n", format_val);
#ifdef HAVE_COMMON_PREPARE
		err = snd_hda_codec_prepare(_codec, hinfo, subid + 1,
					    format_val, substream);
#else
		err = hinfo->ops.prepare(hinfo, _codec, 1, format_val, substream);
#endif
	}

	if (op != PCM_TEST_START) {
		hda_log(HDA_LOG_INFO, "PCM Clean up\n");
#ifdef HAVE_COMMON_PREPARE
		snd_hda_codec_cleanup(_codec, hinfo, substream);
#else
		hinfo->ops.cleanup(hinfo, _codec, substream);
#endif
		substream->ref_count = 0;
		hda_log(HDA_LOG_INFO, "Close PCM\n");
		hinfo->ops.close(hinfo, _codec, substream);
		snd_hda_power_down(_codec);

#ifdef HAVE_HDA_ATTACH_PCM
		substream->runtime = NULL;
		substream->ref_count = 0;
		pstr->substream_opened = 0;
		free(runtime);
	}
#endif
}

/* attach_pcm callback -- register the stream */
static int attach_pcm(struct hda_bus *bus, struct hda_codec *codec,
		      struct hda_pcm *cpcm)
{
	int i, s;

	if (cpcm->stream[SNDRV_PCM_STREAM_PLAYBACK].substreams ||
	    cpcm->stream[SNDRV_PCM_STREAM_CAPTURE].substreams) {
#ifdef OLD_HDA_PCM
		hda_log(HDA_LOG_INFO,
			"Attach PCM name %s, play #%d, capture #%d\n",
			cpcm->name,
			cpcm->stream[SNDRV_PCM_STREAM_PLAYBACK].substreams,
			cpcm->stream[SNDRV_PCM_STREAM_CAPTURE].substreams);
#else
		cpcm->device = num_pcm_streams;
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
		pcm_streams[num_pcm_streams] = cpcm;
#ifdef HAVE_HDA_ATTACH_PCM
		cpcm->pcm = calloc(1, sizeof(*cpcm->pcm));
		if (!cpcm->pcm) {
			hda_log(HDA_LOG_ERR, "cannot malloc\n");
			exit(1);
		}
		cpcm->pcm->device = cpcm->device;
		for (s = 0; s < 2; s++) {
			struct snd_pcm_str *str = &cpcm->pcm->streams[s];
			str->substream_count = cpcm->stream[s].substreams;
			if (!str->substream_count)
				continue;
			str->substream = calloc(str->substream_count,
						sizeof(*str->substream));
			if (!str->substream) {
				hda_log(HDA_LOG_ERR, "cannot malloc\n");
				exit(1);
			}
			for (i = 0; i < str->substream_count; i++) {
				str->substream[i].pcm = cpcm->pcm;
				str->substream[i].pstr = str;
				str->substream[i].stream = s;
				str->substream[i].number = i;
			}
		}
#endif
	}
	num_pcm_streams++;
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

#ifndef HAVE_HDA_ATTACH_PCM
static int azx_pcm_create(struct hda_codec *codec)
{
	int c, err;

	/* create audio PCMs */
	for (c = 0; c < codec->num_pcms; c++) {
		err = attach_pcm(codec->bus, codec, &codec->pcm_info[c]);
		if (err < 0)
			return err;
	}
	return 0;
}
#endif

/*
 * power management
 */

#ifdef HAVE_NEW_PM_NOTIFY
static void new_pm_notify(struct hda_bus *bus, bool power_up)
{
	hda_log(HDA_LOG_INFO, "PM-Notified, power_up = %d\n", power_up);
}
#else

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
#endif /* HAVE_NEW_PM_NOTIFY */


/*
 * routings
 */
static int node_type(struct xhda_node *node)
{
	return ((node->wcaps & AC_WCAP_TYPE) >> AC_WCAP_TYPE_SHIFT) & 0xf;
}

static const char *get_node_type_string(struct xhda_node *node)
{
	static char *names[16] = {
		[AC_WID_AUD_OUT] = "Out",
		[AC_WID_AUD_IN] = "In ",
		[AC_WID_AUD_MIX] = "Mix",
		[AC_WID_AUD_SEL] = "Sel",
		[AC_WID_PIN] = "Pin",
		[AC_WID_POWER] = "Pow",
		[AC_WID_VOL_KNB] = "Knb",
		[AC_WID_BEEP] = "Bep",
		[AC_WID_VENDOR] = "Vdr",
	};
	int type = node_type(node);
	return names[type] ? names[type] : "\?\?\?";
}

static int get_muted(unsigned char *amp, unsigned int wcaps)
{
	if (wcaps & AC_WCAP_STEREO)
		return (amp[0] & amp[1]) & 0x80;
	else
		return amp[0] & 0x80;
}

static void show_route_lists(struct xhda_route_list *list, unsigned flags)
{
	int i;
	int show_mute = !!(flags & SHOW_MUTE);

	for (; list; list = list->next) {
		hda_nid_t prev_nid = 0;
		for (i = 0; i < list->num_nodes; i++) {
			struct xhda_node *node = list->node[i];
			int in_mute = 0, out_mute = 0;
			int idx = 0;

			if (i > 0) {
				for (idx = 0; idx < node->num_nodes; idx++) {
					if (node->node[idx] == prev_nid)
						break;
				}
				if (idx >= node->num_nodes)
					idx = 0;
			}

			if (node->wcaps & AC_WCAP_IN_AMP)
				in_mute = get_muted(&node->amp_in_vals.vals[idx][0],
						    node->wcaps);
			if (node->wcaps & AC_WCAP_OUT_AMP)
				out_mute = get_muted(&node->amp_out_vals.vals[0][0],
						     node->wcaps);
			if (node_type(node) == AC_WID_PIN) {
				if (!i) {
					out_mute = in_mute;
					in_mute = false;
				} else {
					in_mute = out_mute;
					out_mute = false;
				}
			}

			if (i > 0) {
				const char *path;
				if (node_type(node) == AC_WID_AUD_MIX)
					path = " -- ";
				else if (node->node[node->curr_conn] == prev_nid)
					path = " -> ";
				else
					path = " -x ";
				hda_log(HDA_LOG_INFO, "%s%s", path,
					show_mute ? (in_mute ? "|" : " ") : "");
			}
			hda_log(HDA_LOG_INFO, "%s[%02x]%s",
				get_node_type_string(node), node->nid,
				show_mute ? (out_mute ? "|" : " ") : "");
			prev_nid = node->nid;
		}
		hda_log(HDA_LOG_INFO, "\n");
	}
}

void hda_show_routes(int nid, unsigned flags)
{
	struct xhda_route_list *list;
	int had_list = 0;

	if (flags & SHOW_DIR_IN) {
		list = hda_routes_connected_to(&proc, nid, flags);
		show_route_lists(list, flags);
		had_list = list != NULL;
		hda_free_route_lists(list);
	}

	if (flags & SHOW_DIR_OUT) {
		list = hda_routes_connected_from(&proc, nid, flags);
		if (list && had_list)
			hda_log(HDA_LOG_INFO, "\n");
		show_route_lists(list, flags);
		hda_free_route_lists(list);
	}
}


/*
 * pin-config override
 */

static struct xhda_node *find_node(struct xhda_codec *codec, int nid)
{
	struct xhda_node *node;

	for (node = &codec->afg; node; node = node->next) {
		if (node->nid == nid)
			return node;
	}
	return NULL;
}

static void set_pincfg(struct xhda_codec *codec, int nid, int val)
{
	struct xhda_node *node = find_node(codec, nid);
	if (node) {
		node->pin_default = val;
		hda_log(HDA_LOG_INFO, "  Pin 0x%02x to 0x%08x\n", nid, val);
		return;
	}
}

static int override_pincfg(struct xhda_codec *codec, char *pincfg)
{
	FILE *fp;
	char buf[256];
	struct xhda_sysfs_list *sys;
	int is_fw_file, is_in_pincfg;

	if (strchr(pincfg, '=')) {
		/* direct pincfg string */
		int reg, val;
		if (sscanf(pincfg, "%i %i", &reg, &val) != 2) {
			hda_log(HDA_LOG_ERR, "Invalid pincfg %s\n", pincfg);
			return -EINVAL;
		}
		set_pincfg(codec, reg, val);
		return 0;
	}

	for (sys = codec->sysfs_list; sys; sys = sys->next) {
		if (sys->type == XHDA_SYS_PINCFG &&
		    !strcmp(sys->id, pincfg)) {
			struct xhda_sysfs_value *val;
			hda_log(HDA_LOG_INFO, "Overriding pin-configs via %s\n", pincfg);
			for (val = sys->entry.vals; val; val = val->next)
				set_pincfg(codec, val->val[0], val->val[1]);
			return 0;
		}
	}

	/* if not found in the given input, try to open it */
	fp = fopen(pincfg, "r");
	if (!fp) {
		hda_log(HDA_LOG_ERR, "Cannot find init pincfg %s\n", pincfg);
		return -EINVAL;
	}

	hda_log(HDA_LOG_INFO, "Overriding pin-configs from file %s\n", pincfg);
	is_fw_file = 0;
	is_in_pincfg = 0;
	while (fgets(buf, sizeof(buf), fp)) {
		int reg, val;
		if (*buf == '#' || *buf == '\n')
			continue;
		if (is_fw_file) {
			if (*buf == '[') {
				if (is_in_pincfg)
					break;
				is_in_pincfg = !strncmp(buf, "[pincfg]", 8);
				continue;
			} else if (!is_in_pincfg)
				continue;
		} else {
			if (!strncmp(buf, "[codec]", 7)) {
				is_fw_file = 1;
				continue;
			}
		}
		if (sscanf(buf, "%i %i", &reg, &val) != 2)
			break;
		set_pincfg(codec, reg, val);
	}
	fclose (fp);
	return 0;
}

static int load_init_hints(struct xhda_codec *codec, char *hints)
{
#ifdef CONFIG_SND_HDA_RECONFIG
	FILE *fp;
	char buf[256];
	struct xhda_sysfs_list *sys;
	int is_fw_file, is_in_hint;

	if (strchr(hints, '=')) {
		/* direct hint string */
		if (_parse_hints(_codec, hints)) {
			hda_log(HDA_LOG_ERR, "Invalid hints %s\n", hints);
			return -EINVAL;
		}
		return 0;
	}

	for (sys = codec->sysfs_list; sys; sys = sys->next) {
		if (sys->type == XHDA_SYS_HINTS &&
		    !strcmp(sys->id, hints)) {
			struct xhda_sysfs_hints *val;
			hda_log(HDA_LOG_INFO, "Add hints from %s\n", hints);
			for (val = sys->entry.hints; val; val = val->next)
				_parse_hints(_codec, val->line);
			return 0;
		}
	}

	/* if not found in the given input, try to open it */
	fp = fopen(hints, "r");
	if (!fp) {
		hda_log(HDA_LOG_ERR, "Cannot find hints %s\n", hints);
		return -EINVAL;
	}
	hda_log(HDA_LOG_INFO, "Add hints from file %s\n", hints);
	is_fw_file = 0;
	is_in_hint = 0;
	while (fgets(buf, sizeof(buf), fp)) {
		if (*buf == '#' || *buf == '\n')
			continue;
		if (is_fw_file) {
			if (*buf == '[') {
				if (is_in_hint)
					break;
				is_in_hint = !strncmp(buf, "[hint]", 8);
				continue;
			} else if (!is_in_hint)
				continue;
		} else {
			if (!strncmp(buf, "[codec]", 7)) {
				is_fw_file = 1;
				continue;
			}
		}
		_parse_hints(_codec, buf);
	}
	fclose(fp);
	return 0;
#else
	hda_log(HDA_LOG_ERR, "-H option isn't supported for this kernel\n");
	return -EINVAL;
#endif
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
	fprintf(stderr, "  -C             print messages in color (default)\n");
	fprintf(stderr, "  -M             no color print\n");
	fprintf(stderr, "  -F             print prefixes to messages\n");
	fprintf(stderr, "  -a             issues SIGTRAP at codec errors\n");
	fprintf(stderr, "  -n             don't configure codec at start\n");
	fprintf(stderr, "  -P pincfg      initialize pin-configuration from sysfs entry\n");
	fprintf(stderr, "  -H hints       add initial hints from sysfs entry or file\n");
	fprintf(stderr, "  -j NID         turn on the initial jack-state of the given pin\n");
}

#include "kernel/init_hooks.h"

static FILE *popen_var(const char *v1, const char *v2, const char *type)
{
	char tmp[4096];
	snprintf(tmp, sizeof(tmp), "%s %s", v1, v2);
	return popen(tmp, type);
}

static FILE *file_open(const char *fname)
{
	const char *p;

	if (access(fname, R_OK))
		return NULL;

	p = strrchr(fname, '.');
	if (p) {
		if (!strcmp(p, ".bz2"))
			return popen_var("bzcat", fname, "r");
		if (!strcmp(p, ".gz"))
			return popen_var("zcat", fname, "r");
	}
	return fopen(fname, "r");
}

int main(int argc, char **argv)
{
	int c, err;
	FILE *fp;
	int idx = -1;
	int pci_subvendor = 0;
	int pci_subdevice = 0;
	char *opt_model = NULL;
	char *logfile = NULL;
	unsigned int log_flags = HDA_LOG_FLAG_COLOR;
	struct pci_dev mypci;
	struct hda_bus_template temp;
	struct hda_codec *codec;
	char *init_pincfg = NULL;
	char *init_hints = NULL;
	int num_active_jacks = 0;
	int no_configure = 0;
	unsigned int active_jacks[16];

	while ((c = getopt(argc, argv, "al:i:p:m:do:qCMFP:H:j:n")) != -1) {
		switch (c) {
		case 'a':
			hda_log_trap_on_error = 1;
			break;
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
		case 'F':
			log_flags |= HDA_LOG_FLAG_PREFIX;
			break;
		case 'M':
			log_flags &= ~HDA_LOG_FLAG_COLOR;
			break;
		case 'P':
			init_pincfg = optarg;
			break;
		case 'H':
			init_hints = optarg;
			break;
		case 'j':
			if (num_active_jacks >= ARRAY_SIZE(active_jacks)) {
				fprintf(stderr, "Too many -j options given\n");
				return 1;
			}
			active_jacks[num_active_jacks++] =
				strtoul(optarg, NULL, 0);
			break;
		case 'n':
			no_configure = 1;
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

	fp = file_open(argv[optind]);
	if (!fp) {
		fprintf(stderr, "cannot open %s\n", argv[optind]);
		return 1;
	}

	srandom((unsigned int)time(NULL));

	hda_log_init(logfile, log_flags);

	/* ignore SIGTRAP as default; gdb will override anyway... */
	signal(SIGTRAP, SIG_IGN);

	hda_log(HDA_LOG_INFO, "# Parsing..\n");
	if (parse_codec_proc(fp, &proc, idx) < 0) {
		hda_log(HDA_LOG_INFO, "error at reading proc\n");
		return 1;
	}

	if (init_pincfg) {
		if (override_pincfg(&proc, init_pincfg) < 0)
			return 1;
	}
	if (num_active_jacks) {
		int i;
		for (i = 0; i < num_active_jacks; i++) {
			struct xhda_node *node;
			node = find_node(&proc, active_jacks[i]);
			if (node)
				node->jack_state = 1;
		}
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
#else /* !OLD_HDA_CMD */
	temp.ops.command = cmd_send;
#ifdef HAVE_GET_RESPONSE_WITH_CADDR
	temp.ops.get_response = resp_get_caddr;
#else
	temp.ops.get_response = resp_get;
#endif
#ifdef HAVE_HDA_ATTACH_PCM
	temp.ops.attach_pcm = attach_pcm;
#endif
#ifdef HAVE_NEW_PM_NOTIFY
	temp.ops.pm_notify = new_pm_notify;
#else
	temp.ops.pm_notify = pm_notify;
#endif
#endif /* OLD_HDA_CMD */
	gather_codec_hooks();

	if (snd_hda_bus_new(&card, &temp, &bus) < 0) {
		hda_log(HDA_LOG_ERR, "cannot create snd_hda_bus\n");
		return 1;
	}

	ignore_invalid_ftype = 1;
#ifdef OLD_HDA_CODEC_NEW
	err = snd_hda_codec_new(bus, proc.addr, &codec);
#else
	err = snd_hda_codec_new(bus, proc.addr, 1, &codec);
#endif
	ignore_invalid_ftype = 0;
	if (err) {
		hda_log(HDA_LOG_ERR, "cannot create codec\n");
		return 1;
	}
#ifdef HDA_BEEP_MODE_ON
	codec->beep_mode = HDA_BEEP_MODE_ON;
#endif
	_codec = codec;

#ifdef HAVE_USER_PINCFGS
	snd_array_init(&codec->user_pins, sizeof(struct hda_pincfg), 16);
#endif

	if (init_hints) {
		if (load_init_hints(&proc, init_hints) < 0)
			return 1;
	}

#ifdef HAVE_HDA_PATCH_LOADER
	if (!no_configure)
		snd_hda_codec_configure(codec);
#endif

	if (!no_configure) {
		hda_log(HDA_LOG_INFO, "# Building PCMs...\n");
		snd_hda_build_pcms(bus);
#ifndef HAVE_HDA_ATTACH_PCM
		azx_pcm_create(codec);
#endif

		hda_log(HDA_LOG_INFO, "# Init and building controls...\n");
#ifdef CONFIG_SND_HDA_RECONFIG
		snd_hda_codec_build_controls(codec);
#else
		snd_hda_build_controls(codec->bus);
#endif
	}

	/* power-down after init phase */
	snd_hda_power_down(codec);
	
	cmd_loop(stdin);

	return 0;
}
