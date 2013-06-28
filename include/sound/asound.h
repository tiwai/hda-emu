struct snd_ctl_card_info {
	int card;			/* card number */
	int pad;			/* reserved for future (was type) */
	unsigned char id[16];		/* ID of card (user selectable) */
	unsigned char driver[16];	/* Driver name */
	unsigned char name[32];		/* Short name of soundcard */
	unsigned char longname[80];	/* name + info text about soundcard */
	unsigned char reserved_[16];	/* reserved for future (was ID of mixer) */
	unsigned char mixername[80];	/* visual mixer identification */
	unsigned char components[80];	/* card components / fine identification, delimited with one space (AC97 etc..) */
	unsigned char reserved[48];	/* reserved for future */
};

typedef int snd_ctl_elem_type_t;
#define	SNDRV_CTL_ELEM_TYPE_NONE	(0) /* invalid */
#define	SNDRV_CTL_ELEM_TYPE_BOOLEAN	(1) /* boolean type */
#define	SNDRV_CTL_ELEM_TYPE_INTEGER	(2) /* integer type */
#define	SNDRV_CTL_ELEM_TYPE_ENUMERATED	(3) /* enumerated type */
#define	SNDRV_CTL_ELEM_TYPE_BYTES	(4) /* byte array */
#define	SNDRV_CTL_ELEM_TYPE_IEC958	(5) /* IEC958 (S/PDIF) setup */
#define	SNDRV_CTL_ELEM_TYPE_INTEGER64	(6) /* 64-bit integer type */
#define	SNDRV_CTL_ELEM_TYPE_LAST	SNDRV_CTL_ELEM_TYPE_INTEGER64

typedef int snd_ctl_elem_iface_t;
#define	SNDRV_CTL_ELEM_IFACE_CARD	(0) /* global control */
#define	SNDRV_CTL_ELEM_IFACE_HWDEP	(1) /* hardware dependent device */
#define	SNDRV_CTL_ELEM_IFACE_MIXER	(2) /* virtual mixer device */
#define	SNDRV_CTL_ELEM_IFACE_PCM	(3) /* PCM device */
#define	SNDRV_CTL_ELEM_IFACE_RAWMIDI	(4) /* RawMidi device */
#define	SNDRV_CTL_ELEM_IFACE_TIMER	(5) /* timer device */
#define	SNDRV_CTL_ELEM_IFACE_SEQUENCER	(6) /* sequencer client */
#define	SNDRV_CTL_ELEM_IFACE_LAST	SNDRV_CTL_ELEM_IFACE_SEQUENCER

#define SNDRV_CTL_ELEM_ACCESS_READ		(1<<0)
#define SNDRV_CTL_ELEM_ACCESS_WRITE		(1<<1)
#define SNDRV_CTL_ELEM_ACCESS_READWRITE		(SNDRV_CTL_ELEM_ACCESS_READ|SNDRV_CTL_ELEM_ACCESS_WRITE)
#define SNDRV_CTL_ELEM_ACCESS_VOLATILE		(1<<2)	/* control value may be changed without a notification */
#define SNDRV_CTL_ELEM_ACCESS_TIMESTAMP		(1<<3)	/* when was control changed */
#define SNDRV_CTL_ELEM_ACCESS_TLV_READ		(1<<4)	/* TLV read is possible */
#define SNDRV_CTL_ELEM_ACCESS_TLV_WRITE		(1<<5)	/* TLV write is possible */
#define SNDRV_CTL_ELEM_ACCESS_TLV_READWRITE	(SNDRV_CTL_ELEM_ACCESS_TLV_READ|SNDRV_CTL_ELEM_ACCESS_TLV_WRITE)
#define SNDRV_CTL_ELEM_ACCESS_TLV_COMMAND	(1<<6)	/* TLV command is possible */
#define SNDRV_CTL_ELEM_ACCESS_INACTIVE		(1<<8)	/* control does actually nothing, but may be updated */
#define SNDRV_CTL_ELEM_ACCESS_LOCK		(1<<9)	/* write lock */
#define SNDRV_CTL_ELEM_ACCESS_OWNER		(1<<10)	/* write lock owner */
#define SNDRV_CTL_ELEM_ACCESS_TLV_CALLBACK	(1<<28)	/* kernel use a TLV callback */ 
#define SNDRV_CTL_ELEM_ACCESS_USER		(1<<29) /* user space element */
/* bits 30 and 31 are obsoleted (for indirect access) */

