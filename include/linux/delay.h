#ifndef __LINUX_DELAY_H
#define __LINUX_DELAY_H

static inline void mdelay(unsigned long x) {}
static inline void udelay(unsigned long x) {}
static inline void msleep(unsigned long x) {}
static inline void ssleep(unsigned long x) {}
#define HZ	1000
#define msecs_to_jiffies(x)	(x)

#endif /* __LINUX_DELAY_H */
