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
};

struct snd_jack {
	/*struct input_dev *input_dev;*/
	int registered;
	int type;
	const char *id;
	char name[100];
	unsigned int key[6];   /* Keep in sync with definitions above */
	void *private_data;
	void (*private_free)(struct snd_jack *);
};

struct device;

int snd_jack_new(struct snd_card *card, const char *id, int type,
		 struct snd_jack **jack);

void snd_jack_set_parent(struct snd_jack *jack, struct device *parent);

void snd_jack_report(struct snd_jack *jack, int status);

#endif
