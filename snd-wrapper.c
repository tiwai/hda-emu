/*
 * hda-emu - simple HD-audio codec emulator for debugging snd-hda-intel driver
 *
 * Misc wrappers
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

#include <ctype.h>
#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <assert.h>
#include "hda/hda_codec.h"
#include "hda-types.h"
#include "hda-log.h"

int snd_pcm_format_width(int format)
{
	switch (format) {
	case SNDRV_PCM_FORMAT_U8:
		return 8;
	case SNDRV_PCM_FORMAT_S16_LE:
		return 16;
	case SNDRV_PCM_FORMAT_S32_LE:
	case SNDRV_PCM_FORMAT_FLOAT_LE:
	case SNDRV_PCM_FORMAT_IEC958_SUBFRAME_LE:
		return 32;
	default:
		return -EINVAL;
	}
}

#ifdef CONFIG_SND_HDA_RECONFIG

/* hint string pair */
struct hda_hint {
	const char *key;
	const char *val;	/* contained in the same alloc as key */
};

static char *skip_spaces(const char *p)
{
	while (*p && isspace(*p))
		p++;
	return (char *)p;
}

static void remove_trail_spaces(char *str)
{
	char *p;
	if (!*str)
		return;
	p = str + strlen(str) - 1;
	for (; isspace(*p); p--) {
		*p = 0;
		if (p == str)
			return;
	}
}

static struct hda_hint *get_hint(struct hda_codec *codec, const char *key)
{
	int i;

	for (i = 0; i < codec->hints.used; i++) {
		struct hda_hint *hint = snd_array_elem(&codec->hints, i);
		if (!strcmp(hint->key, key))
			return hint;
	}
	return NULL;
}

#define MAX_HINTS	1024

int _parse_hints(struct hda_codec *codec, const char *buf)
{
	char *key, *val;
	struct hda_hint *hint;
	int err = 0;

	buf = skip_spaces(buf);
	if (!*buf || *buf == '#' || *buf == '\n')
		return 0;
	if (*buf == '=')
		return -EINVAL;
	key = strdup(buf);
	if (!key)
		return -ENOMEM;
	val = strrchr(buf, '\n');
	if (val)
		*val = 0;
	/* extract key and val */
	val = strchr(key, '=');
	if (!val) {
		kfree(key);
		return -EINVAL;
	}
	*val++ = 0;
	val = skip_spaces(val);
	remove_trail_spaces(key);
	remove_trail_spaces(val);
	hint = get_hint(codec, key);
	if (hint) {
		/* replace */
		free((void *)hint->key);
		hint->key = key;
		hint->val = val;
		return 0;
	}
	/* allocate a new hint entry */
	if (codec->hints.used >= MAX_HINTS)
		hint = NULL;
	else
		hint = snd_array_new(&codec->hints);
	if (hint) {
		hint->key = key;
		hint->val = val;
	} else {
		err = -ENOMEM;
	}
	if (err)
		free(key);
	return err;
}

int _show_hints(struct hda_codec *codec, const char *key)
{
	int i;

	for (i = 0; i < codec->hints.used; i++) {
		struct hda_hint *hint = snd_array_elem(&codec->hints, i);
		if (!key || !strcmp(hint->key, key))
			hda_log(HDA_LOG_INFO, "%s = %s\n", hint->key, hint->val);
	}
	return 0;
}

const char *snd_hda_get_hint(struct hda_codec *codec, const char *key)
{
	struct hda_hint *hint = get_hint(codec, key);
	return hint ? hint->val : NULL;
}

int snd_hda_get_bool_hint(struct hda_codec *codec, const char *key)
{
	const char *p;
	int ret;

	p = snd_hda_get_hint(codec, key);
	if (!p || !*p)
		ret = -ENOENT;
	else {
		switch (toupper(*p)) {
		case 'T': /* true */
		case 'Y': /* yes */
		case '1':
			ret = 1;
			break;
		default:
			ret = 0;
			break;
		}
	}
	return ret;
}

int snd_hda_get_int_hint(struct hda_codec *codec, const char *key, int *valp)
{
	const char *p;
	int ret;

	p = snd_hda_get_hint(codec, key);
	if (!p)
		ret = -ENOENT;
	else {
		*valp = strtoul(p, NULL, 0);
		ret = 0;
	}
	return ret;
}

