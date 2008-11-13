/*
 */

#ifndef __SOUND_INFO_H
#define __SOUND_INFO_H

#include <stdio.h>
#include <stdarg.h>

struct snd_ctl_elem_info;
struct snd_info_entry;
struct snd_info_buffer {
	FILE *fp;
};

typedef void (*snd_info_proc_t)(struct snd_info_entry *entry,
				struct snd_info_buffer *buf);

struct snd_info_entry {
	void *private_data;
	snd_info_proc_t func;
};

static inline void snd_iprintf(struct snd_info_buffer *buf,
			       const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(buf->fp, fmt, ap);
	va_end(ap);
}

static inline int snd_card_proc_new(struct snd_card *card, const char *name,
				    struct snd_info_entry **entryp)
{
	*entryp = malloc(sizeof(**entryp));
	if (!*entryp)
		return -ENOMEM;
	card->proc = *entryp;
	return 0;
}

static void snd_info_set_text_ops(struct snd_info_entry *entry,
				  void *private_data,
				  snd_info_proc_t func)
{
	entry->private_data = private_data;
	entry->func = func;
}

#endif /* __SOUND_INFO_H */
