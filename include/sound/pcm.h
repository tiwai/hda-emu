/*
 */
#ifndef __SOUND_PCM_H
#define __SOUND_PCM_H

#define SNDRV_PCM_DEVICES	8

struct snd_pcm;
struct snd_pcm_str;
struct snd_pcm_substream;
struct snd_pcm_runtime;
typedef int snd_pcm_access_t;
typedef int snd_pcm_format_t;
typedef int snd_pcm_subformat_t;

struct snd_pcm_hardware {
	unsigned int info;		/* SNDRV_PCM_INFO_* */
	u64 formats;			/* SNDRV_PCM_FMTBIT_* */
	unsigned int rates;		/* SNDRV_PCM_RATE_* */
	unsigned int rate_min;		/* min rate */
	unsigned int rate_max;		/* max rate */
	unsigned int channels_min;	/* min channels */
	unsigned int channels_max;	/* max channels */
	size_t buffer_bytes_max;	/* max buffer size */
	size_t period_bytes_min;	/* min period size */
	size_t period_bytes_max;	/* max period size */
	unsigned int periods_min;	/* min # of periods */
	unsigned int periods_max;	/* max # of periods */
	size_t fifo_size;		/* fifo size in bytes */
};

struct snd_pcm_runtime {
	struct timespec trigger_tstamp;	/* trigger timestamp */
	bool trigger_tstamp_latched;     /* trigger timestamp latched in low-level driver/hardware */
	snd_pcm_access_t access;	/* access mode */
	snd_pcm_format_t format;	/* SNDRV_PCM_FORMAT_* */
	snd_pcm_subformat_t subformat;	/* subformat */
	unsigned int rate;		/* rate in Hz */
	unsigned int channels;		/* channels */
	snd_pcm_uframes_t period_size;	/* period size */
	unsigned int periods;		/* periods */
	snd_pcm_uframes_t buffer_size;	/* buffer size */
	snd_pcm_uframes_t min_align;	/* Min alignment for the format */
	size_t byte_align;
	unsigned int frame_bits;
	unsigned int sample_bits;
	unsigned int info;
	unsigned int rate_num;
	unsigned int rate_den;
	unsigned int no_period_wakeup: 1;
	/* -- mmap -- */
	struct snd_pcm_mmap_status *status;

	void *private_data;
	void (*private_free)(struct snd_pcm_runtime *runtime);
	struct snd_pcm_hardware hw;
};

struct snd_pcm_substream {
	struct snd_pcm *pcm;
	struct snd_pcm_str *pstr;
	void *private_data;		/* copied from pcm->private_data */
	int number;
	char name[32];			/* substream name */
	int stream;			/* stream (direction) */
	/* -- hardware operations -- */
	struct snd_pcm_ops *ops;
	/* -- runtime information -- */
	struct snd_pcm_runtime *runtime;
	/* -- next substream -- */
	struct snd_pcm_substream *next;
	void *file;
	int ref_count;

};

struct snd_pcm_str {
	/* -- substreams -- */
	unsigned int substream_opened;
	unsigned int substream_count;
	struct snd_pcm_substream *substream;

	struct snd_kcontrol *chmap_kctl; /* channel-mapping controls */

};

struct snd_pcm {
	struct snd_card *card;
	struct list_head list;
	unsigned int device;	/* device number */
	unsigned int info_flags;
	unsigned short dev_class;
	unsigned short dev_subclass;
	char id[64];
	char name[80];
	struct snd_pcm_str streams[2];
	void *private_data;
	void (*private_free) (struct snd_pcm *pcm);
	struct device *dev; /* actual hw device this belongs to */
};

#define SNDRV_PCM_DEFAULT_CON_SPDIF	(IEC958_AES0_CON_EMPHASIS_NONE|\
					 (IEC958_AES1_CON_ORIGINAL<<8)|\
					 (IEC958_AES1_CON_PCM_CODER<<8)|\
					 (IEC958_AES3_CON_FS_48000<<24))

