/*
 */

#ifndef __SOUND_INFO_H
#define __SOUND_INFO_H

#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>

struct snd_ctl_elem_info;
struct snd_info_entry;
struct snd_info_buffer;

struct snd_info_entry_text {
	void (*read)(struct snd_info_entry *entry,
		     struct snd_info_buffer *buffer);
	void (*write)(struct snd_info_entry *entry,
		      struct snd_info_buffer *buffer);
};

typedef void (*snd_info_proc_t)(struct snd_info_entry *entry,
				struct snd_info_buffer *buf);

struct snd_info_entry {
	unsigned int mode;
	union {
		struct snd_info_entry_text text;
		/* struct snd_info_entry_ops *ops; */
	} c;
	void *private_data;
	snd_info_proc_t func;
};

extern void snd_iprintf(struct snd_info_buffer *buf, const char *fmt, ...);

static inline int snd_card_proc_new(struct snd_card *card, const char *name,
				    struct snd_info_entry **entryp)
{
	*entryp = malloc(sizeof(**entryp));
	if (!*entryp)
		return -ENOMEM;
	card->proc = *entryp;
	return 0;
}

static inline void snd_info_set_text_ops(struct snd_info_entry *entry,
				  void *private_data,
				  snd_info_proc_t func)
{
	entry->private_data = private_data;
	entry->func = func;
}

static inline int snd_info_get_line(struct snd_info_buffer *buffer, char *line,
			     int len)
{
	return 1; /* EOF */
}

static inline
int snd_card_rw_proc_new(struct snd_card *card, const char *name,
			 void *private_data,
			 void (*read)(struct snd_info_entry *,
				      struct snd_info_buffer *),
			 void (*write)(struct snd_info_entry *entry,
				       struct snd_info_buffer *buffer))
{
	struct snd_info_entry *entry;
	int err;

	err = snd_card_proc_new(card, name, &entry);
	if (err < 0)
		return err;
	snd_info_set_text_ops(entry, private_data, read);
	if (write) {
		entry->mode |= 0200;
		entry->c.text.write = write;
	}
	return 0;
}

static inline int
snd_card_ro_proc_new(struct snd_card *card, const char *name,
		     void *private_data,
		     void (*read)(struct snd_info_entry *,
				  struct snd_info_buffer *))
{
	return snd_card_rw_proc_new(card, name, private_data, read, NULL);
}

#endif /* __SOUND_INFO_H */
