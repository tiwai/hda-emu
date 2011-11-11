#ifndef __SOUND_CONTROL_H
#define __SOUND_CONTROL_H

#define snd_kcontrol_chip(kcontrol) ((kcontrol)->private_data)
#define _snd_kcontrol_chip(kcontrol) ((kcontrol)->private_data)

struct snd_kcontrol;
typedef int (snd_kcontrol_info_t) (struct snd_kcontrol * kcontrol, struct snd_ctl_elem_info * uinfo);
typedef int (snd_kcontrol_get_t) (struct snd_kcontrol * kcontrol, struct snd_ctl_elem_value * ucontrol);
typedef int (snd_kcontrol_put_t) (struct snd_kcontrol * kcontrol, struct snd_ctl_elem_value * ucontrol);
typedef int (snd_kcontrol_tlv_rw_t)(struct snd_kcontrol *kcontrol,
				    int op_flag, /* 0=read,1=write,-1=command */
				    unsigned int size,
				    unsigned int __user *tlv);

struct snd_kcontrol_new {
	snd_ctl_elem_iface_t iface;	/* interface identifier */
	unsigned int device;		/* device/client number */
	unsigned int subdevice;		/* subdevice (substream) number */
	unsigned char *name;		/* ASCII name of item */
	unsigned int index;		/* index of item */
	unsigned int access;		/* access rights */
	unsigned int count;		/* count of same elements */
	snd_kcontrol_info_t *info;
	snd_kcontrol_get_t *get;
	snd_kcontrol_put_t *put;
	union {
		snd_kcontrol_tlv_rw_t *c;
		const unsigned int *p;
	} tlv;
	unsigned long private_value;
};

struct snd_kcontrol {
	struct list_head list;		/* list of controls */
	struct snd_ctl_elem_id id;
	unsigned int count;		/* count of same elements */
	snd_kcontrol_info_t *info;
	snd_kcontrol_get_t *get;
	snd_kcontrol_put_t *put;
	union {
		snd_kcontrol_tlv_rw_t *c;
		const unsigned int *p;
	} tlv;
	unsigned long private_value;
	void *private_data;
	void (*private_free)(struct snd_kcontrol *kcontrol);
	struct {
		unsigned int access;
	} vd[1];
};

#define snd_kcontrol(n) list_entry(n, struct snd_kcontrol, list)

void snd_ctl_notify(struct snd_card * card, unsigned int mask, struct snd_ctl_elem_id * id);

struct snd_kcontrol *snd_ctl_new1(const struct snd_kcontrol_new * kcontrolnew, void * private_data);
void snd_ctl_free_one(struct snd_kcontrol * kcontrol);
int snd_ctl_add(struct snd_card * card, struct snd_kcontrol * kcontrol);
int snd_ctl_remove(struct snd_card * card, struct snd_kcontrol * kcontrol);
int snd_ctl_remove_id(struct snd_card * card, struct snd_ctl_elem_id *id);
int snd_ctl_rename_id(struct snd_card * card, struct snd_ctl_elem_id *src_id, struct snd_ctl_elem_id *dst_id);
int snd_ctl_activate_id(struct snd_card *card, struct snd_ctl_elem_id *id,
			int active);
struct snd_kcontrol *snd_ctl_find_numid(struct snd_card * card, unsigned int numid);
struct snd_kcontrol *snd_ctl_find_id(struct snd_card * card, struct snd_ctl_elem_id *id);

static inline unsigned int snd_ctl_get_ioffnum(struct snd_kcontrol *kctl, struct snd_ctl_elem_id *id)
{
	return id->numid - kctl->id.numid;
}

static inline unsigned int snd_ctl_get_ioffidx(struct snd_kcontrol *kctl, struct snd_ctl_elem_id *id)
{
	return id->index - kctl->id.index;
}

static inline unsigned int snd_ctl_get_ioff(struct snd_kcontrol *kctl, struct snd_ctl_elem_id *id)
{
	if (id->numid) {
		return snd_ctl_get_ioffnum(kctl, id);
	} else {
		return snd_ctl_get_ioffidx(kctl, id);
	}
}