/* for further details see the ACPI and PCI power management specification */
#define SNDRV_CTL_POWER_D0		0x0000	/* full On */
#define SNDRV_CTL_POWER_D1		0x0100	/* partial On */
#define SNDRV_CTL_POWER_D2		0x0200	/* partial On */
#define SNDRV_CTL_POWER_D3		0x0300	/* Off */
#define SNDRV_CTL_POWER_D3hot		(SNDRV_CTL_POWER_D3|0x0000)	/* Off, with power */
#define SNDRV_CTL_POWER_D3cold		(SNDRV_CTL_POWER_D3|0x0001)	/* Off, without power */

#define SNDRV_CTL_ELEM_ID_NAME_MAXLEN	44

struct snd_ctl_elem_id {
	unsigned int numid;		/* numeric identifier, zero = invalid */
	snd_ctl_elem_iface_t iface;	/* interface identifier */
	unsigned int device;		/* device/client number */
	unsigned int subdevice;		/* subdevice (substream) number */
        unsigned char name[44];		/* ASCII name of item */
	unsigned int index;		/* index of item */
};

struct snd_ctl_elem_list {
	unsigned int offset;		/* W: first element ID to get */
	unsigned int space;		/* W: count of element IDs to get */
	unsigned int used;		/* R: count of element IDs set */
	unsigned int count;		/* R: count of all elements */
	struct snd_ctl_elem_id *pids; /* R: IDs */
	unsigned char reserved[50];
};

struct snd_ctl_elem_info {
	struct snd_ctl_elem_id id;	/* W: element ID */
	snd_ctl_elem_type_t type;	/* R: value type - SNDRV_CTL_ELEM_TYPE_* */
	unsigned int access;		/* R: value access (bitmask) - SNDRV_CTL_ELEM_ACCESS_* */
	unsigned int count;		/* count of values */
	pid_t owner;			/* owner's PID of this control */
	union {
		struct {
			long min;		/* R: minimum value */
			long max;		/* R: maximum value */
			long step;		/* R: step (0 variable) */
		} integer;
		struct {
			long long min;		/* R: minimum value */
			long long max;		/* R: maximum value */
			long long step;		/* R: step (0 variable) */
		} integer64;
		struct {
			unsigned int items;	/* R: number of items */
			unsigned int item;	/* W: item number */
			char name[64];		/* R: value name */
		} enumerated;
		unsigned char reserved[128];
	} value;
	union {
		unsigned short d[4];		/* dimensions */
		unsigned short *d_ptr;		/* indirect - obsoleted */
	} dimen;
	unsigned char reserved[64-4*sizeof(unsigned short)];
};

struct snd_aes_iec958 {
	unsigned char status[24];	/* AES/IEC958 channel status bits */
	unsigned char subcode[147];	/* AES/IEC958 subcode bits */
	unsigned char pad;		/* nothing */
	unsigned char dig_subframe[4];	/* AES/IEC958 subframe bits */
};

struct snd_ctl_elem_value {
	struct snd_ctl_elem_id id;	/* W: element ID */
	unsigned int indirect: 1;	/* W: indirect access - obsoleted */
        union {
		union {
			long value[128];
			long *value_ptr;	/* obsoleted */
		} integer;
		union {
			long long value[64];
			long long *value_ptr;	/* obsoleted */
		} integer64;
		union {
			unsigned int item[128];
			unsigned int *item_ptr;	/* obsoleted */
		} enumerated;
		union {
			unsigned char data[512];
			unsigned char *data_ptr;	/* obsoleted */
		} bytes;
		struct snd_aes_iec958 iec958;
        } value;                /* RO */
	struct timespec tstamp;
        unsigned char reserved[128-sizeof(struct timespec)];
};

struct snd_ctl_tlv {
        unsigned int numid;	/* control element numeric identification */
        unsigned int length;	/* in bytes aligned to 4 */
        unsigned int tlv[0];	/* first TLV */
};

enum sndrv_ctl_event_type {
	SNDRV_CTL_EVENT_ELEM = 0,
	SNDRV_CTL_EVENT_LAST = SNDRV_CTL_EVENT_ELEM,
};

