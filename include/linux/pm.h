#ifndef __LINUX_PM_H
#define __LINUX_PM_H

struct device;

typedef struct pm_message {
	int event;
} pm_message_t;

#define PM_EVENT_INVALID	(-1)
#define PM_EVENT_ON		0x0000
#define PM_EVENT_FREEZE		0x0001
#define PM_EVENT_SUSPEND	0x0002
#define PM_EVENT_HIBERNATE	0x0004
#define PM_EVENT_QUIESCE	0x0008
#define PM_EVENT_RESUME		0x0010
#define PM_EVENT_THAW		0x0020
#define PM_EVENT_RESTORE	0x0040
#define PM_EVENT_RECOVER	0x0080
#define PM_EVENT_USER		0x0100
#define PM_EVENT_REMOTE		0x0200
#define PM_EVENT_AUTO		0x0400

#define PMSG_INVALID	((struct pm_message){ .event = PM_EVENT_INVALID, })
#define PMSG_ON		((struct pm_message){ .event = PM_EVENT_ON, })
#define PMSG_FREEZE	((struct pm_message){ .event = PM_EVENT_FREEZE, })
#define PMSG_QUIESCE	((struct pm_message){ .event = PM_EVENT_QUIESCE, })
#define PMSG_SUSPEND	((struct pm_message){ .event = PM_EVENT_SUSPEND, })
#define PMSG_HIBERNATE	((struct pm_message){ .event = PM_EVENT_HIBERNATE, })
#define PMSG_RESUME	((struct pm_message){ .event = PM_EVENT_RESUME, })
#define PMSG_THAW	((struct pm_message){ .event = PM_EVENT_THAW, })
#define PMSG_RESTORE	((struct pm_message){ .event = PM_EVENT_RESTORE, })
#define PMSG_RECOVER	((struct pm_message){ .event = PM_EVENT_RECOVER, })
#define PMSG_USER_SUSPEND	((struct pm_message) \
					{ .event = PM_EVENT_USER_SUSPEND, })
#define PMSG_USER_RESUME	((struct pm_message) \
					{ .event = PM_EVENT_USER_RESUME, })
#define PMSG_REMOTE_RESUME	((struct pm_message) \
					{ .event = PM_EVENT_REMOTE_RESUME, })
#define PMSG_AUTO_SUSPEND	((struct pm_message) \
					{ .event = PM_EVENT_AUTO_SUSPEND, })
#define PMSG_AUTO_RESUME	((struct pm_message) \
					{ .event = PM_EVENT_AUTO_RESUME, })

/* stripped version */
struct dev_pm_info {
	pm_message_t		power_state;
};

/* stripped version */
struct dev_pm_ops {
	int (*suspend)(struct device *dev);
	int (*resume)(struct device *dev);
	int (*runtime_suspend)(struct device *dev);
	int (*runtime_resume)(struct device *dev);
	int (*runtime_idle)(struct device *dev);
};

#define SET_SYSTEM_SLEEP_PM_OPS(suspend_fn, resume_fn) \
	.suspend = suspend_fn, \
	.resume = resume_fn,

#define SET_RUNTIME_PM_OPS(suspend_fn, resume_fn, idle_fn) \
	.runtime_suspend = suspend_fn, \
	.runtime_resume = resume_fn, \
	.runtime_idle = idle_fn,

#endif /* __LINUX_PM_H */
