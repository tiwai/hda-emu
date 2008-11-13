#ifndef __LINUX_WORKQUEUE_H
#define __LINUX_WORKQUEUE_H

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
#define schedule_delayed_work(x,y) (__work_pending = (x))
#define cancel_delayed_work(x) do {} while (0)
#define cancel_work_sync(x) do {} while (0)
static inline void flush_scheduled_work(void)
{
	if (__work_pending) {
		__work_pending->work.func(&__work_pending->work);
		__work_pending = NULL;
	}
}

#endif /* __LINUX_WORKQUEUE_H */