#endif /* CONFIG_SND_HDA_RECONFIG */

int snd_hda_create_hwdep(struct hda_codec *codec)
{
#ifdef CONFIG_SND_HDA_RECONFIG

#ifdef HAVE_CODEC_USER_MUTEX
	mutex_init(&codec->user_mutex);
#endif
	snd_array_init(&codec->init_verbs, sizeof(struct hda_verb), 32);
	snd_array_init(&codec->hints, sizeof(struct hda_hint), 32);
	snd_array_init(&codec->user_pins, sizeof(struct hda_pincfg), 16);
#endif /* CONFIG_SND_HDA_RECONFIG */
	return 0;
}

int snd_hda_hwdep_add_sysfs(struct hda_codec *codec)
{
	return 0;
}

/*
 */
void (*snd_iprintf_dumper)(struct snd_info_buffer *buf,
			   const char *fmt, va_list ap);

void snd_iprintf(struct snd_info_buffer *buf, const char *fmt, ...)
{
	va_list ap;
	if (!snd_iprintf_dumper)
		return;
	va_start(ap, fmt);
	snd_iprintf_dumper(buf, fmt, ap);
	va_end(ap);
}

/* there is a compat wrapper in the latest SLE11 kernel */
#ifndef snd_pci_quirk_lookup

/*
 * quirk lookup
 */
const struct snd_pci_quirk *
snd_pci_quirk_lookup(struct pci_dev *pci, const struct snd_pci_quirk *list)
{
	const struct snd_pci_quirk *q;

	for (q = list; q->subvendor; q++) {
		if (q->subvendor != pci->subsystem_vendor)
			continue;
#ifdef NEW_QUIRK_LIST
		if (!q->subdevice ||
		    (pci->subsystem_device & q->subdevice_mask) == q->subdevice)
			return q;
#else
		if (!q->subdevice ||
		    q->subdevice == pci->subsystem_device)
			return q;
#endif
	}
	return NULL;
}

#endif /* snd_pci_quirk_lookup */

/* malloc debug */
struct __hda_malloc_elem {
	void *ptr;
	size_t size;
	const char *file;
	int line;
	struct list_head list;
};

static LIST_HEAD(malloc_list);

void *__hda_malloc(size_t size, const char *file, int line, int gfp)
{
	struct __hda_malloc_elem *elem = malloc(sizeof(*elem));
	if (!elem)
		return NULL;
	if (gfp & __GFP_ZERO)
		elem->ptr = calloc(1, size);
	else
		elem->ptr = malloc(size);
	if (!elem->ptr) {
		free(elem);
		return NULL;
	}
	elem->file = file;
	elem->size = size;
	elem->line = line;
	list_add_tail(&elem->list, &malloc_list);
	return elem->ptr;
}

void __hda_free(void *ptr, const char *file, int line)
{
	struct __hda_malloc_elem *elem;

	if (!ptr)
		return;

	list_for_each_entry(elem, &malloc_list, list) {
		if (elem->ptr == ptr) {
			list_del(&elem->list);
			free(elem->ptr);
			free(elem);
			return;
		}
	}
	hda_log(HDA_LOG_ERR, "Untracked malloc freed in %s:%d\n",
		file, line);
	assert(0);
}

void *__hda_realloc(const void *p, size_t new_size, const char *file, int line, int gfp)
{
	struct __hda_malloc_elem *elem;

	if (!p)
		return __hda_malloc(new_size, file, line, gfp);
	if (!new_size) {
		__hda_free((void *)p, file, line);
		return NULL;
	}

	list_for_each_entry(elem, &malloc_list, list) {
		if (elem->ptr == p) {
			void *nptr;
			if (gfp & __GFP_ZERO)
				nptr = calloc(1, new_size);
			else
				nptr = malloc(new_size);
			if (nptr) {
				memcpy(nptr, elem->ptr, elem->size);
				free(elem->ptr);
				elem->ptr = nptr;
				elem->size = new_size;
			}
			return nptr;
		}
	}
	hda_log(HDA_LOG_ERR, "Untracked malloc realloced in %s:%d\n",
		file, line);
	return __hda_malloc(new_size, file, line, gfp);
}

void *__hda_strdup(const char *str, const char *file, int line, int gfp)
{
	char *dest = __hda_malloc(strlen(str) + 1, file, line, gfp);
	if (!dest)
		return NULL;
	strcpy(dest, str);
	return dest;
}

