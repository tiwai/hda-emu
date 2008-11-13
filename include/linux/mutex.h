#ifndef __LINUX_MUTEX_H
#define __LINUX_MUTEX_H

#include "wrapper.h"

struct mutex {
	int dummy;
};

#define mutex_init(x)
#define mutex_lock(x)
#define mutex_unlock(x)

#endif /* __LINUX_MUTEX_H */
