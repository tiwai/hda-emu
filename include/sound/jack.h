#ifndef __SOUND_JACK_H
#define __SOUND_JACK_H

#include <sound/core.h>

/**
 * Jack types which can be reported.  These values are used as a
 * bitmask.
 */
enum snd_jack_types {
	SND_JACK_HEADPHONE	= 0x0001,
	SND_JACK_MICROPHONE	= 0x0002,
	SND_JACK_HEADSET	= SND_JACK_HEADPHONE | SND_JACK_MICROPHONE,
	SND_JACK_LINEOUT	= 0x0004,
	SND_JACK_MECHANICAL	= 0x0008, /* If detected separately */
	SND_JACK_VIDEOOUT	= 0x0010,
	SND_JACK_AVOUT		= SND_JACK_LINEOUT | SND_JACK_VIDEOOUT,
	SND_JACK_LINEIN		= 0x0020,

	/* Kept separate from switches to facilitate implementation */
	SND_JACK_BTN_0		= 0x4000,
	SND_JACK_BTN_1		= 0x2000,
	SND_JACK_BTN_2		= 0x1000,
	SND_JACK_BTN_3		= 0x0800,
	SND_JACK_BTN_4		= 0x0400,
	SND_JACK_BTN_5		= 0x0200,
};

struct snd_jack {
	/*struct input_dev *input_dev;*/
	struct snd_card *card;
	int registered;
	int type;
	const char *id;
	char name[100];
	unsigned int key[6];   /* Keep in sync with definitions above */
	void *private_data;
	void (*private_free)(struct snd_jack *);
#ifdef NEW_JACK_API
	struct snd_kcontrol *kctl; /* XXX assuming 1:1 mapping */
#endif
};

struct device;

#ifdef NEW_JACK_API
int snd_jack_new(struct snd_card *card, const char *id, int type,
		 struct snd_jack **jack, bool initial_kctl, bool phantom_jack);
#else
int snd_jack_new(struct snd_card *card, const char *id, int type,
		 struct snd_jack **jack);
#endif

void snd_jack_set_parent(struct snd_jack *jack, struct device *parent);

void snd_jack_report(struct snd_jack *jack, int status);

#endif