/* jack API */
#include <sound/jack.h>
int snd_jack_new(struct snd_card *card, const char *id, int type,
		 struct snd_jack **jack)
{
	struct snd_jack *jp;

	jp = calloc(1, sizeof(*jp));
	if (!jp)
		return -ENOMEM;
	jp->id = strdup(id);
	if (!jp->id)
		return -ENOMEM;
	jp->type = type;
	hda_log(HDA_LOG_INFO, "JACK created %s, type %d\n", id, type);
	*jack = jp;
	return 0;
}

void snd_jack_set_parent(struct snd_jack *jack, struct device *parent)
{
	/* NOP */
}

void snd_jack_report(struct snd_jack *jack, int status)
{
	hda_log(HDA_LOG_INFO, "JACK report %s, status %d\n", jack->id, status);
}

/*
 * lock
 */
void mylock_init(int *lock)
{
	*lock = MYLOCK_UNLOCKED;
}

void mylock_lock(int *lock, const char *file, int line)
{
	switch (*lock) {
	case MYLOCK_UNINIT:
		hda_log(HDA_LOG_ERR, "Locking uninitialized at %s:%d\n",
			file, line);
		break;
	case MYLOCK_UNLOCKED:
		*lock = MYLOCK_LOCKED;
		break;
	case MYLOCK_LOCKED:
		hda_log(HDA_LOG_ERR, "Double-lock detected at %s:%d\n",
			file, line);
		break;
	default:
		hda_log(HDA_LOG_ERR, "Unknown lock state %d! at %s:%d\n",
			*lock, file, line);
		break;
	}
}

void mylock_unlock(int *lock, const char *file, int line)
{
	switch (*lock) {
	case MYLOCK_UNINIT:
		hda_log(HDA_LOG_ERR, "Unlocking uninitialized at %s:%d\n",
			file, line);
		break;
	case MYLOCK_UNLOCKED:
		hda_log(HDA_LOG_ERR, "Double-unlock detected at %s:%d\n",
			file, line);
		break;
	case MYLOCK_LOCKED:
		*lock = MYLOCK_UNLOCKED;
		break;
	default:
		hda_log(HDA_LOG_ERR, "Unknown lock state %d! at %s:%d\n",
			*lock, file, line);
		break;
	}
}

#define MYLOCK_WRITE_LOCKED	0x10000

void mylock_read_lock(int *lock, const char *file, int line)
{
	if (*lock == MYLOCK_UNINIT) {
		hda_log(HDA_LOG_ERR, "Read-locking uninitialized obj at %s:%d\n",
			file, line);
		return;
	}
	if (*lock >= MYLOCK_WRITE_LOCKED) {
		hda_log(HDA_LOG_ERR, "Read-locking write-locked obj at %s:%d\n",
			file, line);
		return;
	}
	(*lock)++;
}

void mylock_read_unlock(int *lock, const char *file, int line)
{
	if (*lock == MYLOCK_UNINIT) {
		hda_log(HDA_LOG_ERR, "Read-unlocking uninitialized obj at %s:%d\n",
			file, line);
		return;
	}
	if (*lock == MYLOCK_UNLOCKED) {
		hda_log(HDA_LOG_ERR, "Read-unlocking unlocked obj at %s:%d\n",
			file, line);
		return;
	}
	if (*lock >= MYLOCK_WRITE_LOCKED) {
		hda_log(HDA_LOG_ERR, "Read-unlocking write-locked obj at %s:%d\n",
			file, line);
		return;
	}
	(*lock)--;
}

void mylock_write_lock(int *lock, const char *file, int line)
{
	if (*lock == MYLOCK_UNINIT) {
		hda_log(HDA_LOG_ERR, "Write-locking uninitialized obj at %s:%d\n",
			file, line);
		return;
	}
	if (*lock == MYLOCK_WRITE_LOCKED) {
		hda_log(HDA_LOG_ERR, "Double write-locking at %s:%d\n",
			file, line);
		return;
	}
	if (*lock != MYLOCK_UNLOCKED) {
		hda_log(HDA_LOG_ERR, "Write-locking read-locked obj at %s:%d\n",
			file, line);
		return;
	}
	*lock = MYLOCK_WRITE_LOCKED;
}

