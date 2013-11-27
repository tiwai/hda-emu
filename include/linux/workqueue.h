#ifndef __LINUX_WORKQUEUE_H
#define __LINUX_WORKQUEUE_H

struct workqueue_struct {
	int dummy;
};

struct work_struct {
#ifdef OLD_WORKQUEUE
	void (*func)(void *data);
	void *work_data;
#else
	void (*func)(struct work_struct *);
#endif
};

struct delayed_work {
	struct work_struct work;
};

extern struct delayed_work *__work_pending;

#ifdef OLD_WORKQUEUE
#define INIT_WORK(x,y,z) ((x)->func = (y), (x)->work_data = (z))
#define schedule_work(x) (x)->func((x)->work_data)
#else
#define INIT_WORK(x,y) ((x)->func = (y))
#define schedule_work(x) (x)->func(x)
#endif
#define INIT_DELAYED_WORK(x,y) ((x)->work.func = (y))
#define schedule_delayed_work(x,y) (__work_pending = (x))
#define cancel_delayed_work(x) do {} while (0)
static inline bool cancel_work_sync(struct work_struct *x) { return 0; }
static inline bool cancel_delayed_work_sync(struct delayed_work *x) { return 0; }
static inline void flush_scheduled_work(void)
{
	struct delayed_work *tmp_work = __work_pending;
	__work_pending = NULL;
	if (tmp_work)
		tmp_work->work.func(&tmp_work->work);
}

#define create_workqueue(x)	(struct workqueue_struct *)1 /* dummy */
#define create_singlethread_workqueue(x)	create_workqueue(x)
#define destroy_workqueue(x)	do {} while (0)
#define queue_work(q,x)		schedule_work(x)
#define queue_delayed_work(q,x,d) schedule_delayed_work(x,d)
#define flush_workqueue(q)	flush_scheduled_work()
#define delayed_work_pending(q)	0
#define workqueue_set_max_active(q,n) do {} while (0)

static inline struct delayed_work *to_delayed_work(struct work_struct *work)
{
	return container_of(work, struct delayed_work, work);
}

#endif /* __LINUX_WORKQUEUE_H */