static inline struct snd_ctl_elem_id *snd_ctl_build_ioff(struct snd_ctl_elem_id *dst_id,
						    struct snd_kcontrol *src_kctl,
						    unsigned int offset)
{
	*dst_id = src_kctl->id;
	dst_id->index += offset;
	dst_id->numid += offset;
	return dst_id;
}

/*
 * Frequently used control callbacks
 */
int snd_ctl_boolean_mono_info(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_info *uinfo);
int snd_ctl_boolean_stereo_info(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_info *uinfo);

/*
 * virtual master control
 */
struct snd_kcontrol *snd_ctl_make_virtual_master(char *name,
						 const unsigned int *tlv);
int _snd_ctl_add_slave(struct snd_kcontrol *master, struct snd_kcontrol *slave,
		       unsigned int flags);
/* optional flags for slave */
#define SND_CTL_SLAVE_NEED_UPDATE	(1 << 0)

/**
 * snd_ctl_add_slave - Add a virtual slave control
 * @master: vmaster element
 * @slave: slave element to add
 *
 * Add a virtual slave control to the given master element created via
 * snd_ctl_create_virtual_master() beforehand.
 * Returns zero if successful or a negative error code.
 *
 * All slaves must be the same type (returning the same information
 * via info callback).  The function doesn't check it, so it's your
 * responsibility.
 *
 * Also, some additional limitations:
 * at most two channels,
 * logarithmic volume control (dB level) thus no linear volume,
 * master can only attenuate the volume without gain
 */
static inline int
snd_ctl_add_slave(struct snd_kcontrol *master, struct snd_kcontrol *slave)
{
	return _snd_ctl_add_slave(master, slave, 0);
}

/**
 * snd_ctl_add_slave_uncached - Add a virtual slave control
 * @master: vmaster element
 * @slave: slave element to add
 *
 * Add a virtual slave control to the given master.
 * Unlike snd_ctl_add_slave(), the element added via this function
 * is supposed to have volatile values, and get callback is called
 * at each time quried from the master.
 *
 * When the control peeks the hardware values directly and the value
 * can be changed by other means than the put callback of the element,
 * this function should be used to keep the value always up-to-date.
 */
static inline int
snd_ctl_add_slave_uncached(struct snd_kcontrol *master,
			   struct snd_kcontrol *slave)
{
	return _snd_ctl_add_slave(master, slave, SND_CTL_SLAVE_NEED_UPDATE);
}

/*
 * Helper functions for jack-detection controls
 */
struct snd_kcontrol *
snd_kctl_jack_new(const char *name, int idx, void *private_data);
void snd_kctl_jack_report(struct snd_card *card,
			  struct snd_kcontrol *kctl, bool status);

/*
 */

#define SNDRV_CTL_NAME_NONE				""
#define SNDRV_CTL_NAME_PLAYBACK				"Playback "
#define SNDRV_CTL_NAME_CAPTURE				"Capture "

#define SNDRV_CTL_NAME_IEC958_NONE			""
#define SNDRV_CTL_NAME_IEC958_SWITCH			"Switch"
#define SNDRV_CTL_NAME_IEC958_VOLUME			"Volume"
#define SNDRV_CTL_NAME_IEC958_DEFAULT			"Default"
#define SNDRV_CTL_NAME_IEC958_MASK			"Mask"
#define SNDRV_CTL_NAME_IEC958_CON_MASK			"Con Mask"
#define SNDRV_CTL_NAME_IEC958_PRO_MASK			"Pro Mask"
#define SNDRV_CTL_NAME_IEC958_PCM_STREAM		"PCM Stream"
#define SNDRV_CTL_NAME_IEC958(expl,direction,what)	"IEC958 " expl SNDRV_CTL_NAME_##direction SNDRV_CTL_NAME_IEC958_##what

#define snd_ctl_elem_read(card,ctl) 0
#define snd_ctl_elem_write(card,file,ctl) 0

#endif /* __SOUND_CONTROL_H */