void mylock_write_unlock(int *lock, const char *file, int line)
{
	if (*lock == MYLOCK_UNINIT) {
		hda_log(HDA_LOG_ERR, "Write-unlocking uninitialized obj at %s:%d\n",
			file, line);
		return;
	}
	if (*lock == MYLOCK_UNLOCKED) {
		hda_log(HDA_LOG_ERR, "Write-unlocking unlocked obj at %s:%d\n",
			file, line);
		return;
	}
	if (*lock != MYLOCK_WRITE_LOCKED) {
		hda_log(HDA_LOG_ERR, "Write-unlocking read-locked obj at %s:%d\n",
			file, line);
		return;
	}
	*lock = MYLOCK_UNLOCKED;
}

/*
 * standard channel mapping helpers
 */
#include <sound/tlv.h>

/* default channel maps for multi-channel playbacks, up to 8 channels */
const struct snd_pcm_chmap_elem snd_pcm_std_chmaps[] = {
	{ .channels = 1,
	  .map = { SNDRV_CHMAP_MONO } },
	{ .channels = 2,
	  .map = { SNDRV_CHMAP_FL, SNDRV_CHMAP_FR } },
	{ .channels = 4,
	  .map = { SNDRV_CHMAP_FL, SNDRV_CHMAP_FR,
		   SNDRV_CHMAP_RL, SNDRV_CHMAP_RR } },
	{ .channels = 6,
	  .map = { SNDRV_CHMAP_FL, SNDRV_CHMAP_FR,
		   SNDRV_CHMAP_RL, SNDRV_CHMAP_RR,
		   SNDRV_CHMAP_FC, SNDRV_CHMAP_LFE } },
	{ .channels = 8,
	  .map = { SNDRV_CHMAP_FL, SNDRV_CHMAP_FR,
		   SNDRV_CHMAP_RL, SNDRV_CHMAP_RR,
		   SNDRV_CHMAP_FC, SNDRV_CHMAP_LFE,
		   SNDRV_CHMAP_SL, SNDRV_CHMAP_SR } },
	{ }
};
EXPORT_SYMBOL_GPL(snd_pcm_std_chmaps);

/* alternative channel maps with CLFE <-> surround swapped for 6/8 channels */
const struct snd_pcm_chmap_elem snd_pcm_alt_chmaps[] = {
	{ .channels = 1,
	  .map = { SNDRV_CHMAP_MONO } },
	{ .channels = 2,
	  .map = { SNDRV_CHMAP_FL, SNDRV_CHMAP_FR } },
	{ .channels = 4,
	  .map = { SNDRV_CHMAP_FL, SNDRV_CHMAP_FR,
		   SNDRV_CHMAP_RL, SNDRV_CHMAP_RR } },
	{ .channels = 6,
	  .map = { SNDRV_CHMAP_FL, SNDRV_CHMAP_FR,
		   SNDRV_CHMAP_FC, SNDRV_CHMAP_LFE,
		   SNDRV_CHMAP_RL, SNDRV_CHMAP_RR } },
	{ .channels = 8,
	  .map = { SNDRV_CHMAP_FL, SNDRV_CHMAP_FR,
		   SNDRV_CHMAP_FC, SNDRV_CHMAP_LFE,
		   SNDRV_CHMAP_RL, SNDRV_CHMAP_RR,
		   SNDRV_CHMAP_SL, SNDRV_CHMAP_SR } },
	{ }
};
EXPORT_SYMBOL_GPL(snd_pcm_alt_chmaps);

static bool valid_chmap_channels(const struct snd_pcm_chmap *info, int ch)
{
	if (ch > info->max_channels)
		return false;
	return !info->channel_mask || (info->channel_mask & (1U << ch));
}

static int pcm_chmap_ctl_info(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_info *uinfo)
{
	struct snd_pcm_chmap *info = snd_kcontrol_chip(kcontrol);

	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 0;
	uinfo->count = info->max_channels;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = SNDRV_CHMAP_LAST;
	return 0;
}

/* get callback for channel map ctl element
 * stores the channel position firstly matching with the current channels
 */
static int pcm_chmap_ctl_get(struct snd_kcontrol *kcontrol,
			     struct snd_ctl_elem_value *ucontrol)
{
	struct snd_pcm_chmap *info = snd_kcontrol_chip(kcontrol);
	unsigned int idx = snd_ctl_get_ioffidx(kcontrol, &ucontrol->id);
	struct snd_pcm_substream *substream;
	const struct snd_pcm_chmap_elem *map;

