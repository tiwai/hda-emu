#ifndef __LINUX_DELAY_H
#define __LINUX_DELAY_H

#define mdelay(x) do { } while (0)
#define udelay(x) do { } while (0)
#define msleep(x) do { } while (0)
#define HZ	1000
#define msecs_to_jiffies(x)	(x)

#endif /* __LINUX_DELAY_H */
