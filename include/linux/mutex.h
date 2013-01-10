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

struct rw_semaphore {
	int lock;
};

#define rwsem_init(x)	mylock_init(&(x)->lock)
#define down_read(x)	mylock_read_lock(&(x)->lock, __FILE__, __LINE__)
#define up_read(x)	mylock_read_unlock(&(x)->lock, __FILE__, __LINE__)
#define down_write(x)	mylock_write_lock(&(x)->lock, __FILE__, __LINE__)
#define up_write(x)	mylock_write_unlock(&(x)->lock, __FILE__, __LINE__)

#endif /* __LINUX_MUTEX_H */
