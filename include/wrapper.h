/*
 */

#ifndef __HDA_WRAPPER_H
#define __HDA_WRAPPER_H

#include <stdarg.h>

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

#define pr_fmt(fmt) fmt
#define pr_emerg(fmt, ...) \
	printk(KERN_EMERG pr_fmt(fmt), ##__VA_ARGS__)
#define pr_alert(fmt, ...) \
	printk(KERN_ALERT pr_fmt(fmt), ##__VA_ARGS__)
#define pr_crit(fmt, ...) \
	printk(KERN_CRIT pr_fmt(fmt), ##__VA_ARGS__)
#define pr_err(fmt, ...) \
	printk(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__)
#define pr_warning(fmt, ...) \
	printk(KERN_WARNING pr_fmt(fmt), ##__VA_ARGS__)
#define pr_warn pr_warning
#define pr_notice(fmt, ...) \
	printk(KERN_NOTICE pr_fmt(fmt), ##__VA_ARGS__)
#define pr_info(fmt, ...) \
	printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)
#define pr_cont(fmt, ...) \
	printk(KERN_CONT fmt, ##__VA_ARGS__)
#define pr_debug(fmt, ...) \
	printk(KERN_DEBUG fmt, ##__VA_ARGS__)

/* FIXME */
#define scnprintf	snprintf

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

#define USHRT_MAX	((u16)(~0U))
#define SHRT_MAX	((s16)(USHRT_MAX>>1))
#define SHRT_MIN	((s16)(-SHRT_MAX - 1))
#define INT_MAX		((int)(~0U>>1))
#define INT_MIN		(-INT_MAX - 1)
#define UINT_MAX	(~0U)
#define LONG_MAX	((long)(~0UL>>1))
#define LONG_MIN	(-LONG_MAX - 1)
#define ULONG_MAX	(~0UL)
#define LLONG_MAX	((long long)(~0ULL>>1))
#define LLONG_MIN	(-LLONG_MAX - 1)
#define ULLONG_MAX	(~0ULL)

typedef _Bool bool;
#define true 1
#define false 0

typedef unsigned long dma_addr_t;

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

#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))

#define jiffies		0
static inline bool time_after_eq(unsigned long a, unsigned long b)
{ return 1; }

static inline bool time_before(unsigned long a, unsigned long b)
{ return 0; }

#define BUG_ON(x) do { \
	if (x) { fprintf(stderr, "ERROR! (%s:%d)\n", __func__, __LINE__); } \
	} while (0)

#define BUILD_BUG_ON(x) do {} while (0)

static inline int _WARN_ON(int cond, const char *func, int line)
{
	if (cond)
		printk(KERN_ERR "WARNING! (%s:%d)\n", func, line);
	return cond;
}

#define WARN_ON(cond)	_WARN_ON(cond, __func__, __LINE__)

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

static inline int kstrtoint(const char *s, unsigned int base, int *res)
{
	*res = strtol(s, NULL, base);
	return 0;
}

static inline size_t strlcat(char *dest, const char *src, size_t count)
{
	size_t dsize = strlen(dest);
	size_t len = strlen(src);
	size_t res = dsize + len;

	dest += dsize;
	count -= dsize;
	if (len >= count)
		len = count-1;
	memcpy(dest, src, len);
	dest[len] = 0;
	return res;
}

void *__hda_malloc(size_t size, const char *file, int line, int gfp);
void __hda_free(void *ptr, const char *file, int line);
void *__hda_realloc(const void *p, size_t new_size, const char *file, int line, int gfp);
void *__hda_strdup(const char *str, const char *file, int line, int gfp);

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
void mylock_read_lock(int *lock, const char *file, int line);
void mylock_read_unlock(int *lock, const char *file, int line);
void mylock_write_lock(int *lock, const char *file, int line);
void mylock_write_unlock(int *lock, const char *file, int line);

#include "linux/atomic.h"
#include "linux/spinlock.h"
#include "linux/pci_ids.h"
#include "linux/workqueue.h"
#include "linux/bitmap.h"
#include "linux/module.h"
#include "linux/log2.h"

/* FIXME */
#define IS_ENABLED(x)		1
#define IS_BUILTIN(x)		1
#define IS_MODULE(x)		0

#define symbol_request(x)	x
#define symbol_put(x)
#define symbol_put_addr(x)

static inline long copy_from_user(void *to, const void __user *from, unsigned long n)
{
	memcpy(to, from, n);
	return 0;
}

static inline long copy_to_user(void __user *to, const void *from, unsigned long n)
{
	memcpy(to, from, n);
	return 0;
}


static inline char *kvasprintf(int gfp, const char *fmt, va_list ap)
{
	char tmp;
	va_list app;
	int len;
	char *buf;

	va_copy(app, ap);
	len = vsnprintf(&tmp, 1, fmt, app);
	va_end(app);
	if (len < 0)
		return NULL;
	buf = __hda_malloc(len + 1, __FILE__, __LINE__, 0);
	if (!buf)
		return NULL;
	vsnprintf(buf, len + 1, fmt, ap);
	return buf;
}

static inline char *kasprintf(int gfp, const char *fmt, ...)
{
	va_list ap;
	char *ret;

	va_start(ap, fmt);
	ret = kvasprintf(gfp, fmt, ap);
	va_end(ap);
	return ret;
}

#define KBUILD_MODNAME	__FILE__

#define module_driver(__driver, __register, __unregister, ...) \
static int __init __driver##_init(void) \
{ \
	return __register(&(__driver) , ##__VA_ARGS__); \
} \
module_init(__driver##_init); \
static void __exit __driver##_exit(void) \
{ \
	__unregister(&(__driver) , ##__VA_ARGS__); \
} \
module_exit(__driver##_exit);

/* attributes */
#define __printf(a, b)	__attribute__((format(printf, a, b)))

#endif /* __HDA_WRAPPER_H */
