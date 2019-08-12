#ifndef __SOUND_MEMALLOC_H
#define __SOUND_MEMALLOC_H

/*
 * buffer types
 */
#define SNDRV_DMA_TYPE_UNKNOWN		0	/* not defined */
#define SNDRV_DMA_TYPE_CONTINUOUS	1	/* continuous no-DMA memory */
#define SNDRV_DMA_TYPE_DEV		2	/* generic device continuous */
#define SNDRV_DMA_TYPE_DEV_UC		5	/* continuous non-cahced */
#ifdef CONFIG_SND_DMA_SGBUF
#define SNDRV_DMA_TYPE_DEV_SG		3	/* generic device SG-buffer */
#define SNDRV_DMA_TYPE_DEV_UC_SG	6	/* SG non-cached */
#else
#define SNDRV_DMA_TYPE_DEV_SG	SNDRV_DMA_TYPE_DEV /* no SG-buf support */
#define SNDRV_DMA_TYPE_DEV_UC_SG	SNDRV_DMA_TYPE_DEV_UC
#endif
#ifdef CONFIG_GENERIC_ALLOCATOR
#define SNDRV_DMA_TYPE_DEV_IRAM		4	/* generic device iram-buffer */
#else
#define SNDRV_DMA_TYPE_DEV_IRAM	SNDRV_DMA_TYPE_DEV
#endif

#include <sound/pcm.h>

#endif