	if (snd_BUG_ON(!info->chmap))
		return -EINVAL;
	substream = snd_pcm_chmap_substream(info, idx);
	if (!substream)
		return -ENODEV;
	memset(ucontrol->value.integer.value, 0,
	       sizeof(ucontrol->value.integer.value));
	if (!substream->runtime)
		return 0; /* no channels set */
	for (map = info->chmap; map->channels; map++) {
		int i;
		if (map->channels == substream->runtime->channels &&
		    valid_chmap_channels(info, map->channels)) {
			for (i = 0; i < map->channels; i++)
				ucontrol->value.integer.value[i] = map->map[i];
			return 0;
		}
	}
	return -EINVAL;
}

/* tlv callback for channel map ctl element
 * expands the pre-defined channel maps in a form of TLV
 */
static int pcm_chmap_ctl_tlv(struct snd_kcontrol *kcontrol, int op_flag,
			     unsigned int size, unsigned int __user *tlv)
{
	struct snd_pcm_chmap *info = snd_kcontrol_chip(kcontrol);
	const struct snd_pcm_chmap_elem *map;
	unsigned int __user *dst;
	int c, count = 0;

	if (snd_BUG_ON(!info->chmap))
		return -EINVAL;
	if (size < 8)
		return -ENOMEM;
	if (put_user(SNDRV_CTL_TLVT_CONTAINER, tlv))
		return -EFAULT;
	size -= 8;
	dst = tlv + 2;
	for (map = info->chmap; map->channels; map++) {
		int chs_bytes = map->channels * 4;
		if (!valid_chmap_channels(info, map->channels))
			continue;
		if (size < 8)
			return -ENOMEM;
		if (put_user(SNDRV_CTL_TLVT_CHMAP_FIXED, dst) ||
		    put_user(chs_bytes, dst + 1))
			return -EFAULT;
		dst += 2;
		size -= 8;
		count += 8;
		if (size < chs_bytes)
			return -ENOMEM;
		size -= chs_bytes;
		count += chs_bytes;
		for (c = 0; c < map->channels; c++) {
			if (put_user(map->map[c], dst))
				return -EFAULT;
			dst++;
		}
	}
	if (put_user(count, tlv + 1))
		return -EFAULT;
	return 0;
}

static void pcm_chmap_ctl_private_free(struct snd_kcontrol *kcontrol)
{
	struct snd_pcm_chmap *info = snd_kcontrol_chip(kcontrol);
	info->pcm->streams[info->stream].chmap_kctl = NULL;
	kfree(info);
}

/**
 * snd_pcm_add_chmap_ctls - create channel-mapping control elements
 * @pcm: the assigned PCM instance
 * @stream: stream direction
 * @chmap: channel map elements (for query)
 * @max_channels: the max number of channels for the stream
 * @private_value: the value passed to each kcontrol's private_value field
 * @info_ret: store struct snd_pcm_chmap instance if non-NULL
 *
 * Create channel-mapping control elements assigned to the given PCM stream(s).
 * Returns zero if succeed, or a negative error value.
 */
int snd_pcm_add_chmap_ctls(struct snd_pcm *pcm, int stream,
			   const struct snd_pcm_chmap_elem *chmap,
			   int max_channels,
			   unsigned long private_value,
			   struct snd_pcm_chmap **info_ret)
{
	struct snd_pcm_chmap *info;
	struct snd_kcontrol_new knew = {
		.iface = SNDRV_CTL_ELEM_IFACE_PCM,
		.access = SNDRV_CTL_ELEM_ACCESS_READ |
			SNDRV_CTL_ELEM_ACCESS_TLV_READ |
			SNDRV_CTL_ELEM_ACCESS_TLV_CALLBACK,
		.info = pcm_chmap_ctl_info,
		.get = pcm_chmap_ctl_get,
		.tlv.c = pcm_chmap_ctl_tlv,
	};
	int err;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;
	info->pcm = pcm;
	info->stream = stream;
	info->chmap = chmap;
	info->max_channels = max_channels;
	if (stream == SNDRV_PCM_STREAM_PLAYBACK)
		knew.name = "Playback Channel Map";
	else
		knew.name = "Capture Channel Map";
	knew.device = pcm->device;
	knew.count = pcm->streams[stream].substream_count;
	knew.private_value = private_value;
	info->kctl = snd_ctl_new1(&knew, info);
	if (!info->kctl) {
		kfree(info);
		return -ENOMEM;
	}
	info->kctl->private_free = pcm_chmap_ctl_private_free;
	err = snd_ctl_add(pcm->card, info->kctl);
	if (err < 0)
		return err;
	pcm->streams[stream].chmap_kctl = info->kctl;
	if (info_ret)
		*info_ret = info;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_pcm_add_chmap_ctls);

