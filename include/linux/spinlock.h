#ifndef __LINUX_SPINLOCK_H
#define __LINUX_SPINLOCK_H

typedef int spinlock_t;
typedef int rwlock_t;

#define spin_lock_init(x)	mylock_init(x)
#define spin_lock(x)		mylock_lock(x, __FILE__, __LINE__)
#define spin_unlock(x)		mylock_unlock(x, __FILE__, __LINE__)
#define spin_lock_irq(x)	mylock_lock(x, __FILE__, __LINE__)
#define spin_unlock_irq(x)	mylock_unlock(x, __FILE__, __LINE__)
#define spin_lock_irqsave(x, flags) do { (flags) = 0; mylock_lock(x); } while (0)
#define spin_unlock_irqrestore(x, flags) do { (flags) = 0; mylock_unlock(x); } while (0)

#define rwlock_init(x)
#define read_lock(x)
#define read_unlock(x)
#define read_lock_irq(x)
#define read_unlock_irq(x)
#define read_lock_irqsave(x, flags)		((flags) = 0)
#define read_unlock_irqrestore(x, flags)	((flags) = 0)
#define write_lock(x)
#define write_unlock(x)
#define write_lock_irq(x)
#define write_unlock_irq(x)
#define write_lock_irqsave(x, flags)		((flags) = 0)
#define write_unlock_irqrestore(x, flags)	((flags) = 0)

#endif
