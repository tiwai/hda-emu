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
	int type;
};

static inline int snd_jack_new(struct snd_card *card, const char *id, int type,
			       struct snd_jack **jack)
{
	return 0;
}

struct device;

static inline void snd_jack_set_parent(struct snd_jack *jack,
				       struct device *parent)
{
}

static inline void snd_jack_report(struct snd_jack *jack, int status)
{
}

#endif