/* If you change this don't forget to change rates[] table in pcm_native.c */
#define SNDRV_PCM_RATE_5512		(1<<0)		/* 5512Hz */
#define SNDRV_PCM_RATE_8000		(1<<1)		/* 8000Hz */
#define SNDRV_PCM_RATE_11025		(1<<2)		/* 11025Hz */
#define SNDRV_PCM_RATE_16000		(1<<3)		/* 16000Hz */
#define SNDRV_PCM_RATE_22050		(1<<4)		/* 22050Hz */
#define SNDRV_PCM_RATE_32000		(1<<5)		/* 32000Hz */
#define SNDRV_PCM_RATE_44100		(1<<6)		/* 44100Hz */
#define SNDRV_PCM_RATE_48000		(1<<7)		/* 48000Hz */
#define SNDRV_PCM_RATE_64000		(1<<8)		/* 64000Hz */
#define SNDRV_PCM_RATE_88200		(1<<9)		/* 88200Hz */
#define SNDRV_PCM_RATE_96000		(1<<10)		/* 96000Hz */
#define SNDRV_PCM_RATE_176400		(1<<11)		/* 176400Hz */
#define SNDRV_PCM_RATE_192000		(1<<12)		/* 192000Hz */

#define SNDRV_PCM_RATE_CONTINUOUS	(1<<30)		/* continuous range */
#define SNDRV_PCM_RATE_KNOT		(1<<31)		/* supports more non-continuos rates */

#define	SNDRV_PCM_FORMAT_S8	(0)
#define	SNDRV_PCM_FORMAT_U8	(1)
#define	SNDRV_PCM_FORMAT_S16_LE	(2)
#define	SNDRV_PCM_FORMAT_S16_BE	(3)
#define	SNDRV_PCM_FORMAT_U16_LE	(4)
#define	SNDRV_PCM_FORMAT_U16_BE	(5)
#define	SNDRV_PCM_FORMAT_S24_LE	(6) /* low three bytes */
#define	SNDRV_PCM_FORMAT_S24_BE	(7) /* low three bytes */
#define	SNDRV_PCM_FORMAT_U24_LE	(8) /* low three bytes */
#define	SNDRV_PCM_FORMAT_U24_BE	(9) /* low three bytes */
#define	SNDRV_PCM_FORMAT_S32_LE	(10)
#define	SNDRV_PCM_FORMAT_S32_BE	(11)
#define	SNDRV_PCM_FORMAT_U32_LE	(12)
#define	SNDRV_PCM_FORMAT_U32_BE	(13)
#define	SNDRV_PCM_FORMAT_FLOAT_LE	(14) /* 4-byte float, IEEE-754 32-bit, range -1.0 to 1.0 */
#define	SNDRV_PCM_FORMAT_FLOAT_BE	(15) /* 4-byte float, IEEE-754 32-bit, range -1.0 to 1.0 */
#define	SNDRV_PCM_FORMAT_FLOAT64_LE	(16) /* 8-byte float, IEEE-754 64-bit, range -1.0 to 1.0 */
#define	SNDRV_PCM_FORMAT_FLOAT64_BE	(17) /* 8-byte float, IEEE-754 64-bit, range -1.0 to 1.0 */
#define	SNDRV_PCM_FORMAT_IEC958_SUBFRAME_LE (18) /* IEC-958 subframe, Little Endian */
#define	SNDRV_PCM_FORMAT_IEC958_SUBFRAME_BE (19) /* IEC-958 subframe, Big Endian */
#define	SNDRV_PCM_FORMAT_MU_LAW		(20)
#define	SNDRV_PCM_FORMAT_A_LAW		(21)
#define	SNDRV_PCM_FORMAT_IMA_ADPCM	(22)
#define	SNDRV_PCM_FORMAT_MPEG		(23)
#define	SNDRV_PCM_FORMAT_GSM		(24)
#define	SNDRV_PCM_FORMAT_SPECIAL	(31)
#define	SNDRV_PCM_FORMAT_S24_3LE	(32)	/* in three bytes */
#define	SNDRV_PCM_FORMAT_S24_3BE	(33)	/* in three bytes */
#define	SNDRV_PCM_FORMAT_U24_3LE	(34)	/* in three bytes */
#define	SNDRV_PCM_FORMAT_U24_3BE	(35)	/* in three bytes */
#define	SNDRV_PCM_FORMAT_S20_3LE	(36)	/* in three bytes */
#define	SNDRV_PCM_FORMAT_S20_3BE	(37)	/* in three bytes */
#define	SNDRV_PCM_FORMAT_U20_3LE	(38)	/* in three bytes */
#define	SNDRV_PCM_FORMAT_U20_3BE	(39)	/* in three bytes */
#define	SNDRV_PCM_FORMAT_S18_3LE	(40)	/* in three bytes */
#define	SNDRV_PCM_FORMAT_S18_3BE	(41)	/* in three bytes */
#define	SNDRV_PCM_FORMAT_U18_3LE	(42)	/* in three bytes */
#define	SNDRV_PCM_FORMAT_U18_3BE	(43)	/* in three bytes */

