/*
 * hda-emu - simple HD-audio codec emulator for debugging snd-hda-intel driver
 *
 * Simple cut-out version of ALSA control API handlers
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

#include <sound/core.h>
#include <sound/control.h>
#include "hda-log.h"

static int snd_ctl_list_elems;
LIST_HEAD(snd_ctl_list_head); /* exported */

struct snd_kcontrol *snd_ctl_new1(const struct snd_kcontrol_new *knew,
				  void *private_data)
{
	struct snd_kcontrol *kctl;
	kctl = calloc(1, sizeof(*kctl));
	if (!kctl)
		return NULL;
	kctl->id.iface = knew->iface;
	kctl->id.device = knew->device;
	kctl->id.subdevice = knew->subdevice;
	kctl->id.index = knew->index;
	if (knew->name)
		strcpy(kctl->id.name, knew->name);
	kctl->count = knew->count ? knew->count : 1;
	kctl->vd[0].access = knew->access ?
		knew->access : SNDRV_CTL_ELEM_ACCESS_READWRITE;
	kctl->info = knew->info;
	kctl->get = knew->get;
	kctl->put = knew->put;
	memcpy(&kctl->tlv, &knew->tlv, sizeof(kctl->tlv));
	kctl->private_value = knew->private_value;
	kctl->private_data = private_data;
	INIT_LIST_HEAD(&kctl->list);
	return kctl;
}

void snd_ctl_free_one(struct snd_kcontrol *kctl)
{
	if (kctl) {
		if (kctl->private_free)
			kctl->private_free(kctl);
		free(kctl);
	}
}

int snd_ctl_add(struct snd_card *card, struct snd_kcontrol *kctl)
{
	if (snd_ctl_find_id(card, &kctl->id)) {
		hda_log(HDA_LOG_ERR, "Control element %s:%d already exists!\n",
			kctl->id.name, kctl->id.index);
		return -EBUSY;
	}
	kctl->id.numid = snd_ctl_list_elems + 1;
	snd_ctl_list_elems += kctl->count;
	list_add_tail(&kctl->list, &snd_ctl_list_head);
	hda_log(HDA_LOG_INFO, "CTRL: add: %s:%d\n",
		kctl->id.name, kctl->id.index);
	return 0;
}

int snd_ctl_remove(struct snd_card *card, struct snd_kcontrol *kctl)
{
	hda_log(HDA_LOG_INFO, "CTRL: remove: %s:%d\n",
		kctl->id.name, kctl->id.index);
	list_del(&kctl->list);
	snd_ctl_free_one(kctl);
	return 0;
}

int snd_ctl_remove_id(struct snd_card *card, struct snd_ctl_elem_id *id)
{
	struct snd_kcontrol *kctl = snd_ctl_find_id(card, id);
	if (!kctl)
		return -ENODEV;
	return snd_ctl_remove(card, kctl);
}

int snd_ctl_rename_id(struct snd_card *card, struct snd_ctl_elem_id *src_id,
		      struct snd_ctl_elem_id *dst_id)
{
	struct snd_kcontrol *kctl = snd_ctl_find_id(card, src_id);
	if (!kctl)
		return -ENODEV;
	kctl->id = *dst_id;
	return 0;
}

struct snd_kcontrol *snd_ctl_find_numid(struct snd_card *card, unsigned int numid)
{
	struct snd_kcontrol *kctl;

	list_for_each_entry(kctl, &snd_ctl_list_head, list) {
		if (kctl->id.numid <= numid && kctl->id.numid + kctl->count > numid)
			return kctl;
	}
	return NULL;
}

struct snd_kcontrol *snd_ctl_find_id(struct snd_card *card,
				     struct snd_ctl_elem_id *id)
{
	struct snd_kcontrol *kctl;

	list_for_each_entry(kctl, &snd_ctl_list_head, list) {
		if (kctl->id.iface == id->iface &&
		    kctl->id.device == id->device &&
		    kctl->id.subdevice == id->subdevice &&
		    kctl->id.index == id->index &&
		    !strcmp(kctl->id.name, id->name))
			return kctl;
	}
	return NULL;
}

#ifndef HAVE_BOOLEAN_INFO
int snd_ctl_boolean_mono_info(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}

int snd_ctl_boolean_stereo_info(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
	uinfo->count = 2;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}
#endif

int snd_ctl_enum_info(struct snd_ctl_elem_info *info, unsigned int channels,
		      unsigned int items, const char *const names[])
{
	info->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
	info->count = channels;
	info->value.enumerated.items = items;
	if (!items)
		return 0;
	if (info->value.enumerated.item >= items)
		info->value.enumerated.item = items - 1;
	if (strlen(names[info->value.enumerated.item]) >= sizeof(info->value.enumerated.name))
		hda_log(HDA_LOG_WARN, "ALSA: too long item name '%s'\n", names[info->value.enumerated.item]);
	strlcpy(info->value.enumerated.name,
		names[info->value.enumerated.item],
		sizeof(info->value.enumerated.name));
	return 0;
}

int snd_ctl_activate_id(struct snd_card *card, struct snd_ctl_elem_id *id,
			int active)
{
	struct snd_kcontrol *kctl;
	unsigned int index_offset;
	int ret;

	kctl = snd_ctl_find_id(card, id);
	if (!kctl)
		return -ENOENT;
	index_offset = snd_ctl_get_ioff(kctl, &kctl->id);
	ret = 0;
	if (active) {
		if (!(kctl->vd[0].access & SNDRV_CTL_ELEM_ACCESS_INACTIVE))
			goto unlock;
		kctl->vd[0].access &= ~SNDRV_CTL_ELEM_ACCESS_INACTIVE;
	} else {
		if (kctl->vd[0].access & SNDRV_CTL_ELEM_ACCESS_INACTIVE)
			goto unlock;
		kctl->vd[0].access |= SNDRV_CTL_ELEM_ACCESS_INACTIVE;
	}
	snd_ctl_build_ioff(id, kctl, index_offset);
	ret = 1;
 unlock:
	if (ret > 0) {
		hda_log(HDA_LOG_INFO, "Control element %s:%d active changed to %s\n",
			kctl->id.name, kctl->id.index,
			(kctl->vd[0].access & SNDRV_CTL_ELEM_ACCESS_INACTIVE) ? "inactive" : "active");
	}
	return ret;
}

void snd_ctl_notify(struct snd_card *card, unsigned int mask,
		    struct snd_ctl_elem_id *id)
{
	struct snd_kcontrol *kctl;
	
	kctl = snd_ctl_find_id(card, id);
	if (!kctl)
		return;
	hda_log(HDA_LOG_INFO, "CTL Notify: %s:%d, mask=%d\n",
		kctl->id.name, kctl->id.index, mask);
}