#define SNDRV_CTL_EVENT_MASK_VALUE	(1<<0)	/* element value was changed */
#define SNDRV_CTL_EVENT_MASK_INFO	(1<<1)	/* element info was changed */
#define SNDRV_CTL_EVENT_MASK_ADD	(1<<2)	/* element was added */
#define SNDRV_CTL_EVENT_MASK_TLV	(1<<3)	/* element TLV tree was changed */
#define SNDRV_CTL_EVENT_MASK_REMOVE	(~0U)	/* element was removed */

struct snd_ctl_event {
	int type;	/* event type - SNDRV_CTL_EVENT_* */
	union {
		struct {
			unsigned int mask;
			struct snd_ctl_elem_id id;
		} elem;
		unsigned char data8[60];
	} data;
};

typedef int __bitwise snd_pcm_state_t;
#define	SNDRV_PCM_STATE_OPEN		((snd_pcm_state_t) 0) /* stream is open */
#define	SNDRV_PCM_STATE_SETUP		((snd_pcm_state_t) 1) /* stream has a setup */
#define	SNDRV_PCM_STATE_PREPARED	((snd_pcm_state_t) 2) /* stream is ready to start */
#define	SNDRV_PCM_STATE_RUNNING		((snd_pcm_state_t) 3) /* stream is running */
#define	SNDRV_PCM_STATE_XRUN		((snd_pcm_state_t) 4) /* stream reached an xrun */
#define	SNDRV_PCM_STATE_DRAINING	((snd_pcm_state_t) 5) /* stream is draining */
#define	SNDRV_PCM_STATE_PAUSED		((snd_pcm_state_t) 6) /* stream is paused */
#define	SNDRV_PCM_STATE_SUSPENDED	((snd_pcm_state_t) 7) /* hardware is suspended */
#define	SNDRV_PCM_STATE_DISCONNECTED	((snd_pcm_state_t) 8) /* hardware is disconnected */
#define	SNDRV_PCM_STATE_LAST		SNDRV_PCM_STATE_DISCONNECTED

struct snd_pcm_mmap_status {
	snd_pcm_state_t state;		/* RO: state - SNDRV_PCM_STATE_XXXX */
};


/* channel positions */
enum {
	SNDRV_CHMAP_UNKNOWN = 0,
	SNDRV_CHMAP_NA,		/* N/A, silent */
	SNDRV_CHMAP_MONO,	/* mono stream */
	/* this follows the alsa-lib mixer channel value + 3 */
	SNDRV_CHMAP_FL,		/* front left */
	SNDRV_CHMAP_FR,		/* front right */
	SNDRV_CHMAP_RL,		/* rear left */
	SNDRV_CHMAP_RR,		/* rear right */
	SNDRV_CHMAP_FC,		/* front center */
	SNDRV_CHMAP_LFE,	/* LFE */
	SNDRV_CHMAP_SL,		/* side left */
	SNDRV_CHMAP_SR,		/* side right */
	SNDRV_CHMAP_RC,		/* rear center */
	/* new definitions */
	SNDRV_CHMAP_FLC,	/* front left center */
	SNDRV_CHMAP_FRC,	/* front right center */
	SNDRV_CHMAP_RLC,	/* rear left center */
	SNDRV_CHMAP_RRC,	/* rear right center */
	SNDRV_CHMAP_FLW,	/* front left wide */
	SNDRV_CHMAP_FRW,	/* front right wide */
	SNDRV_CHMAP_FLH,	/* front left high */
	SNDRV_CHMAP_FCH,	/* front center high */
	SNDRV_CHMAP_FRH,	/* front right high */
	SNDRV_CHMAP_TC,		/* top center */
	SNDRV_CHMAP_TFL,	/* top front left */
	SNDRV_CHMAP_TFR,	/* top front right */
	SNDRV_CHMAP_TFC,	/* top front center */
	SNDRV_CHMAP_TRL,	/* top rear left */
	SNDRV_CHMAP_TRR,	/* top rear right */
	SNDRV_CHMAP_TRC,	/* top rear center */
	SNDRV_CHMAP_LAST = SNDRV_CHMAP_TRC,
};

#define SNDRV_CHMAP_POSITION_MASK	0xffff
#define SNDRV_CHMAP_PHASE_INVERSE	(0x01 << 16)
#define SNDRV_CHMAP_DRIVER_SPEC		(0x02 << 16)