#define SNDRV_PCM_RATE_8000_44100	(SNDRV_PCM_RATE_8000|SNDRV_PCM_RATE_11025|\
					 SNDRV_PCM_RATE_16000|SNDRV_PCM_RATE_22050|\
					 SNDRV_PCM_RATE_32000|SNDRV_PCM_RATE_44100)
#define SNDRV_PCM_RATE_8000_48000	(SNDRV_PCM_RATE_8000_44100|SNDRV_PCM_RATE_48000)
#define SNDRV_PCM_RATE_8000_96000	(SNDRV_PCM_RATE_8000_48000|SNDRV_PCM_RATE_64000|\
					 SNDRV_PCM_RATE_88200|SNDRV_PCM_RATE_96000)
#define SNDRV_PCM_RATE_8000_192000	(SNDRV_PCM_RATE_8000_96000|SNDRV_PCM_RATE_176400|\
					 SNDRV_PCM_RATE_192000)
#define SNDRV_PCM_FMTBIT_S8		(1ULL << SNDRV_PCM_FORMAT_S8)
#define SNDRV_PCM_FMTBIT_U8		(1ULL << SNDRV_PCM_FORMAT_U8)
#define SNDRV_PCM_FMTBIT_S16_LE		(1ULL << SNDRV_PCM_FORMAT_S16_LE)
#define SNDRV_PCM_FMTBIT_S16_BE		(1ULL << SNDRV_PCM_FORMAT_S16_BE)
#define SNDRV_PCM_FMTBIT_U16_LE		(1ULL << SNDRV_PCM_FORMAT_U16_LE)
#define SNDRV_PCM_FMTBIT_U16_BE		(1ULL << SNDRV_PCM_FORMAT_U16_BE)
#define SNDRV_PCM_FMTBIT_S24_LE		(1ULL << SNDRV_PCM_FORMAT_S24_LE)
#define SNDRV_PCM_FMTBIT_S24_BE		(1ULL << SNDRV_PCM_FORMAT_S24_BE)
#define SNDRV_PCM_FMTBIT_U24_LE		(1ULL << SNDRV_PCM_FORMAT_U24_LE)
#define SNDRV_PCM_FMTBIT_U24_BE		(1ULL << SNDRV_PCM_FORMAT_U24_BE)
#define SNDRV_PCM_FMTBIT_S32_LE		(1ULL << SNDRV_PCM_FORMAT_S32_LE)
#define SNDRV_PCM_FMTBIT_S32_BE		(1ULL << SNDRV_PCM_FORMAT_S32_BE)
#define SNDRV_PCM_FMTBIT_U32_LE		(1ULL << SNDRV_PCM_FORMAT_U32_LE)
#define SNDRV_PCM_FMTBIT_U32_BE		(1ULL << SNDRV_PCM_FORMAT_U32_BE)
#define SNDRV_PCM_FMTBIT_FLOAT_LE	(1ULL << SNDRV_PCM_FORMAT_FLOAT_LE)
#define SNDRV_PCM_FMTBIT_FLOAT_BE	(1ULL << SNDRV_PCM_FORMAT_FLOAT_BE)
#define SNDRV_PCM_FMTBIT_FLOAT64_LE	(1ULL << SNDRV_PCM_FORMAT_FLOAT64_LE)
#define SNDRV_PCM_FMTBIT_FLOAT64_BE	(1ULL << SNDRV_PCM_FORMAT_FLOAT64_BE)
#define SNDRV_PCM_FMTBIT_IEC958_SUBFRAME_LE (1ULL << SNDRV_PCM_FORMAT_IEC958_SUBFRAME_LE)
#define SNDRV_PCM_FMTBIT_IEC958_SUBFRAME_BE (1ULL << SNDRV_PCM_FORMAT_IEC958_SUBFRAME_BE)
#define SNDRV_PCM_FMTBIT_MU_LAW		(1ULL << SNDRV_PCM_FORMAT_MU_LAW)
#define SNDRV_PCM_FMTBIT_A_LAW		(1ULL << SNDRV_PCM_FORMAT_A_LAW)
#define SNDRV_PCM_FMTBIT_IMA_ADPCM	(1ULL << SNDRV_PCM_FORMAT_IMA_ADPCM)
#define SNDRV_PCM_FMTBIT_MPEG		(1ULL << SNDRV_PCM_FORMAT_MPEG)
#define SNDRV_PCM_FMTBIT_GSM		(1ULL << SNDRV_PCM_FORMAT_GSM)
#define SNDRV_PCM_FMTBIT_SPECIAL	(1ULL << SNDRV_PCM_FORMAT_SPECIAL)
#define SNDRV_PCM_FMTBIT_S24_3LE	(1ULL << SNDRV_PCM_FORMAT_S24_3LE)
#define SNDRV_PCM_FMTBIT_U24_3LE	(1ULL << SNDRV_PCM_FORMAT_U24_3LE)
#define SNDRV_PCM_FMTBIT_S24_3BE	(1ULL << SNDRV_PCM_FORMAT_S24_3BE)
#define SNDRV_PCM_FMTBIT_U24_3BE	(1ULL << SNDRV_PCM_FORMAT_U24_3BE)
#define SNDRV_PCM_FMTBIT_S20_3LE	(1ULL << SNDRV_PCM_FORMAT_S20_3LE)
#define SNDRV_PCM_FMTBIT_U20_3LE	(1ULL << SNDRV_PCM_FORMAT_U20_3LE)
#define SNDRV_PCM_FMTBIT_S20_3BE	(1ULL << SNDRV_PCM_FORMAT_S20_3BE)
#define SNDRV_PCM_FMTBIT_U20_3BE	(1ULL << SNDRV_PCM_FORMAT_U20_3BE)
#define SNDRV_PCM_FMTBIT_S18_3LE	(1ULL << SNDRV_PCM_FORMAT_S18_3LE)
#define SNDRV_PCM_FMTBIT_U18_3LE	(1ULL << SNDRV_PCM_FORMAT_U18_3LE)
#define SNDRV_PCM_FMTBIT_S18_3BE	(1ULL << SNDRV_PCM_FORMAT_S18_3BE)
#define SNDRV_PCM_FMTBIT_U18_3BE	(1ULL << SNDRV_PCM_FORMAT_U18_3BE)

