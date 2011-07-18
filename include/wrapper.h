/*
 */

#ifndef __HDA_WRAPPER_H
#define __HDA_WRAPPER_H

#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })

#define clamp(val, min, max) ({			\
	typeof(val) __val = (val);		\
	typeof(min) __min = (min);		\
	typeof(max) __max = (max);		\
	(void) (&__val == &__min);		\
	(void) (&__val == &__max);		\
	__val = __val < __min ? __min: __val;	\
	__val > __max ? __max: __val; })

#define min_t(type, x, y) ({			\
	type __min1 = (x);			\
	type __min2 = (y);			\
	__min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({			\
	type __max1 = (x);			\
	type __max2 = (y);			\
	__max1 > __max2 ? __max1: __max2; })

#define clamp_t(type, val, min, max) ({		\
	type __val = (val);			\
	type __min = (min);			\
	type __max = (max);			\
	__val = __val < __min ? __min: __val;	\
	__val > __max ? __max: __val; })

#define clamp_val(val, min, max) ({		\
	typeof(val) __val = (val);		\
	typeof(val) __min = (min);		\
	typeof(val) __max = (max);		\
	__val = __val < __min ? __min: __val;	\
	__val > __max ? __max: __val; })

#define swap(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

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
#define KERN_DEBUG	"<7>"
#define KERN_INFO	"<6>"
#define KERN_NOTICE	"<5>"
#define KERN_WARNING	"<4>"
#define KERN_ERR	"<3>"

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

typedef _Bool bool;
#define true 1
#define false 0

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

#define put_user(a,b)	({*(b) = (a); 0;})

#define jiffies		0
#define time_after_eq(a,b)	1

#define BUG_ON(x) do { \
	if (x) { fprintf(stderr, "ERROR!\n"); } \
	} while (0)

#define BUILD_BUG_ON(x) do {} while (0)

#define simple_strtoul	strtoul
#define simple_strtol	strtol

static inline int strict_strtoul(const char *str, unsigned int base, unsigned long *val)
{
	*val = strtoul(str, NULL, base);
	return 0;
}

static inline int strict_strtol(const char *str, unsigned int base, long *val)
{
	*val = strtol(str, NULL, base);
	return 0;
}

/* XXX: this is just a workaround */
static inline char *strlcat(char *dst, const char *src, size_t n)
{
	return strncat(dst, src, n);
}

#ifdef DEBUG_MALLOC
void *__hda_malloc(size_t size, const char *file, int line);
void __hda_free(void *ptr, const char *file, int line);
void *__hda_realloc(const void *p, size_t new_size, const char *file, int line);
void *__hda_strdup(const char *str, const char *file, int line);
#endif

/*
 * lock debug
 */
enum {
	MYLOCK_UNINIT,
	MYLOCK_UNLOCKED,
	MYLOCK_LOCKED,
};

void mylock_init(int *lock);
void mylock_lock(int *lock, const char *file, int line);
void mylock_unlock(int *lock, const char *file, int line);

#include "linux/spinlock.h"
#include "linux/pci_ids.h"
#include "linux/workqueue.h"
#include "linux/bitmap.h"
#include "linux/module.h"
#include "linux/log2.h"

#endif /* __HDA_WRAPPER_H */
