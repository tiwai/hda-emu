#ifndef __LINUX_MUTEX_H
#define __LINUX_MUTEX_H

#include "wrapper.h"

struct mutex {
	int lock;
};

#define mutex_init(x)	mylock_init(&(x)->lock)
#define mutex_lock(x)	mylock_lock(&(x)->lock, __FILE__, __LINE__)
#define mutex_unlock(x)	mylock_unlock(&(x)->lock, __FILE__, __LINE__)

#define DEFINE_MUTEX(x) struct mutex x = { .lock = MYLOCK_UNLOCKED }

#endif /* __LINUX_MUTEX_H */
