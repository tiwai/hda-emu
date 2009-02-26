/*
 */

#ifndef __HDA_WRAPPER_H
#define __HDA_WRAPPER_H

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})
#define prefetch(x) (x)

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "hda-log.h"
/* #define printk	printf */
#define printk	hda_log_printk
#define KERN_DEBUG
#define KERN_INFO
#define KERN_WARNING
#define KERN_ERR

typedef uint64_t u64;
typedef uint64_t __u64;
typedef int64_t s64;
typedef int64_t __s64;
typedef uint32_t u32;
typedef uint32_t __u32;
typedef int32_t s32;
typedef int32_t __s32;
typedef uint16_t u16;
typedef uint16_t __u16;
typedef int16_t s16;
typedef int16_t __s16;
typedef uint8_t u8;
typedef uint8_t __u8;
typedef int8_t s8;
typedef int8_t __s8;

#define PAGE_SIZE	4096

#define __user
#define __bitwise

typedef int pm_message_t;
#define PMSG_SUSPEND	0

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define cond_resched()

#undef abs
#define abs(x) ({				\
		int __x = (x);			\
		(__x < 0) ? -__x : __x;		\
	})

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define put_user(a,b)	0

#define jiffies		0
#define time_after_eq(a,b)	1

#define BUG_ON(x) do { \
	if (x) { fprintf(stderr, "ERROR!\n"); } \
	} while (0)

#define BUILD_BUG_ON(x) do {} while (0)

#define PCI_VENDOR_ID_DELL		0x1028
#define PCI_VENDOR_ID_HP		0x103c
#define PCI_VENDOR_ID_DFI		0x15bd
#define PCI_VENDOR_ID_INTEL		0x8086

#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/bitmap.h>
#include <linux/module.h>

#endif /* __HDA_WRAPPER_H */