typedef int snd_pcm_hw_param_t;
#define	SNDRV_PCM_HW_PARAM_ACCESS	(0) /* Access type */
#define	SNDRV_PCM_HW_PARAM_FORMAT	(1) /* Format */
#define	SNDRV_PCM_HW_PARAM_SUBFORMAT	(2) /* Subformat */
#define	SNDRV_PCM_HW_PARAM_FIRST_MASK	SNDRV_PCM_HW_PARAM_ACCESS
#define	SNDRV_PCM_HW_PARAM_LAST_MASK	SNDRV_PCM_HW_PARAM_SUBFORMAT

#define	SNDRV_PCM_HW_PARAM_SAMPLE_BITS	(8) /* Bits per sample */
#define	SNDRV_PCM_HW_PARAM_FRAME_BITS	(9) /* Bits per frame */
#define	SNDRV_PCM_HW_PARAM_CHANNELS	(10) /* Channels */
#define	SNDRV_PCM_HW_PARAM_RATE		(11) /* Approx rate */
#define	SNDRV_PCM_HW_PARAM_PERIOD_TIME	(12) /* Approx distance between interrupts in us */
#define	SNDRV_PCM_HW_PARAM_PERIOD_SIZE	(13) /* Approx frames between interrupts */
#define	SNDRV_PCM_HW_PARAM_PERIOD_BYTES	(14) /* Approx bytes between interrupts */
#define	SNDRV_PCM_HW_PARAM_PERIODS	(15) /* Approx interrupts per buffer */
#define	SNDRV_PCM_HW_PARAM_BUFFER_TIME	(16) /* Approx duration of buffer in us */
#define	SNDRV_PCM_HW_PARAM_BUFFER_SIZE	(17) /* Size of buffer in frames */
#define	SNDRV_PCM_HW_PARAM_BUFFER_BYTES	(18) /* Size of buffer in bytes */
#define	SNDRV_PCM_HW_PARAM_TICK_TIME	(19) /* Approx tick duration in us */
#define	SNDRV_PCM_HW_PARAM_FIRST_INTERVAL	SNDRV_PCM_HW_PARAM_SAMPLE_BITS
#define	SNDRV_PCM_HW_PARAM_LAST_INTERVAL	SNDRV_PCM_HW_PARAM_TICK_TIME

