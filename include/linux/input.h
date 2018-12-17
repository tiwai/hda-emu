#ifndef __LINUX_INPUT_H
#define __LINUX_INPUT_H

#include "wrapper.h"
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/slab.h>

struct file;

struct input_id {
	__u16 bustype;
	__u16 vendor;
	__u16 product;
	__u16 version;
};

struct input_dev {
	const char *name;
	const char *phys;
	const char *uniq;
	struct input_id id;

	unsigned long evbit[8];
	unsigned long sndbit[8];
	unsigned long keybit[0x300 / (sizeof(long) * 8)];
	/* ... */

        unsigned int keycodemax;
        unsigned int keycodesize;
        void *keycode;
	/* ... */

	int (*open)(struct input_dev *dev);
	void (*close)(struct input_dev *dev);
	int (*flush)(struct input_dev *dev, struct file *file);
	int (*event)(struct input_dev *dev, unsigned int type, unsigned int code, int value);

	spinlock_t event_lock;
	struct mutex mutex;

	struct device dev;
};

static inline struct input_dev *input_allocate_device(void)
{
	return calloc(1, sizeof(struct input_dev));
}

static inline void input_free_device(struct input_dev *dev)
{
	free(dev);
}

#define input_set_drvdata(idev, data) ((idev)->dev.driver_data = data)
#define input_get_drvdata(idev)	((idev)->dev.driver_data)
#define input_register_device(dev) 0
#define input_unregister_device(dev) input_free_device(dev)
static inline void input_report_key(struct input_dev *dev, unsigned int code, int value) {}
static inline void input_sync(struct input_dev *dev) {}

#define BUS_PCI			0x01

#define SND_CLICK		0x00
#define SND_BELL		0x01
#define SND_TONE		0x02
#define SND_MAX			0x07
#define SND_CNT			(SND_MAX+1)

#define EV_SYN			0x00
#define EV_KEY			0x01
#define EV_REL			0x02
#define EV_ABS			0x03
#define EV_MSC			0x04
#define EV_SW			0x05
#define EV_LED			0x11
#define EV_SND			0x12
#define EV_REP			0x14
#define EV_FF			0x15
#define EV_PWR			0x16
#define EV_FF_STATUS		0x17
#define EV_MAX			0x1f
#define EV_CNT			(EV_MAX+1)

#define KEY_MUTE		113
#define KEY_VOLUMEDOWN		114
#define KEY_VOLUMEUP		115
#define KEY_PLAYPAUSE		164
#define KEY_MICMUTE		248	/* Mute / unmute the microphone */
#define KEY_VOICECOMMAND		0x246	/* Listening Voice Command */

#endif /* __LINUX_INPUT_H */
