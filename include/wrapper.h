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
typedef int64_t s64;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint8_t u8;
typedef int8_t s8;

#define PAGE_SIZE	4096

/* export as XXX_parameter */
#define module_param(a,b,c) b *a##_parameter = &a
#define module_param_array(a,b,c,d)
#define MODULE_PARM_DESC(a,b)

struct work_struct {
	void (*func)(struct work_struct *);
};

struct delayed_work {
	struct work_struct work;
};

extern struct delayed_work *__work_pending;

#define INIT_WORK(x,y) ((x)->func = (y))
#define schedule_work(x) (x)->func(x)
#define INIT_DELAYED_WORK(x,y) ((x)->work.func = (y))
#define schedule_delayed_work(x,y) __work_pending = (x)
#define cancel_delayed_work(x)
static inline void flush_scheduled_work(void)
{
	if (__work_pending) {
		__work_pending->work.func(&__work_pending->work);
		__work_pending = NULL;
	}
}

#define EXPORT_SYMBOL(x)

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

#define PCI_VENDOR_ID_DELL		0x1028
#define PCI_VENDOR_ID_HP		0x103c
#define PCI_VENDOR_ID_INTEL		0x8086

#endif /* __HDA_WRAPPER_H */