int tpacpi_led_set(int whichled, bool on)
{
	hda_log(HDA_LOG_INFO, "Setting thinkpad LED %d to %s\n", whichled, on ? "on" : "off" );
	return 0;
}

int dell_app_wmi_led_set(int whichled, int on)
{
	hda_log(HDA_LOG_INFO, "Setting Dell LED %d to %s\n", whichled, on ? "on" : "off" );
	return 0;
}

/*
 * dummy entries just for builds
 */
void snd_hda_sysfs_init(struct hda_codec *codec) {}
void snd_hda_sysfs_clear(struct hda_codec *codec) {}
struct class *sound_class;
const struct attribute_group *snd_hda_dev_attr_groups[1];

/*
 * for device binding
 */

static LIST_HEAD(registered_drivers);

int driver_register(struct device_driver *drv)
{
	list_add_tail(&drv->list, &registered_drivers);
	return 0;
}

void driver_unregister(struct device_driver *drv)
{
	list_del(&drv->list);
}

static struct bus_type *_bus;

int bus_register(struct bus_type *bus)
{
	_bus = bus;
	return 0;
}

void bus_unregister(struct bus_type *bus)
{
}

void device_initialize(struct device *dev)
{
	dev->pmcnt = 0;
}

int device_add(struct device *dev)
{
	dev->registered = true;
	return device_attach(dev);
}

void device_del(struct device *dev)

{
	dev->registered = false;
	dev->driver->remove(dev);
	dev->driver = NULL;
}

int device_attach(struct device *dev)
{
	struct device_driver *drv;
	int err;

	if (!_bus) {
		hda_log(HDA_LOG_ERR, "Bus not registered!!\n");
		return -ENXIO;
	}

	list_for_each_entry(drv, &registered_drivers, list) {
		if (!_bus->match(dev, drv))
			continue;
		dev->driver = drv;
		err = drv->probe(dev);
		if (!err)
			return 1; /* bound */
		dev->driver = NULL;
		if (err < 0) {
			hda_log(HDA_LOG_INFO, "Driver %s not bound\n", drv->name);
			continue;
		}
	}

	return 0;
}

/*
 */
static void check_resume(struct device *dev)
{
	if (dev->pmcnt > 0 && dev->pmsuspended) {
		hda_log(HDA_LOG_INFO, "Codec resuming...\n");
		dev->pmsuspended = false;
		dev->driver->pm->runtime_resume(dev);
	}
}

int pm_runtime_get_sync(struct device *dev)
{
	dev->pmcnt++;
	check_resume(dev);
	return 0;
}

static void check_suspend(struct device *dev)
{
	if (!dev->pmcnt && !dev->pmsuspended && dev->pmallow) {
		hda_log(HDA_LOG_INFO, "Codec suspending...\n");
		dev->driver->pm->runtime_suspend(dev);
		dev->pmsuspended = true;
	}
}

int pm_runtime_put_autosuspend(struct device *dev)
{
	dev->pmcnt--;
	check_suspend(dev);
	return 0;
}

int pm_runtime_force_suspend(struct device *dev)
{
	if (!dev->driver || !dev->driver->pm)
		return -ENODEV;
	return dev->driver->pm->runtime_suspend(dev);
}

int pm_runtime_force_resume(struct device *dev)
{
	if (!dev->driver || !dev->driver->pm)
		return -ENODEV;
	return dev->driver->pm->runtime_resume(dev);
}

void pm_runtime_allow(struct device *dev)
{
	dev->pmallow = 1;
	check_suspend(dev);
}

void pm_runtime_forbid(struct device *dev)
{
	dev->pmallow = 0;
	check_resume(dev);
}
