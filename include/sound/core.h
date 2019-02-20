#ifndef __SOUND_CORE_H
#define __SOUND_CORE_H

#include "wrapper.h"
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/device.h>

enum {
	SND_PR_ALWAYS,
	SND_PR_DEBUG,
	SND_PR_VERBOSE,
};

#define snd_printk	hda_log_printk
#define snd_printd	hda_log_printk
#define snd_printdd	hda_log_printk
#define _snd_printd(level, fmt, args...) hda_log_printk(fmt, ##args)

#define snd_assert(x,y)
static inline int _snd_BUG_ON(int cond, const char *func, int line)
{
	if (cond)
		printk(KERN_ERR "XXX BUG in %s:%d\n", func, line);
	return cond;
}
#define snd_BUG_ON(cond)	_snd_BUG_ON(cond, __func__, __LINE__)
#define snd_BUG()		snd_BUG_ON(1)

#include "asound.h"
#include "codec_config.h"

struct snd_info_entry;

extern struct class *sound_class;

static inline void snd_info_free_entry(struct snd_info_entry *entry) {}

struct snd_card {
	int number;			/* number of soundcard (index to
								snd_cards) */

	char id[16];			/* id string of this card */
	char driver[16];		/* driver name */
	char shortname[32];		/* short name of this soundcard */
	char longname[80];		/* name of this soundcard */
	char mixername[80];		/* mixer name */
	char components[80];		/* card components delimited with
								space */
	struct snd_info_entry *proc;

	int shutdown;
	struct list_head ctl_files;
	spinlock_t files_lock;

	struct device *dev;
	struct device card_dev;

	bool registered;
};

struct snd_device {
	void *device_data;
};

typedef unsigned long snd_pcm_uframes_t;
typedef signed long snd_pcm_sframes_t;

typedef int snd_device_type_t;
#define	SNDRV_DEV_TOPLEVEL	(0)
#define	SNDRV_DEV_CONTROL	(1)
#define	SNDRV_DEV_LOWLEVEL_PRE	(2)
#define	SNDRV_DEV_LOWLEVEL_NORMAL (0x1000)
#define	SNDRV_DEV_PCM		(0x1001)
#define	SNDRV_DEV_RAWMIDI	(0x1002)
#define	SNDRV_DEV_TIMER		(0x1003)
#define	SNDRV_DEV_SEQUENCER	(0x1004)
#define	SNDRV_DEV_HWDEP		(0x1005)
#define	SNDRV_DEV_INFO		(0x1006)
#define	SNDRV_DEV_BUS		(0x1007)
#define	SNDRV_DEV_CODEC		(0x1008)
#define	SNDRV_DEV_JACK		(0x1009)
#define	SNDRV_DEV_LOWLEVEL	(0x2000)

struct snd_device_ops {
	int (*dev_free)(struct snd_device *dev);
	int (*dev_register)(struct snd_device *dev);
	int (*dev_disconnect)(struct snd_device *dev);
};

static inline
int snd_device_new(struct snd_card *card, snd_device_type_t type,
		   void *device_data, struct snd_device_ops *ops)
{
	return 0;
}

static inline
int snd_device_free(struct snd_card *card, void *device_data)
{
	return 0;
}

static inline
int snd_device_disconnect(struct snd_card *card, void *device_data)
{
	return 0;
}

static inline
int snd_card_register(struct snd_card *card)
{
	card->registered = true;
	return 0;
}

#define snd_power_get_state(card)       ({ (void)(card); SNDRV_CTL_POWER_D0; })

#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/pci.h>

/* PCI quirk list helper */
#ifdef NEW_QUIRK_LIST

struct snd_pci_quirk {
	unsigned short subvendor;	/* PCI subvendor ID */
	unsigned short subdevice;	/* PCI subdevice ID */
	unsigned short subdevice_mask;	/* bitmask to match */
	int value;			/* value */
	const char *name;		/* name of the device (optional) */
};

#define _SND_PCI_QUIRK_ID_MASK(vend, mask, dev)	\
	.subvendor = (vend), .subdevice = (dev), .subdevice_mask = (mask)
#define _SND_PCI_QUIRK_ID(vend, dev) \
	_SND_PCI_QUIRK_ID_MASK(vend, 0xffff, dev)
#define SND_PCI_QUIRK_ID(vend,dev) {_SND_PCI_QUIRK_ID(vend, dev)}
#define SND_PCI_QUIRK(vend,dev,xname,val) \
	{_SND_PCI_QUIRK_ID(vend, dev), .value = (val), .name = (xname)}
#define SND_PCI_QUIRK_VENDOR(vend, xname, val)			\
	{_SND_PCI_QUIRK_ID_MASK(vend, 0, 0), .value = (val), .name = (xname)}
#define SND_PCI_QUIRK_MASK(vend, mask, dev, xname, val)			\
	{_SND_PCI_QUIRK_ID_MASK(vend, mask, dev),			\
			.value = (val), .name = (xname)}

#else /* !NEW_QUIRK_LIST */

struct snd_pci_quirk {
	unsigned short subvendor;	/* PCI subvendor ID */
	unsigned short subdevice;	/* PCI subdevice ID */
	int value;			/* value */
	const char *name;		/* name of the device (optional) */
};

#define _SND_PCI_QUIRK_ID(vend,dev) \
	.subvendor = (vend), .subdevice = (dev)
#define SND_PCI_QUIRK_ID(vend,dev) {_SND_PCI_QUIRK_ID(vend, dev)}
#define SND_PCI_QUIRK(vend,dev,xname,val) \
	{_SND_PCI_QUIRK_ID(vend, dev), .value = (val), .name = (xname)}

#endif /* NEW_QUIRK_LIST */

const struct snd_pci_quirk *
snd_pci_quirk_lookup(struct pci_dev *pci, const struct snd_pci_quirk *list);

#define snd_component_add(x,y)

/*
 * for old kernels
 */
#define down(x)	mutex_lock(x)
#define up(x)	mutex_unlock(x)
struct semaphore { int x; };
#define init_MUTEX(x) do {} while (0)

#endif /* __SOUND_CORE_H */