#define SNDRV_PCM_HW_PARAMS_NORESAMPLE		(1<<0)	/* avoid rate resampling */

#define SNDRV_PCM_STREAM_PLAYBACK	0
#define SNDRV_PCM_STREAM_CAPTURE	1

static inline int snd_pcm_hw_constraint_step(struct snd_pcm_runtime *runtime,
					     int b, int parm, int val)
{
	return 0;
}

struct snd_pcm_hw_constraint_list {
	unsigned int count;
	unsigned int *list;
	unsigned int mask;
};

static inline int snd_pcm_hw_constraint_list(struct snd_pcm_runtime *run,
					     int var1, int var2, void *ptr)
{
	return 0;
}

int snd_pcm_format_width(int fmt);

/*
 * PCM channel-mapping control API
 */
/* array element of channel maps */
struct snd_pcm_chmap_elem {
	unsigned char channels;
	unsigned char map[15];
};

/* channel map information; retrieved via snd_kcontrol_chip() */
struct snd_pcm_chmap {
	struct snd_pcm *pcm;	/* assigned PCM instance */
	int stream;		/* PLAYBACK or CAPTURE */
	struct snd_kcontrol *kctl;
	const struct snd_pcm_chmap_elem *chmap;
	unsigned int max_channels;
	unsigned int channel_mask;	/* optional: active channels bitmask */
	void *private_data;	/* optional: private data pointer */
};

/* get the PCM substream assigned to the given chmap info */
static inline struct snd_pcm_substream *
snd_pcm_chmap_substream(struct snd_pcm_chmap *info, unsigned int idx)
{
	struct snd_pcm_substream *s;
	for (s = info->pcm->streams[info->stream].substream; s; s = s->next)
		if (s->number == idx)
			return s;
	return NULL;
}

/* ALSA-standard channel maps (RL/RR prior to C/LFE) */
extern const struct snd_pcm_chmap_elem snd_pcm_std_chmaps[];
/* Other world's standard channel maps (C/LFE prior to RL/RR) */
extern const struct snd_pcm_chmap_elem snd_pcm_alt_chmaps[];

/* bit masks to be passed to snd_pcm_chmap.channel_mask field */
#define SND_PCM_CHMAP_MASK_24	((1U << 2) | (1U << 4))
#define SND_PCM_CHMAP_MASK_246	(SND_PCM_CHMAP_MASK_24 | (1U << 6))
#define SND_PCM_CHMAP_MASK_2468	(SND_PCM_CHMAP_MASK_246 | (1U << 8))

int snd_pcm_add_chmap_ctls(struct snd_pcm *pcm, int stream,
			   const struct snd_pcm_chmap_elem *chmap,
			   int max_channels,
			   unsigned long private_value,
			   struct snd_pcm_chmap **info_ret);

/*
 */
struct snd_dma_device {
	int type;			/* SNDRV_DMA_TYPE_XXX */
	struct device *dev;		/* generic device */
};

struct snd_dma_buffer {
	struct snd_dma_device dev;	/* device type */
	unsigned char *area;	/* virtual pointer */
	dma_addr_t addr;	/* physical address */
	size_t bytes;		/* buffer size in bytes */
	void *private_data;	/* private for allocator; don't touch */
};

#define snd_pcm_suspend_all(pcm)

static inline ssize_t frames_to_bytes(struct snd_pcm_runtime *runtime, snd_pcm_sframes_t size)
{
	return size * runtime->frame_bits / 8;
}

/* XXX just for compilation, not really working */
#define snd_sgbuf_get_addr(dmab, offset)	0
#define snd_sgbuf_get_chunk_size(dmab, ofs, size)	(size)
#define snd_pcm_get_dma_buf(substream) NULL
#define snd_pcm_lib_buffer_bytes(substream)	0
#define snd_pcm_lib_period_bytes(substream)	0
#define snd_pcm_gettime(runtime, tv) /* nop */

#endif /* __SOUND_PCM_H */
