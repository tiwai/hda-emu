#ifndef __LINUX_MUTEX_H
#define __LINUX_MUTEX_H

#include "wrapper.h"

struct mutex {
	int dummy;
};

#define mutex_init(x)	do { (void)(x); } while (0)
#define mutex_lock(x)	do { (void)(x); } while (0)
#define mutex_unlock(x)	do { (void)(x); } while (0)

#define DEFINE_MUTEX(x) struct mutex x

#endif /* __LINUX_MUTEX_H */
